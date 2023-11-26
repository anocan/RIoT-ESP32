#ifndef RFID_H_
#define RFID_H_

#include <WString.h>

/**
 *
 * @brief Initialize the RFID and configure debug and RIoT system settings.
 * @return Boolean value, indicates the success of the operation.
 *
 */
bool initRFID();

/**
 *
 * @brief Read from the RFID and return the tagUID.
 * @return If success String tagUID value, if not "NULL".
 *
 */
String readRFID();

#endif