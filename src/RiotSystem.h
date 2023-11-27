#ifndef RIOTSYSTEM_H_
#define RIOTSYSTEM_H_

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

  void beep(int duration);

  RIoTSystem() {
    SYSTEM_STATUS SYSTEM = SYS_NORMAL;
    buzzerWrongDuration = 0.55 * 1000; // in seconds

    maintenanceLowerHour = 05;
    maintenanceLowerMinute = 00;

    maintenanceUpperHour = 05;
    maintenanceUpperMinute = 30;

    taskExecuted = false;

    startTimer = true;
    resetCounter = 0;
    resetThreshold = 5;
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
  static SYSTEM_STATUS SYSTEM;

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
   * @brief Releases the door and handles to communication between Büyük
   * Ağabey (Big Brother) and Küçük Kızkardeş (Little Sister).
   * @return None.
   *
   */
  void releaseDoor();

  /**
   *
   * @brief Controls the door depending on the coniditon of the virtual lock
   * and acts accordingly to userType, riotCardStatus, and riotCardID.
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
};

#endif
