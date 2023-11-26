#ifndef RIOTSYSTEM_H_
#define RIOTSYSTEM_H_

#include <WString.h>

enum SYSTEM_STATUS {
  SYS_NORMAL,
  SYS_BACKUP,
};

enum DOOR_STATUS { DOOR_LOCKED, DOOR_UNLOCKED, DOOR_SECURED, DOOR_DEFAULT };

extern SYSTEM_STATUS SYSTEM;

extern bool startTimer;
extern long now;
extern long lastTrigger;

extern bool taskExecuted;

extern const char *ntpServer;
extern const long gmtOffset_sec; // +3 UTC in seconds
extern const int daylightOffset_sec;

extern const int maintenanceLowerHour;
extern const int maintenanceLowerMinute;

extern const int maintenanceUpperHour;
extern const int maintenanceUpperMinute;

/**
 *
 * @brief Set ups the pins, attaches the interrupt and starts the Serial.
 * @return None.
 *
 */
void setUpPins();

/**
 *
 * @brief Backup read (interrupt) if RIoT cannot connect to the Network,
 * Firebase, or unknown discrepency happens.
 * @return None.
 *
 * @note Backup, checks from the embedded RIoT knownTagUIDs such as "Master
 * Key", and "Backup Card".
 */
void backUpRead();

/**
 *
 * @brief Releases the door and handles to communication between Büyük Ağabey
 * (Big Brother) and Küçük Kızkardeş (Little Sister).
 * @return None.
 *
 */
void releaseDoor();

/**
 *
 * @brief Controls the door depending on the coniditon of the virtual lock and
 * acts accordingly to userType, riotCardStatus, and riotCardID.
 * @return None.
 *
 */
void doorController(String tagUID);

/**
 *
 * @brief Maintenance mode for handling RIoT Health and correct the
 * abnormalities in a specific time interval of a day.
 * @return Boolean value, indicates the success of the operation.
 *
 * @note Maintenance time is adjusted for GMT +3 with zero dayLightOffset.
 */
bool systemMaintenance();

#endif
