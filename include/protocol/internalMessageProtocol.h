#pragma once

#include "drivers/gpsTypes.h"
#include "telemetry/runtimeTypes.h"
#include "types/inputHandleTypes.h"
#include <type_traits>

struct ErrorData_t
{
};

enum class ModuleID
{
  THUB = 0,
  GPS = 1,
  INPUT_HANDLER = 2
};

struct ErrorMessage
{
  ModuleID id{};
  ErrorData_t error;
};

struct RuntimeLogMessage
{
  explicit RuntimeLogMessage(ModuleID ID) : id(ID) {};
  ModuleID id{};
  RuntimeLog_t payload;
};

struct TelemetryMessage
{
  ModuleID id{};
  union
  {
    gpsDriverData_t gpsData;
    InputHandleData_t inputHandleData;
  } payload;
};

/*
 * @param [in] data
 * 		The payload data from the Module
 *
 * @returns a Telemetry Message with id and tagged union data according to the data param
 */
inline TelemetryMessage initialize_TelemetryMessage_with_payload(gpsDriverData_t data)
{
  TelemetryMessage msg{};

  msg.id = ModuleID::GPS;
  msg.payload.gpsData = data;

  return msg;
};

inline TelemetryMessage initialize_TelemetryMessage_with_payload(InputHandleData_t data)
{
  TelemetryMessage msg{};

  msg.id = ModuleID::INPUT_HANDLER;
  msg.payload.inputHandleData = data;

  return msg;
}
