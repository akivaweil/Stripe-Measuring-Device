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

void otaAllLedsOff() {}

void otaUpdateProgressLEDs(unsigned int progress, unsigned int total) {}

void setupOTA() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
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

  Serial.println("OTA Initialized");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void handleOTA() {
  ArduinoOTA.handle();
}
