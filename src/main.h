#include "M5_Ethernet_FtpClient.h"
#include "M5_Ethernet_NtpClient.h"
#include "M5_ToF4M.h"

#ifndef MAIN_BASE_FUNCTION_H
#define MAIN_BASE_FUNCTION_H

extern M5_Ethernet_FtpClient ftp;
extern M5_Ethernet_NtpClient NtpClient;
extern VL53L1X tofDevice;;

extern SemaphoreHandle_t mutex_Ethernet;

#endif