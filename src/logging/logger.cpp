#include "logging/logger.h"

void vLoggingTask(void* pvParameters)
{
  log::task_context* context = static_cast<log::task_context*>(pvParameters);

  while (1)
  {
  }
};

void Logger::init() {
  xTaskCreate(vLoggingTask, "Logging", 10000, context_, 1, nullptr);

};
