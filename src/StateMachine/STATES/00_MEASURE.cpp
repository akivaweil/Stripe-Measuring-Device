#include "StateMachine/Measure.h"
#include "Config/Pin_Def.h"
#include "Config/Config.h"
#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ MEASURE STATE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

#define READINGS_AVG_COUNT 10

static float s_readings[READINGS_AVG_COUNT];
static int s_readingCount = 0;
static int s_readingWriteIndex = 0;
static float s_distanceInches = 0.0f;
static float s_boardLengthInches = 0.0f;
static bool s_sensorValid = false;

void Measure::Run() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  unsigned long durationUs = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);
  if (durationUs > 0) {
    float raw = (float)durationUs / ULTRASONIC_US_PER_INCH;
    if (raw < 0.0f) raw = 0.0f;
    if (raw > MEASUREMENT_SPAN_INCHES) raw = MEASUREMENT_SPAN_INCHES;
    s_readings[s_readingWriteIndex] = raw;
    s_readingWriteIndex = (s_readingWriteIndex + 1) % READINGS_AVG_COUNT;
    if (s_readingCount < READINGS_AVG_COUNT) s_readingCount++;
    float sum = 0.0f;
    for (int i = 0; i < s_readingCount; i++) sum += s_readings[i];
    s_distanceInches = sum / (float)s_readingCount;
    s_boardLengthInches = MEASUREMENT_SPAN_INCHES - s_distanceInches;
  }
  s_sensorValid = (s_readingCount > 0);
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
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
}
