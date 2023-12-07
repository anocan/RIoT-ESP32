#ifndef RIOTSYSTEM_H_
#define RIOTSYSTEM_H_

#include <Preferences.h>
#include <WString.h>
#include <mutex>

///* IMPORTANT
/// BE HIGHLY CAUTIOUS WHEN MODIFYING HERE
/// KNOW WHAT YOU ARE DOING ELSE MEMORY CORRUPT HAPPENS
///*

class RIoTSystem {
private:
  std::mutex mutex_;

  int resetCounter;
  bool startTimer;
  bool taskExecuted;
  int resetThreshold;

  int maintenanceLowerHour;
  int maintenanceLowerMinute;

  int maintenanceUpperHour;
  int maintenanceUpperMinute;

  int buzzerWrongDuration;
  int buzzerCorrectDuration;

  void beep(int duration);

  RIoTSystem() {
    SYSTEM_STATUS SYSTEM = SYS_NORMAL;
    buzzerWrongDuration = 0.55 * 1000;  // in seconds
    buzzerCorrectDuration = 0.2 * 1000; // in seconds

    maintenanceLowerHour = 19;
    maintenanceLowerMinute = 00;

    maintenanceUpperHour = 20;
    maintenanceUpperMinute = 30;

    taskExecuted = false;

    startTimer = true;
    resetCounter = 0;
    resetThreshold = 5;
    releaseCommand = "release";
    holdCommand = "hold";
    releaseCommandBackup = "releaseBackup";
    holdCommandBackup = "holdBackup";
    doorHoldDuration = 2 * 1000; // t seconds in milliseconds
  } // Private constructor prevents external instantiation
  RIoTSystem(const RIoTSystem &) = delete;
  RIoTSystem &operator=(const RIoTSystem &) = delete;

public:
  enum SYSTEM_STATUS {
    SYS_NORMAL,
    SYS_BACKUP,
  };
  enum DOOR_STATUS {
    DOOR_LOCKED,
    DOOR_UNLOCKED,
    DOOR_SECURED,
    DOOR_DEFAULT,
  };
  Preferences preferences;
  static SYSTEM_STATUS SYSTEM;
  int doorHoldDuration;
  unsigned long doorHoldStartTime;
  const char *releaseCommand;
  const char *holdCommand;
  const char *releaseCommandBackup;
  const char *holdCommandBackup;

  DOOR_STATUS hashit(String string);

  static RIoTSystem &getInstance() {
    static RIoTSystem instance;
    return instance;
  };
  void setSystemStatus(SYSTEM_STATUS status) {
    std::lock_guard<std::mutex> lock(mutex_);
    SYSTEM = status;
  }

  static void ISR_function() {
    RIoTSystem &instance = getInstance();
    instance.backUpRead();
  }
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
  static void backUpRead();

  /**
   *
   * @brief Handles to communication between BüyükAğabey
   * (BigBrother) and KüçükKızkardeş (LittleSister).
   * @param request Request message to send.
   * @return Boolean value, indicates the success of the operation.
   *
   */
  void requestToLittleLister(const char *request);

  /**
   *
   * @brief Controls the door depending on the condition of the virtual lock
   * and acts accordingly to doorStatus and requestFromBigBrother for
   * LittleSister.
   * @return Boolean value, indicates the success of the operation.
   *
   */
  bool littleSisterDoorController();

  /**
   *
   * @brief Controls the door depending on the condition of the virtual lock
   * and acts accordingly to userType, riotCardStatus, and riotCardID for
   * BigBrother.
   * @param tagUID The tagUID to compare.
   * @return None.
   *
   */
  void bigBrotherDoorController(String tagUID);

  /**
   *
   * @brief Maintenance mode for handling RIoT Health and correct the
   * abnormalities in a specific time interval of a day.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note Maintenance time is adjusted for GMT +3 with zero dayLightOffset.
   */
  bool systemMaintenance();
};

#endif
