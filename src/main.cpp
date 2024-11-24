#include <M5Unified.h>
#include <SPI.h>
// #include <Wire.h>
#include <M5_Ethernet.h>
// #include <time.h>
#include <EEPROM.h>
#include "M5_ToF4M.h"
#include "M5_Ethernet_FtpClient.h"
#include "M5_Ethernet_NtpClient.h"
#include "M5_GetBoardName.h"

#include "main.h"
#include "main_EEPROM_handler.h"
#include "main_HTTP_UI.h"
#include "main_Loop.h"

#include "esp_mac.h"

// == M5AtomS3R_Bus ==
#define SCK 5
#define MISO 7
#define MOSI 8
#define CS 6

void TimeUpdateLoop(void *arg);
void TimeServerAccessLoop(void *arg);
void ButtonKeepCountLoop(void *arg);
void ShotLoop(void *arg);
void ShotTask(void *arg);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient FtpClient(21);
M5_Ethernet_FtpClient ftp(ftpSrvIP_String, ftp_user, ftp_pass, 60000);
M5_Ethernet_NtpClient NtpClient;

VL53L1X tofDevice;

bool UnitEnable = true;

String getInterfaceMacAddress(esp_mac_type_t interface)
{

  String macString = "";
  unsigned char mac_base[6] = {0};
  if (esp_read_mac(mac_base, interface) == ESP_OK)
  {
    char buffer[18]; // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    macString = buffer;
  }

  return macString;
}

bool EthernetBegin()
{
  CONFIG_IDF_TARGET_ESP32S3;
  M5_LOGI("MAX_SOCK_NUM = %s", String(MAX_SOCK_NUM));
  if (MAX_SOCK_NUM < 8)
  {
    M5_LOGE("need overwrite MAX_SOCK_NUM = 8 in M5_Ethernet.h");
    UnitEnable = false;
    return false;
  }
  SPI.begin(SCK, MISO, MOSI, -1);
  Ethernet.init(CS);

  esp_read_mac(mac, ESP_MAC_ETH);
  Ethernet.begin(mac, storeData.deviceIP);

  return UnitEnable;
}

bool TofDeviceBegin()
{
  M5.Ex_I2C.begin();
  tofDevice.setBus(&Wire);
  tofDevice.setTimeout(500);
  if (!tofDevice.init())
  {
    M5_LOGE("Failed to detect and initialize sensor.");
    while (1)
      ;
  }
  tofDevice.setDistanceMode(VL53L1X::Long);
  tofDevice.setMeasurementTimingBudget(50000);
  tofDevice.startContinuous(50);
  M5_LOGI("TofDeveceStart");
  return UnitEnable;
}

String TofDeviceStatusRead()
{
  String result = String(tofDevice.read());
  M5_LOGI("range: %u\tstatus: %s\tpeak signal: %.2f\tambient: %.2f", tofDevice.ranging_data.range_mm, VL53L1X::rangeStatusToString(tofDevice.ranging_data.range_status), tofDevice.ranging_data.peak_signal_count_rate_MCPS, tofDevice.ranging_data.ambient_count_rate_MCPS);
  return result;
}

void updateFTP_ParameterFromGrobalStrings()
{
  M5_LOGD("ftpSrvIP_String: %s", ftpSrvIP_String);
  ftp.SetServerAddress(ftpSrvIP_String);
  ftp.SetUserName(ftp_user);
  ftp.SetPassWord(ftp_pass);

  M5_LOGD("GetServerAddress: %s", ftp.GetServerAddress());
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_VERBOSE);
  delay(1000); // M5_Log starting wait

  M5_LOGI("%s", getBoardName(M5.getBoard()));

  EEPROM.begin(STORE_DATA_SIZE);
  LoadEEPROM();

  updateFTP_ParameterFromGrobalStrings();

  if (!EthernetBegin())
    return;

  if (!TofDeviceBegin())
    return;

  HttpUIServer.begin();
  NtpClient.begin();
  NtpClient.getTime(ntpSrvIP_String, (int)(storeData.timeZoneOffset));

  xTaskCreatePinnedToCore(TimeUpdateLoop, "TimeUpdateLoop", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TimeServerAccessLoop, "TimeServerAccessLoop", 4096, NULL, 6, NULL, 0);
  xTaskCreatePinnedToCore(ButtonKeepCountLoop, "ButtonKeepCountLoop", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(ShotLoop, "ShotLoop", 4096, NULL, 3, NULL, 1);

  M5.Display.println(String(storeData.deviceName));
  M5.Display.println("Board: " + getBoardName(M5.getBoard()));
  M5.Display.println("HOST: " + deviceIP_String);
}

void loop()
{
  delay(189);
  HTTP_UI();
  M5.Display.setTextFont(6);
  M5.Display.setCursor(8, 30);
  M5.Display.println(SensorValueString);

  M5.Display.setTextFont(1);
  M5.Display.println("  " + getInterfaceMacAddress(ESP_MAC_WIFI_STA));
  M5.Display.println("  " + getInterfaceMacAddress(ESP_MAC_WIFI_SOFTAP));
  M5.Display.println("  " + getInterfaceMacAddress(ESP_MAC_BT));
  M5.Display.println("  " + getInterfaceMacAddress(ESP_MAC_ETH));

  String ntpAddressLine = "NTP:" + ntpSrvIP_String;
  for (u_int8_t i = 0; i < (21 - ntpAddressLine.length()) / 2; i++)
  {
    M5.Display.print(" ");
  }

  M5.Display.println(ntpAddressLine);

  if (NtpClient.enable())
  {
    M5.Display.println(" " + NtpClient.convertTimeEpochToString() + "   ");
  }
  else
  {
    M5.Display.println("time information can not use now.     ");
  }

  Ethernet.maintain();
}
