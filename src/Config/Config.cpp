//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ CONFIG - STRIPE MEASUREMENT DEVICE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Edit values here. Declarations are in include/Config/Config.h
// Calibrated from test board: 7-15/16" away, 4-7/16" long (dashboard was 7.84 / 4.16).

#include "Config/Config.h"

const float MEASUREMENT_SPAN_INCHES = 12.375f;   // 7.9375 + 4.4375 (actual test span)
const float ULTRASONIC_US_PER_INCH = 146.18f;    // 148 * (7.84/7.9375) so distance reads correct
