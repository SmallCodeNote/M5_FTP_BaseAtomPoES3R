#include <M5Unified.h>
#include <SPI.h>
#include <Wire.h>
#include <M5_Ethernet.h>
#include <time.h>
#include <EEPROM.h>
#include "M5_ToF4M.h"
#include "M5_Ethernet_FtpClient.hpp"
#include "M5_Ethernet_NtpClient.hpp"

#include "main_EEPROM_handler.h"
#include "main_HTTP_UI.h"
#include "M5_GetBoardName.h"

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

VL53L1X tofDevice;
String tofDeviceValueString = "";

bool UnitEnable = true;

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
  Ethernet.begin(mac, deviceIP);

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

void sendMessage(String messageString)
{
  M5.Display.println(messageString);
  // M5_LOGV("%s",messageString);
}
void updateFTP_Parameter()
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
  updateFTP_Parameter();

  M5.Display.println(String(storeData.deviceName));
  M5.Display.println("Board: " + getBoardName(M5.getBoard()));
  M5.Display.println("HOST: " + deviceIP_String);

  if (!EthernetBegin())
    return;

  if (!TofDeviceBegin())
    return;

  HttpServer.begin();
  NtpClient.begin();
  NtpClient.getTime(ntpSrvIP_String, +9);

  xTaskCreatePinnedToCore(TimeUpdateLoop, "TimeUpdateLoop", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TimeServerAccessLoop, "TimeServerAccessLoop", 4096, NULL, 6, NULL, 0);
  xTaskCreatePinnedToCore(ButtonKeepCountLoop, "ButtonKeepCountLoop", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(ShotLoop, "ShotLoop", 4096, NULL, 3, NULL, 1);
}

void loop()
{
  delay(189);
  HTTP_UI();
  M5.Display.setTextFont(6);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(0, 40);
  M5.Display.print(tofDeviceValueString);

  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(0, 111);
  M5.Display.println("NTP:" + ntpSrvIP_String);
  M5.Display.println(NtpClient.readHour() + ":" + NtpClient.readMinute() + ":" + NtpClient.readSecond());
  Ethernet.maintain();
}

void TimeUpdateLoop(void *arg)
{
  M5_LOGI("TimeUpdateLoop Start  ");
  unsigned long lastepoc = 0;

  while (1)
  {
    delay(200);
    String timeLine = NtpClient.getTime(ntpSrvIP_String, +9);
    if (lastepoc > NtpClient.currentEpoch)
      lastepoc = 0;

    if (lastepoc + 10 <= NtpClient.currentEpoch)
    {
      M5.Log.println(("timeLine= " + timeLine).c_str());
      lastepoc = NtpClient.currentEpoch;
    }
  }

  vTaskDelete(NULL);
}

void TimeServerAccessLoop(void *arg)
{
  M5_LOGI("TimeServerAccessLoop Start  ");
  unsigned long count = 0;
  while (1)
  {
    delay(10000);
    if (count > 60)
    {
      NtpClient.updateTimeFromServer(ntpSrvIP_String, +9);
      count = 0;
    }
  }

  vTaskDelete(NULL);
}

void ButtonKeepCountLoop(void *arg)
{
  M5_LOGI("ButtonKeepCountLoop Start  ");
  int PushKeepSubSecCounter = 0;

  while (1)
  {
    delay(1000);
    M5.update();

    if (M5.BtnA.isPressed())
    {
      M5_LOGI("BtnA Pressed");
      PushKeepSubSecCounter++;
    }
    else if (M5.BtnA.wasReleased())
    {
      M5_LOGI("BtnA Released");
      PushKeepSubSecCounter = 0;
    }

    if (PushKeepSubSecCounter > 6)
      break;
  }

  bool LED_ON = true;
  while (1)
  {
    delay(1000);

    M5.update();
    if (M5.BtnA.wasReleased())
      break;

    LED_ON = !LED_ON;
  }

  InitEEPROM();
  delay(1000);
  ESP.restart();

  vTaskDelete(NULL);
}

typedef struct
{
  unsigned long currentEpoch;
  uint16_t tofDeviceValue;
} ShotTaskParams;

unsigned long CheckStartOffset()
{
  const int intervals[] = {3600, 600, 300, 10, 5};
  for (int i = 0; i < sizeof(intervals) / sizeof(intervals[0]); i++)
  {
    if (storeData.interval % intervals[i] == 0)
    {
      return 1;
    }
  }
  return 0;
}

bool ShotTaskRunTrigger(unsigned long currentEpoch)
{
  String ms = NtpClient.readMinute(currentEpoch);
  char mc0 = ms[0];
  char mc1 = ms[1];
  String ss = NtpClient.readSecond(currentEpoch);
  char sc0 = ss[0];
  char sc1 = ss[1];

  if (!(storeData.interval % 3600) && mc0 == '0' && (sc0 == '0' && sc1 == '0'))
    return true;
  else if (!(storeData.interval % 600) && (mc1 == '0') && (sc0 == '0' && sc1 == '0'))
    return true;
  else if (!(storeData.interval % 300) && (mc1 == '0' || mc1 == '5') && (sc0 == '0' && sc1 == '0'))
    return true;
  else if (!(storeData.interval % 10) && sc1 == '0')
    return true;
  else if (!(storeData.interval % 5) && (sc1 == '0' || sc1 == '5'))
    return true;

  return false;
}

void ShotLoop(void *arg)
{
  unsigned long lastWriteEpoc = 0;
  unsigned long lastCheckEpoc = 0;
  ShotTaskParams taskParam;
  uint16_t tofDeviceValue = 0;
  char tofDeviceValueChars[5];

  String directoryName_Past = "";

  M5_LOGI("ShotLoop Start  ");

  if (Ethernet.linkStatus() == LinkON)
    ftp.OpenConnection();

  while (true)
  {
    tofDeviceValue = tofDevice.read();
    sprintf(tofDeviceValueChars, "%04d", tofDeviceValue);
    tofDeviceValueString = String(tofDeviceValueChars);

    if ((Ethernet.linkStatus() == LinkON) && storeData.interval >= 1)
    {
      unsigned long currentEpoch = NtpClient.currentEpoch;
      if (currentEpoch < lastWriteEpoc)
        lastWriteEpoc = 0;

      if (currentEpoch - lastCheckEpoc > 1)
        M5_LOGW("EpocDeltaOver 1:");

      lastCheckEpoc = currentEpoch;
      unsigned long checkStartOffset = CheckStartOffset();

      // if (currentEpoch + checkStartOffset >= lastWriteEpoc + storeData.interval)
      if (currentEpoch >= lastWriteEpoc + storeData.interval)
      {
        if (!ftp.isConnected())
          ftp.OpenConnection();
        if ((checkStartOffset == 0 || ShotTaskRunTrigger(currentEpoch) ))
        {
          taskParam.currentEpoch = currentEpoch;
          taskParam.tofDeviceValue = tofDeviceValue;
          xTaskCreatePinnedToCore(ShotTask, "ShotTask", 4096, &taskParam, 4, NULL, 1);

          lastWriteEpoc = currentEpoch;
          delay(1000);
          continue;
        }
      }
    }
    delay(100);
  }

  if (ftp.isConnected())
    ftp.CloseConnection();

  vTaskDelete(NULL);
}

String LastWriteDirectoryPath = "";
void ShotTask(void *param)
{
  String directoryPath = "/" + deviceName;

  ShotTaskParams *taskParam = (ShotTaskParams *)param;
  unsigned long currentEpoch = taskParam->currentEpoch;
  uint16_t tofDeviceValue = taskParam->tofDeviceValue;

  String ss = NtpClient.readSecond(currentEpoch);
  String mm = NtpClient.readMinute(currentEpoch);
  String HH = NtpClient.readHour(currentEpoch);
  String DD = NtpClient.readDay(currentEpoch);
  String MM = NtpClient.readMonth(currentEpoch);
  String YYYY = NtpClient.readYear(currentEpoch);

  if (storeData.interval < 60)
  {
    directoryPath = "/" + deviceName + "/" + YYYY + "/" + YYYY + MM + "/" + YYYY + MM + DD;
  }
  else
  {
    directoryPath = "/" + deviceName + "/" + YYYY + "/" + YYYY + MM + "/" + YYYY + MM + DD;
  }

  String filePath = directoryPath + "/" + YYYY + MM + DD + "_" + HH;
  String TimeLine = YYYY + "/" + MM + "/" + DD + " " + HH + ":" + mm + ":" + ss;

  if (true)
  {
    // ftp.OpenConnection();
    if (LastWriteDirectoryPath != directoryPath)
      ftp.MakeDirRecursive(directoryPath);
    ftp.AppendTextLine(filePath + ".csv", TimeLine + "," + String(tofDeviceValue));
    // ftp.CloseConnection();
  }

  LastWriteDirectoryPath = directoryPath;
  vTaskDelete(NULL);
}
