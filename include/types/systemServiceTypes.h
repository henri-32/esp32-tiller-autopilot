#pragma once 
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"


struct QueueBundle {
  QueueHandle_t data= nullptr; 
  QueueHandle_t performance= nullptr; 
  QueueHandle_t error= nullptr; 
  
};

struct task_context
{
  void* THIS = nullptr;
  QueueBundle  queueBundle;
};

