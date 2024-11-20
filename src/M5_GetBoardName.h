#include <M5Unified.h>

#ifndef M5_GETBOARDNAME_H
#define M5_GETBOARDNAME_H

String getBoardName(lgfx::board_t board)
{
  switch (board)
  {
  case lgfx::board_unknown:
    return "unknown";
  case lgfx::board_M5Stack:
    return "M5Stack";
  case lgfx::board_M5StackCore2:
    return "M5StackCore2";
  case lgfx::board_M5StickC:
    return "M5StickC";
  case lgfx::board_M5StickCPlus:
    return "M5StickCPlus";
  case lgfx::board_M5StickCPlus2:
    return "M5StickCPlus2";
  case lgfx::board_M5StackCoreInk:
    return "M5StackCoreInk";
  case lgfx::board_M5Paper:
    return "M5Paper";
  case lgfx::board_M5Tough:
    return "M5Tough";
  case lgfx::board_M5Station:
    return "M5Station";
  case lgfx::board_M5StackCoreS3:
    return "M5StackCoreS3";
  case lgfx::board_M5AtomS3:
    return "M5AtomS3";
  case lgfx::board_M5Dial:
    return "M5Dial";
  case lgfx::board_M5DinMeter:
    return "M5DinMeter";
  case lgfx::board_M5Cardputer:
    return "M5Cardputer";
  case lgfx::board_M5AirQ:
    return "M5AirQ";
  case lgfx::board_M5VAMeter:
    return "M5VAMeter";
  case lgfx::board_M5StackCoreS3SE:
    return "M5StackCoreS3SE";
  case lgfx::board_M5AtomS3R:
    return "M5AtomS3R";
  case lgfx::board_M5AtomLite:
    return "M5AtomLite";
  case lgfx::board_M5AtomPsram:
    return "M5AtomPsram";
  case lgfx::board_M5AtomU:
    return "M5AtomU";
  case lgfx::board_M5Camera:
    return "M5Camera";
  case lgfx::board_M5TimerCam:
    return "M5TimerCam";
  case lgfx::board_M5StampPico:
    return "M5StampPico";
  case lgfx::board_M5StampC3:
    return "M5StampC3";
  case lgfx::board_M5StampC3U:
    return "M5StampC3U";
  case lgfx::board_M5StampS3:
    return "M5StampS3";
  case lgfx::board_M5AtomS3Lite:
    return "M5AtomS3Lite";
  case lgfx::board_M5AtomS3U:
    return "M5AtomS3U";
  case lgfx::board_M5Capsule:
    return "M5Capsule";
  case lgfx::board_M5NanoC6:
    return "M5NanoC6";
  case lgfx::board_M5AtomMatrix:
    return "M5AtomMatrix";
  case lgfx::board_M5AtomEcho:
    return "M5AtomEcho";
  case lgfx::board_M5AtomS3RExt:
    return "M5AtomS3RExt";
  case lgfx::board_M5AtomS3RCam:
    return "M5AtomS3RCam";
  case lgfx::board_M5AtomDisplay:
    return "M5AtomDisplay";
  case lgfx::board_M5UnitLCD:
    return "M5UnitLCD";
  case lgfx::board_M5UnitOLED:
    return "M5UnitOLED";
  case lgfx::board_M5UnitMiniOLED:
    return "M5UnitMiniOLED";
  case lgfx::board_M5UnitGLASS:
    return "M5UnitGLASS";
  case lgfx::board_M5UnitGLASS2:
    return "M5UnitGLASS2";
  case lgfx::board_M5UnitRCA:
    return "M5UnitRCA";
  case lgfx::board_M5ModuleDisplay:
    return "M5ModuleDisplay";
  case lgfx::board_M5ModuleRCA:
    return "M5ModuleRCA";
  case lgfx::board_FrameBuffer:
    return "FrameBuffer";
  default:
    return "Unknown Board";
  }
}

#endif