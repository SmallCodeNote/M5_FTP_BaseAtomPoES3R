#include <M5Unified.h>
#include <M5_Ethernet.h>
#include <EEPROM.h>

#ifndef MAIN_EEPROM_HANDLER_H
#define MAIN_EEPROM_HANDLER_H

#define STORE_DATA_SIZE 128                // byte
#define STORE_DATA_DEVICENAME_MAXLENGTH 31 // byte
#define EEPROM_CHECK_CODE 0x44

/// @brief Encorder Profile Struct
struct DATA_SET
{
    uint8_t romCheckCode;

    /// @brief IP address
    IPAddress deviceIP;
    IPAddress ftpSrvIP;
    IPAddress ntpSrvIP;

    u_int16_t interval;

    /// @brief deviceName
    char deviceName[STORE_DATA_DEVICENAME_MAXLENGTH + 1];
    char ftp_user[32];
    char ftp_pass[32];
};

extern String ftp_user;
extern String ftp_pass ;

/// @brief Encorder Profile
extern DATA_SET storeData;
extern String shotInterval;

extern String deviceName ;
extern String deviceIP_String ;
extern String ftpSrvIP_String ;
extern String ntpSrvIP_String ;

void InitEEPROM();
void LoadEEPROM();
void PutEEPROM();
void SetStringsFromStoreData();
void SetStoreDataFromStrings();

#endif
