#pragma once

#include <cstddef>
#include <cstdint>

enum class InitStatus : uint8_t
{
  OK = 0,
  FAIL = 1
};

struct RuntimeLog_t
{
  uint8_t init_status = 1;
  size_t free_task_stack = 64999;
};


