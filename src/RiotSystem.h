#ifndef RIOTSYSTEM_H_
#define RIOTSYSTEM_H_

#include <Arduino.h>

#define INTERRUPT_SECONDS 0 // Wait for INTERRUPT_SECONDS to trigger // BUGGY

enum SYSTEM_STATUS {
    SYS_NORMAL,
    SYS_BACKUP,
};

extern SYSTEM_STATUS SYSTEM;

extern boolean startTimer;
extern long now;
extern long lastTrigger;

extern bool taskExecuted;

extern const char* ntpServer;
extern const long  gmtOffset_sec; // +3 UTC in seconds
extern const int   daylightOffset_sec;

extern const int maintenanceLowerHour;
extern const int maintenanceLowerMinute;

extern const int maintenanceUpperHour;
extern const int maintenanceUpperMinute;

void backUpRead();
void releaseDoor();
void doorController(String tagUID);
void systemMaintenance();

#endif
