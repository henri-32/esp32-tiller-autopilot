#pragma once
#include "core/freertosTypes.h"
#include "core/queueServer.h"
#include "esp_err.h"
#include "protocol/internalMessageProtocol.h"

class UartDriver;

class InputHandler
{
public:
  InputHandler(QueueServer* qs, UartDriver* const uart)
      : id_{ModuleID::INPUT_HANDLER}, queueServer_(qs), uartDriver_(uart),
        runtimeLogMessage_{id_} {};

  esp_err_t init();

private:
  QueueServer* queueServer_;

public:
  ModuleID id_;
  QueueBundle_t queueBundle_;
  UartDriver* uartDriver_;

private:
  RuntimeLogMessage runtimeLogMessage_;
};
