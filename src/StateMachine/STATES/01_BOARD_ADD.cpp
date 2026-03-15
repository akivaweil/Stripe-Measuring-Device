#include "StateMachine/BoardAdd.h"
#include "StateMachine/Measure.h"
#include "StateMachine/Idle.h"
#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ BOARD ADD STATE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Config at top of state
static const unsigned long STABLE_DURATION_MS = 100;
static const float STABLE_TOLERANCE_INCHES = 0.25f;
static const float MIN_BOARD_LENGTH_INCHES = 1.0f;
static const float MAX_BOARD_LENGTH_INCHES = 11.0f;
static const unsigned long MIN_TIME_BETWEEN_ADDS_MS = 300;
static const float CLEAR_BOARD_LENGTH_MAX_INCHES = 0.0f;
static const unsigned long CLEAR_DURATION_MS = 200;

static float s_refLength = 0.0f;
static unsigned long s_stableStartMs = 0;
static bool s_stableTimerActive = false;
static unsigned long s_lastAddMs = 0;
static unsigned long s_clearStartMs = 0;
static bool s_clearSeenFor200ms = false;

void RunBoardAdd() {
  unsigned long now = millis();
  float len = Measure::GetBoardLengthInches();
  bool valid = Measure::HasValidSensorReading();

  //! Require all sensors clear for CLEAR_DURATION_MS before next add
  if (valid && len <= CLEAR_BOARD_LENGTH_MAX_INCHES) {
    if (s_clearStartMs == 0) s_clearStartMs = now;
    if ((now - s_clearStartMs) >= CLEAR_DURATION_MS) s_clearSeenFor200ms = true;
  } else {
    s_clearStartMs = 0;
  }

  //! Clear required only after at least one add
  bool clearOk = (s_lastAddMs == 0) || s_clearSeenFor200ms;

  //! AUTO-ADD when length stable for STABLE_DURATION_MS and within [MIN, MAX] board length
  if (valid && len >= MIN_BOARD_LENGTH_INCHES && len <= MAX_BOARD_LENGTH_INCHES && clearOk) {
    if (!s_stableTimerActive) {
      s_refLength = len;
      s_stableStartMs = now;
      s_stableTimerActive = true;
    } else {
      if (fabsf(len - s_refLength) <= STABLE_TOLERANCE_INCHES) {
        if ((now - s_stableStartMs) >= STABLE_DURATION_MS &&
            (s_lastAddMs == 0 || (now - s_lastAddMs) >= MIN_TIME_BETWEEN_ADDS_MS)) {
          AddBoardToList(len);
          s_lastAddMs = now;
          s_clearSeenFor200ms = false;
          s_clearStartMs = 0;
          s_stableTimerActive = false;
        }
      } else {
        s_refLength = len;
        s_stableStartMs = now;
      }
    }
  } else {
    s_stableTimerActive = false;
  }
}

void ResetBoardAdd() {
  s_stableTimerActive = false;
  s_lastAddMs = 0;
  s_clearStartMs = 0;
  s_clearSeenFor200ms = false;
}

void SetupBoardAdd() {
  //! Manual add button removed
}
