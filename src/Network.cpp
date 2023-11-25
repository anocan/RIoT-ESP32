#include "Network.h"
#include "credentials.h"
#include "RiotSystem.h"
#include "Pins.h"
#include <Wifi.h>

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

int initWiFi() {
    pinMode(NETWORK_PIN, OUTPUT);
    digitalWrite(NETWORK_PIN,HIGH);
    WiFi.disconnect();
    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while (WiFi.status() != WL_CONNECTED) {
        if (SYSTEM == SYS_NORMAL) {
            Serial.print(".");
            delay(100);
        } else if (SYSTEM == SYS_BACKUP) {
            Serial.print("!");
            return -1;
        }

    }
    
    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP8266 IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(NETWORK_PIN,LOW);

    return 1;
}
