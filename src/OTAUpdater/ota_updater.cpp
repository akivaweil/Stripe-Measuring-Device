#include "OTAUpdater/ota_updater.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ OTA UPDATER ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Handles WiFi connection and Over-The-Air updates for the ESP32.

const char* ssid = "Everwood";
const char* password = "Everwood-Staff";

unsigned long previousMillis = 0;
const long interval = 30000; // Check WiFi connection every 30 seconds

void otaAllLedsOff() {}

void otaUpdateProgressLEDs(unsigned int progress, unsigned int total) {}

void setupOTA() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Disable WiFi power save for better responsiveness
  WiFi.setAutoReconnect(true); // Ensure auto-reconnect is enabled
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Try to connect for 10 seconds before proceeding
  // This avoids blocking indefinitely if WiFi is down
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    // Serial.print("."); // Commented out to reduce serial spam if not debugging
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Serial.println("\nWiFi Connected!");
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());
  } else {
    // Serial.println("\nWiFi Connection Failed. Will retry in background.");
  }

  ArduinoOTA.setHostname("stripe-measurement-device");

  ArduinoOTA
    .onStart([]() {})
    .onEnd([]() { otaAllLedsOff(); })
    .onProgress([](unsigned int progress, unsigned int total) {
      otaUpdateProgressLEDs(progress, total);
    })
    .onError([](ota_error_t error) {
      otaAllLedsOff();
    });

  ArduinoOTA.begin();

  // Serial.println("OTA Initialized");
}

void handleOTA() {
  // Ensure WiFi stays connected
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    // Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }
}
