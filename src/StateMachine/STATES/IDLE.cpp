#include "StateMachine/Idle.h"
#include "StateMachine/Measure.h"
#include "StateMachine/BoardAdd.h"
#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ IDLE STATE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

#define MAX_BOARDS 128

static float s_totalInches = 0.0f;
static float s_boardLengths[MAX_BOARDS];
static int s_boardCount = 0;

void RunIdle() {
  Measure::Run();
  RunBoardAdd();
}

void AddBoardToList(float lengthInches) {
  if (s_boardCount >= MAX_BOARDS) return;
  s_totalInches += lengthInches;
  s_boardLengths[s_boardCount++] = lengthInches;
}

void ResetTotal() {
  s_totalInches = 0.0f;
  s_boardCount = 0;
  ResetBoardAdd();
}

float GetDistanceInches() {
  return Measure::GetDistanceInches();
}

float GetBoardLengthInches() {
  return Measure::GetBoardLengthInches();
}

bool HasValidSensorReading() {
  return Measure::HasValidSensorReading();
}

float GetTotalInches() {
  return s_totalInches;
}

int GetBoardCount() {
  return s_boardCount;
}

float GetBoardLengthAtIndex(int i) {
  if (i < 0 || i >= s_boardCount) return 0.0f;
  return s_boardLengths[i];
}

void RemoveLastBoard() {
  if (s_boardCount <= 0) return;
  s_totalInches -= s_boardLengths[s_boardCount - 1];
  s_boardCount--;
}

void SetupIdle() {
  Measure::Setup();
  SetupBoardAdd();
}
