#pragma once
#include "core/queueServer.h"
#include "esp_err.h"

class UartDriver;

class InputHandler
{
public:
  InputHandler(QueueServer* qs, UartDriver* const uart) : queueServer_(qs), uartDriver_(uart) {};

  esp_err_t init();

private:
  QueueServer* queueServer_;
  UartDriver* uartDriver_;
  task_context context_;
};
