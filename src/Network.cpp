#include "Network.h"
#include "Pins.h"
#include "RiotSystem.h"
#include "credentials.h"
#include "esp_wpa2.h"
#include <Wifi.h>

const char *ssid = WIFI_SSID;
const char *ssidWPA2 = WIFI_SSID_WPA2;
const char *password = WIFI_PASSWORD;
unsigned long wifiStartTime = 0;
int wifiTimerThreshold = 2 * 60 * 1000; // 2 minutes in milliseconds

bool initWiFi() {
  int wifiTimer = 0;
  pinMode(NETWORK_PIN, OUTPUT);
  digitalWrite(NETWORK_PIN, HIGH);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // Optional
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME,
                                     strlen(EAP_USERNAME));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD,
                                     strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  WiFi.begin(ssidWPA2); // First try WPA2 (meturoam) network
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  wifiStartTime = millis(); // Record the start time
  Serial.println("\nConnecting to the Network: ");
  Serial.println(WIFI_SSID_WPA2);

  while (WiFi.status() != WL_CONNECTED &&
         millis() - wifiStartTime <= wifiTimerThreshold) {
    if (SYSTEM == SYS_NORMAL) {
      Serial.print(".");
      delay(100);
    } else if (SYSTEM == SYS_BACKUP) {
      Serial.print("!");
      return false;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nTime out for Network: ");
    Serial.println(WIFI_SSID_WPA2);
    Serial.println("\nTrying for Network: ");
    Serial.println(WIFI_SSID);
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      if (SYSTEM == SYS_NORMAL) {
        Serial.print(".");
        delay(100);
      } else if (SYSTEM == SYS_BACKUP) {
        Serial.print("!");
        return false;
      }
    }
  }

  Serial.println("\nConnected to the WiFi Network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(NETWORK_PIN, LOW);

  return true;
}
