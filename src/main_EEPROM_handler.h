#include <M5Unified.h>
#include <M5_Ethernet.h>
#include <EEPROM.h>

#ifndef MAIN_EEPROM_HANDLER_H
#define MAIN_EEPROM_HANDLER_H

#define STORE_DATA_SIZE 128                // byte
#define STORE_DATA_DEVICENAME_MAXLENGTH 31 // byte
#define EEPROM_CHECK_CODE 0x44

IPAddress deviceIP(192, 168, 25, 177);

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

String ftp_user = "ftpusr";
String ftp_pass = "ftpword";

/// @brief Encorder Profile
DATA_SET storeData;
String shotInterval;

String deviceName = "Device";
String deviceIP_String = "";
String ftpSrvIP_String = "";
String ntpSrvIP_String = "";

void SetStringsFromStoreData();
void SetStoreDataFromStrings();

void InitEEPROM()
{
    deviceName = "AtomPoE-S3R";
    deviceIP_String = "192.168.25.177";
    ftpSrvIP_String = "192.168.25.74";
    ntpSrvIP_String = "192.168.25.74";
    ftp_user = "ftpusr";
    ftp_pass = "ftpword";
    shotInterval = "0";

    SetStoreDataFromStrings();
    storeData.romCheckCode = EEPROM_CHECK_CODE;

    EEPROM.put<DATA_SET>(0, storeData);
    EEPROM.commit();
}

void LoadEEPROM()
{
    EEPROM.get<DATA_SET>(0, storeData);
    if (storeData.romCheckCode != EEPROM_CHECK_CODE)
    {
        M5_LOGD("storeData.romCheckCode = %x", storeData.romCheckCode);
        InitEEPROM();
    }
    else
    {
        M5_LOGV("storeData.romCheckCode = %x", storeData.romCheckCode);
    }

    SetStringsFromStoreData();
}

void PutEEPROM()
{
    M5_LOGI("PutEEPROM: romCheckCode = %x", (storeData.romCheckCode));
    M5_LOGI("PutEEPROM: deviceName = %s, %s", deviceName.c_str(), (storeData.deviceName));
    M5_LOGI("PutEEPROM: interval = %s, %u", shotInterval.c_str(), (storeData.interval));

    SetStoreDataFromStrings();
    storeData.romCheckCode = EEPROM_CHECK_CODE;

    EEPROM.put<DATA_SET>(0, storeData);
    EEPROM.commit();
}

void SetStringsFromStoreData()
{
    deviceName = storeData.deviceName;
    deviceIP = storeData.deviceIP;
    deviceIP_String = storeData.deviceIP.toString();
    ftpSrvIP_String = storeData.ftpSrvIP.toString();
    ntpSrvIP_String = storeData.ntpSrvIP.toString();
    ftp_user = storeData.ftp_user;
    ftp_pass = storeData.ftp_pass;
    shotInterval = String(storeData.interval);
}

void SetStoreDataFromStrings()
{
    M5_LOGI("deviceName.length = %u", deviceName.length());

    if (deviceName.length() > STORE_DATA_DEVICENAME_MAXLENGTH)
    {
        deviceName = deviceName.substring(0, STORE_DATA_DEVICENAME_MAXLENGTH - 1);
    }

    strcpy(storeData.deviceName, deviceName.c_str());
    storeData.deviceIP.fromString(deviceIP_String);
    storeData.ntpSrvIP.fromString(ntpSrvIP_String);
    storeData.ftpSrvIP.fromString(ftpSrvIP_String);
    strcpy(storeData.ftp_user, ftp_user.c_str());
    strcpy(storeData.ftp_pass, ftp_pass.c_str());
    storeData.interval = (u_int16_t)(shotInterval.toInt());
}

#endif
