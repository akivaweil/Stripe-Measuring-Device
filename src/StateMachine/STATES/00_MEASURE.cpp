#include "StateMachine/Measure.h"
#include "Config/Pin_Def.h"
#include "Config/Config.h"
#include <Bounce2.h>
#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ MEASURE STATE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

static const float IR_SENSOR_LENGTHS_INCHES[] = {
  1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f
};
static const int IR_SENSOR_LENGTH_COUNT = sizeof(IR_SENSOR_LENGTHS_INCHES) / sizeof(IR_SENSOR_LENGTHS_INCHES[0]);
static const unsigned long IR_SENSOR_DEBOUNCE_MS = 5;

static float s_distanceInches = 0.0f;
static float s_boardLengthInches = 0.0f;
static bool s_sensorValid = false;
static Bounce s_irSensors[IR_SENSOR_LENGTH_COUNT];

void Measure::Run() {
  s_boardLengthInches = 0.0f;

  int sensorCount = IR_SENSOR_COUNT;
  if (sensorCount > IR_SENSOR_LENGTH_COUNT) sensorCount = IR_SENSOR_LENGTH_COUNT;

  int highestTriggeredIndex = -1;
  int triggeredCount = 0;

  //! Valid reading requires a solid stack from 1" up to the highest triggered sensor
  for (int i = 0; i < sensorCount; i++) {
    s_irSensors[i].update();
    bool sensorTriggered = (s_irSensors[i].read() == LOW);

    if (sensorTriggered) {
      highestTriggeredIndex = i;
      triggeredCount++;
    }
  }

  bool invalidSensorStack = (highestTriggeredIndex >= 0) && (triggeredCount != highestTriggeredIndex + 1);

  if (!invalidSensorStack && highestTriggeredIndex >= 0) {
    s_boardLengthInches = IR_SENSOR_LENGTHS_INCHES[highestTriggeredIndex];
  }

  s_distanceInches = MAX_MEASUREMENT_LENGTH_INCHES - s_boardLengthInches;
  if (s_distanceInches < 0.0f) s_distanceInches = 0.0f;

  s_sensorValid = (sensorCount > 0) && !invalidSensorStack;
}

float Measure::GetDistanceInches() {
  return s_distanceInches;
}

float Measure::GetBoardLengthInches() {
  return s_boardLengthInches;
}

bool Measure::HasValidSensorReading() {
  return s_sensorValid;
}

void Measure::Setup() {
  for (int i = 0; i < IR_SENSOR_COUNT; i++) {
    s_irSensors[i].attach(IR_SENSOR_PINS[i], INPUT_PULLUP);
    s_irSensors[i].interval(IR_SENSOR_DEBOUNCE_MS);
  }

  s_distanceInches = MAX_MEASUREMENT_LENGTH_INCHES;
  s_boardLengthInches = 0.0f;
  s_sensorValid = (IR_SENSOR_COUNT > 0);
}
