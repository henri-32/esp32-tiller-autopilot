#pragma once
#include <cstdint>
enum class InitStatus : uint8_t {
	OK = 0,
	FAIL = 1
};

struct RuntimeLog_t {
	uint8_t init_status = 1;
	uint16_t free_task_stack = 64999;


};














struct NmeaConfig
{
  static constexpr uint16_t max_sentence_len = 128;
  static constexpr uint8_t queue_depth = 10;
};
