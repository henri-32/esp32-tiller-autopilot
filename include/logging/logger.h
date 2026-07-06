#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <new>

namespace log
{

struct task_context
{
  void* THIS = nullptr;
  QueueHandle_t dataQueue; 
};

} // namespace log

class Logger
{
public:
  Logger()
//{{{
  {
    void* contextMem = pvPortMalloc(sizeof(log::task_context));
    if (contextMem != nullptr)
    {
      context_ = new (contextMem) log::task_context{};
    }
  };
//}}}

  ~Logger()
//{{{
  {
    if (context_ != nullptr)
    {
      context_->~task_context();
    }
  };

  void init();

private:
  log::task_context* context_;
};
//}}}
