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
static const unsigned long STABLE_DURATION_MS = 500;
static const float STABLE_TOLERANCE_INCHES = 0.3f;
static const float MIN_LENGTH_CHANGE_AFTER_ADD_INCHES = 2.0f;
static const float MIN_BOARD_LENGTH_INCHES = 0.5f;
static const float MAX_BOARD_LENGTH_INCHES = 12.0f;

static float s_refLength = 0.0f;
static unsigned long s_stableStartMs = 0;
static bool s_stableTimerActive = false;
static float s_lastAutoAddedLength = -1.0f;
static bool s_buttonWasHigh = false;
static Bounce2::Button s_addButton;

void RunBoardAdd() {
  unsigned long now = millis();
  float len = Measure::GetBoardLengthInches();
  bool valid = Measure::HasValidSensorReading();

  //! AUTO-ADD when length stable for STABLE_DURATION_MS and within [MIN, MAX] board length
  if (valid && len >= MIN_BOARD_LENGTH_INCHES && len <= MAX_BOARD_LENGTH_INCHES) {
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
          if ((now - s_stableStartMs) >= STABLE_DURATION_MS) {
            AddBoardToList(len);
            s_lastAutoAddedLength = len;
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
  if (buttonHigh && !s_buttonWasHigh && valid && len >= MIN_BOARD_LENGTH_INCHES && len <= MAX_BOARD_LENGTH_INCHES) {
    AddBoardToList(len);
    s_lastAutoAddedLength = len;
    s_stableTimerActive = false;
  }
  s_buttonWasHigh = buttonHigh;
}

void ResetBoardAdd() {
  s_lastAutoAddedLength = -1.0f;
  s_stableTimerActive = false;
}

void SetupBoardAdd() {
  pinMode(ADD_BOARD_BUTTON_PIN, INPUT);
  s_addButton.attach(ADD_BOARD_BUTTON_PIN, INPUT_PULLDOWN);
  s_addButton.interval(25);
}
