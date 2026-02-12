#include "StateMachine/BoardAdd.h"
#include "StateMachine/Measure.h"
#include "StateMachine/Idle.h"
#include "Config/Pin_Def.h"
#include <Bounce2.h>
#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ BOARD ADD STATE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Config at top of state
static const unsigned long STABLE_DURATION_MS = 200;
static const float STABLE_TOLERANCE_INCHES = 0.2f;
static const float MIN_LENGTH_CHANGE_AFTER_ADD_INCHES = 2.0f;
static const float MIN_BOARD_LENGTH_INCHES = 2.0f;
static const float MAX_BOARD_LENGTH_INCHES = 12.0f;
static const unsigned long MIN_TIME_BETWEEN_ADDS_MS = 2000;
static const float CLEAR_LENGTH_INCHES = 12.0f;
static const unsigned long CLEAR_DURATION_MS = 200;

static float s_refLength = 0.0f;
static unsigned long s_stableStartMs = 0;
static bool s_stableTimerActive = false;
static float s_lastAutoAddedLength = -1.0f;
static bool s_buttonWasHigh = false;
static unsigned long s_lastAddMs = 0;
static unsigned long s_clearStartMs = 0;
static bool s_clearSeenFor200ms = false;
static Bounce2::Button s_addButton;

void RunBoardAdd() {
  unsigned long now = millis();
  float len = Measure::GetBoardLengthInches();
  bool valid = Measure::HasValidSensorReading();

  //! Require 12+ inches for CLEAR_DURATION_MS before allowing next add
  if (valid && len >= CLEAR_LENGTH_INCHES) {
    if (s_clearStartMs == 0) s_clearStartMs = now;
    if ((now - s_clearStartMs) >= CLEAR_DURATION_MS) s_clearSeenFor200ms = true;
  } else {
    s_clearStartMs = 0;
  }

  //! Clear required only after at least one add (so first board / after reset works without seeing 12+)
  bool clearOk = (s_lastAddMs == 0) || s_clearSeenFor200ms;

  //! AUTO-ADD when length stable for STABLE_DURATION_MS and within [MIN, MAX] board length
  if (valid && len >= MIN_BOARD_LENGTH_INCHES && len <= MAX_BOARD_LENGTH_INCHES && clearOk) {
    if (s_lastAutoAddedLength >= 0.0f) {
      if (fabsf(len - s_lastAutoAddedLength) > MIN_LENGTH_CHANGE_AFTER_ADD_INCHES) {
        s_lastAutoAddedLength = -1.0f;
      }
    }
    if (s_lastAutoAddedLength < 0.0f) {
      if (!s_stableTimerActive) {
        s_refLength = len;
        s_stableStartMs = now;
        s_stableTimerActive = true;
      } else {
        if (fabsf(len - s_refLength) <= STABLE_TOLERANCE_INCHES) {
          if ((now - s_stableStartMs) >= STABLE_DURATION_MS &&
              (s_lastAddMs == 0 || (now - s_lastAddMs) >= MIN_TIME_BETWEEN_ADDS_MS)) {
            AddBoardToList(len);
            s_lastAutoAddedLength = len;
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
    }
  } else {
    s_stableTimerActive = false;
  }

  //! MANUAL BUTTON ADD (rising edge only)
  s_addButton.update();
  bool buttonHigh = (s_addButton.read() == HIGH);
  if (buttonHigh && !s_buttonWasHigh && valid && clearOk &&
      len >= MIN_BOARD_LENGTH_INCHES && len <= MAX_BOARD_LENGTH_INCHES &&
      (s_lastAddMs == 0 || (now - s_lastAddMs) >= MIN_TIME_BETWEEN_ADDS_MS)) {
    AddBoardToList(len);
    s_lastAutoAddedLength = len;
    s_lastAddMs = now;
    s_clearSeenFor200ms = false;
    s_clearStartMs = 0;
    s_stableTimerActive = false;
  }
  s_buttonWasHigh = buttonHigh;
}

void ResetBoardAdd() {
  s_lastAutoAddedLength = -1.0f;
  s_stableTimerActive = false;
  s_lastAddMs = 0;
  s_clearStartMs = 0;
  s_clearSeenFor200ms = false;
}

void SetupBoardAdd() {
  pinMode(ADD_BOARD_BUTTON_PIN, INPUT);
  s_addButton.attach(ADD_BOARD_BUTTON_PIN, INPUT_PULLDOWN);
  s_addButton.interval(25);
}
