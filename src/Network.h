#ifndef NETWORK_H_
#define NETWORK_H_

extern const char *ssid;
extern const char *password;
extern const char *ssidWPA2;

/**
 *
 * @brief Initialize the Network and configure debug and RIoT system settings.
 * @return Boolean value, indicates the success of the operation.
 *
 * @note Waits specific amount of time to connect to the WPA2 (meturaom), then
 * tries to connect to the default Network
 */
bool initWiFi();

#endif