#ifndef NETWORK_H_
#define NETWORK_H_

#include "credentials.h"

class RIoTNetwork {
private:
  const char *ssid;
  const char *password;
  const char *ssidWPA2;

public:
  unsigned long wifiStartTime;
  int wifiTimerThreshold;
  const char *ntpServer;
  long gmtOffset_sec; // +3 UTC in seconds
  int daylightOffset_sec;

  RIoTNetwork() {
    ssid = WIFI_SSID;
    ssidWPA2 = WIFI_SSID_WPA2;
    password = WIFI_PASSWORD;
    wifiStartTime = 0;
    wifiTimerThreshold = 2 * 60 * 1000; // t minutes in milliseconds
    ntpServer = "pool.ntp.org";
    gmtOffset_sec = 3 * 3600; // +3 UTC in seconds
    daylightOffset_sec = 0;
  }

  /**
   *
   * @brief Initialize the Network and configure debug and RIoT system
   * settings.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Waits specific amount of time to connect to the WPA2
   * (meturaom), then tries to connect to the default Network
   */
  bool initWiFi();
};

#endif