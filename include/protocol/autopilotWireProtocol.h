#pragma once

#include "telemetry/telemetryLogMessage.h"
#include "types/inputHandleTypes.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

// In-memory representation only. The wire header is always exactly three bytes.
struct MessageHeader
{
  uint8_t type = 0;
  uint16_t payload_length = 0;
};

struct NmeaSentences
{
  static constexpr uint8_t max_sentence_count = 20;
  static constexpr uint16_t sentence_length = 128;

  uint8_t sentence_count = 0;
  char sentence[max_sentence_count][sentence_length]{};
};

// Payloads emitted by Logger and routed by the gateway.
enum class PayloadType : uint8_t
{
  NmeaSentences = 1,
  AccumulatedLogMessage = 2
};

// Commands travel in the opposite direction and therefore have their own type
// namespace. Value 3 preserves compatibility with the existing command frames.
enum class CommandType : uint8_t
{
  InputHandleData = 3
};

struct MessageOffsets
{
  static constexpr uint16_t type = 0;
  static constexpr uint16_t payload_length_little_endian = 1;
  static constexpr uint16_t payload_length_big_endian = 2;
  static constexpr uint16_t payload = 3;
};

struct NmeaPayloadOffsets
{
  static constexpr uint16_t sentence_count = 0;
  static constexpr uint16_t sentences = 1;
};

struct AccumulatedLogMessagePayloadOffsets
{
  // SensorSample<uint16_t>: uint16 value, uint8 validity, uint64 timestamp.
  static constexpr uint16_t compass_hdg_dg = 0;
  static constexpr uint16_t gps_cog_dg = compass_hdg_dg + 11;

  // SensorSample<float>: IEEE-754 binary32 value, uint8 validity, uint64 timestamp.
  static constexpr uint16_t gps_sog_kts = gps_cog_dg + 11;
  static constexpr uint16_t target_course = gps_sog_kts + 13;
  static constexpr uint16_t gps_lat = target_course + 2;
  static constexpr uint16_t gps_lon = gps_lat + 4;
  static constexpr uint16_t wind_angle_dg = gps_lon + 4;
  static constexpr uint16_t stw_kts = wind_angle_dg + 11;
  static constexpr uint16_t lead_source = stw_kts + 13;
  static constexpr uint16_t steering_engaged = lead_source + 1;
  static constexpr uint16_t valid_fix = steering_engaged + 1;

  static constexpr uint16_t gps_init_status = valid_fix + 1;
  static constexpr uint16_t gps_free_task_stack = gps_init_status + 1;
  static constexpr uint16_t telemetry_hub_init_status = gps_free_task_stack + 8;
  static constexpr uint16_t telemetry_hub_free_task_stack = telemetry_hub_init_status + 1;
  static constexpr uint16_t input_handler_init_status = telemetry_hub_free_task_stack + 8;
  static constexpr uint16_t input_handler_free_task_stack = input_handler_init_status + 1;
  static constexpr uint16_t payload_length = input_handler_free_task_stack + 8;
};

struct InputHandlePayloadOffsets
{
  static constexpr uint16_t engage = 0;
  static constexpr uint16_t target_course_little_endian = 1;
  static constexpr uint16_t target_course_big_endian = 2;
  static constexpr uint16_t payload_length = target_course_big_endian + 1;
};

template <typename T> struct Message
{
  Message() = default;

  explicit Message(const T* source)
  {
    if (source == nullptr)
    {
      return;
    }

    payload = *source;
    if constexpr (std::is_same_v<T, NmeaSentences>)
    {
      header.type = static_cast<uint8_t>(PayloadType::NmeaSentences);
      header.payload_length = static_cast<uint16_t>(
          NmeaPayloadOffsets::sentences + payload.sentence_count * sizeof(payload.sentence[0]));
    }
    else if constexpr (std::is_same_v<T, AccumulatedLogMessage>)
    {
      header.type = static_cast<uint8_t>(PayloadType::AccumulatedLogMessage);
      header.payload_length = AccumulatedLogMessagePayloadOffsets::payload_length;
    }
    else if constexpr (std::is_same_v<T, InputHandleData_t>)
    {
      header.type = static_cast<uint8_t>(CommandType::InputHandleData);
      header.payload_length = InputHandlePayloadOffsets::payload_length;
    }
  }

  MessageHeader header{};
  T payload{};
};

namespace wire_detail
{
inline void write_uint16(uint8_t* frame, uint16_t offset, uint16_t value)
{
  frame[offset] = static_cast<uint8_t>(value);
  frame[offset + 1] = static_cast<uint8_t>(value >> 8);
}

inline void write_uint32(uint8_t* frame, uint16_t offset, uint32_t value)
{
  for (uint8_t i = 0; i < 4; ++i)
  {
    frame[offset + i] = static_cast<uint8_t>(value >> (i * 8));
  }
}

inline void write_uint64(uint8_t* frame, uint16_t offset, uint64_t value)
{
  for (uint8_t i = 0; i < 8; ++i)
  {
    frame[offset + i] = static_cast<uint8_t>(value >> (i * 8));
  }
}

inline uint16_t read_uint16(const uint8_t* frame, uint16_t offset)
{
  return static_cast<uint16_t>(frame[offset]) | (static_cast<uint16_t>(frame[offset + 1]) << 8);
}

inline uint32_t read_uint32(const uint8_t* frame, uint16_t offset)
{
  uint32_t value = 0;
  for (uint8_t i = 0; i < 4; ++i)
  {
    value |= static_cast<uint32_t>(frame[offset + i]) << (i * 8);
  }
  return value;
}

inline uint64_t read_uint64(const uint8_t* frame, uint16_t offset)
{
  uint64_t value = 0;
  for (uint8_t i = 0; i < 8; ++i)
  {
    value |= static_cast<uint64_t>(frame[offset + i]) << (i * 8);
  }
  return value;
}

inline void write_float(uint8_t* frame, uint16_t offset, float value)
{
  static_assert(sizeof(float) == sizeof(uint32_t), "wire protocol requires 32-bit float");
  uint32_t bits = 0;
  std::memcpy(&bits, &value, sizeof(bits));
  write_uint32(frame, offset, bits);
}

inline float read_float(const uint8_t* frame, uint16_t offset)
{
  const uint32_t bits = read_uint32(frame, offset);
  float value = 0.0F;
  std::memcpy(&value, &bits, sizeof(value));
  return value;
}

inline void write_header(uint8_t* frame, uint8_t type, uint16_t payload_length)
{
  frame[MessageOffsets::type] = type;
  write_uint16(frame, MessageOffsets::payload_length_little_endian, payload_length);
}

inline bool has_header(const void* buffer, uint16_t buffer_size)
{
  return buffer != nullptr && buffer_size >= MessageOffsets::payload;
}

inline bool has_expected_frame(const void* buffer, uint16_t buffer_size, uint8_t expected_type,
                               uint16_t expected_payload_length)
{
  if (!has_header(buffer, buffer_size))
  {
    return false;
  }

  const auto* frame = static_cast<const uint8_t*>(buffer);
  return frame[MessageOffsets::type] == expected_type &&
         read_uint16(frame, MessageOffsets::payload_length_little_endian) ==
             expected_payload_length &&
         buffer_size >= MessageOffsets::payload + expected_payload_length;
}

inline void write_uint16_sample(uint8_t* frame, uint16_t offset,
                                const SensorSample<uint16_t>& sample)
{
  write_uint16(frame, offset, sample.value);
  frame[offset + 2] = static_cast<uint8_t>(sample.valid);
  write_uint64(frame, offset + 3, sample.timestamp);
}

inline void write_float_sample(uint8_t* frame, uint16_t offset, const SensorSample<float>& sample)
{
  write_float(frame, offset, sample.value);
  frame[offset + 4] = static_cast<uint8_t>(sample.valid);
  write_uint64(frame, offset + 5, sample.timestamp);
}

inline SensorSample<uint16_t> read_uint16_sample(const uint8_t* frame, uint16_t offset)
{
  SensorSample<uint16_t> sample{};
  sample.value = read_uint16(frame, offset);
  sample.valid = frame[offset + 2] != 0;
  sample.timestamp = read_uint64(frame, offset + 3);
  return sample;
}

inline SensorSample<float> read_float_sample(const uint8_t* frame, uint16_t offset)
{
  SensorSample<float> sample{};
  sample.value = read_float(frame, offset);
  sample.valid = frame[offset + 4] != 0;
  sample.timestamp = read_uint64(frame, offset + 5);
  return sample;
}
} // namespace wire_detail

inline uint16_t mp_write_NmeaMessage_to_bytes(void* dest_buffer, uint16_t buffer_size,
                                              const Message<NmeaSentences>* msg)
{
  if (dest_buffer == nullptr || msg == nullptr ||
      msg->payload.sentence_count > NmeaSentences::max_sentence_count)
  {
    return 0;
  }

  const uint16_t payload_length =
      static_cast<uint16_t>(NmeaPayloadOffsets::sentences +
                            msg->payload.sentence_count * sizeof(msg->payload.sentence[0]));
  const uint16_t message_size = MessageOffsets::payload + payload_length;
  if (buffer_size < message_size)
  {
    return 0;
  }

  auto* frame = static_cast<uint8_t*>(dest_buffer);
  wire_detail::write_header(frame, static_cast<uint8_t>(PayloadType::NmeaSentences),
                            payload_length);
  frame[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] = msg->payload.sentence_count;
  std::memcpy(frame + MessageOffsets::payload + NmeaPayloadOffsets::sentences,
              msg->payload.sentence,
              static_cast<size_t>(msg->payload.sentence_count) * sizeof(msg->payload.sentence[0]));
  return message_size;
}

inline Message<NmeaSentences> mp_read_NmeaMessage_from_buffer(const void* buffer,
                                                              uint16_t buffer_size)
{
  if (!wire_detail::has_header(buffer, buffer_size) ||
      buffer_size < MessageOffsets::payload + NmeaPayloadOffsets::sentences)
  {
    return {};
  }

  const auto* frame = static_cast<const uint8_t*>(buffer);
  const uint8_t sentence_count =
      frame[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count];
  if (sentence_count > NmeaSentences::max_sentence_count)
  {
    return {};
  }

  const uint16_t payload_length = static_cast<uint16_t>(
      NmeaPayloadOffsets::sentences + sentence_count * sizeof(NmeaSentences::sentence[0]));
  if (!wire_detail::has_expected_frame(
          buffer, buffer_size, static_cast<uint8_t>(PayloadType::NmeaSentences), payload_length))
  {
    return {};
  }

  NmeaSentences sentences{};
  sentences.sentence_count = sentence_count;
  std::memcpy(sentences.sentence, frame + MessageOffsets::payload + NmeaPayloadOffsets::sentences,
              static_cast<size_t>(sentence_count) * sizeof(sentences.sentence[0]));
  return Message<NmeaSentences>{&sentences};
}

inline uint16_t mp_write_AccumulatedLogMessage_to_bytes(void* dest_buffer, uint16_t buffer_size,
                                                      const Message<AccumulatedLogMessage>* msg)
{
  constexpr uint16_t payload_length = AccumulatedLogMessagePayloadOffsets::payload_length;
  constexpr uint16_t message_size = MessageOffsets::payload + payload_length;
  if (dest_buffer == nullptr || msg == nullptr || buffer_size < message_size)
  {
    return 0;
  }

  auto* frame = static_cast<uint8_t*>(dest_buffer);
  wire_detail::write_header(frame, static_cast<uint8_t>(PayloadType::AccumulatedLogMessage),
                            payload_length);

  const auto payload_offset = [](uint16_t offset)
  { return static_cast<uint16_t>(MessageOffsets::payload + offset); };

  wire_detail::write_uint16_sample(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::compass_hdg_dg),
      msg->payload.snapshot.compass_hdg_dg);
  wire_detail::write_uint16_sample(frame,
                                   payload_offset(AccumulatedLogMessagePayloadOffsets::gps_cog_dg),
                                   msg->payload.snapshot.gps_cog_dg);
  wire_detail::write_float_sample(frame,
                                  payload_offset(AccumulatedLogMessagePayloadOffsets::gps_sog_kts),
                                  msg->payload.snapshot.gps_sog_kts);
  wire_detail::write_uint16(frame,
                            payload_offset(AccumulatedLogMessagePayloadOffsets::target_course),
                            msg->payload.snapshot.target_course);
  wire_detail::write_float(frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_lat),
                           msg->payload.snapshot.gps_lat);
  wire_detail::write_float(frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_lon),
                           msg->payload.snapshot.gps_lon);
  wire_detail::write_uint16_sample(frame,
                                   payload_offset(AccumulatedLogMessagePayloadOffsets::wind_angle_dg),
                                   msg->payload.snapshot.wind_angle_dg);
  wire_detail::write_float_sample(frame, payload_offset(AccumulatedLogMessagePayloadOffsets::stw_kts),
                                  msg->payload.snapshot.stw_kts);
  frame[payload_offset(AccumulatedLogMessagePayloadOffsets::lead_source)] =
      static_cast<uint8_t>(msg->payload.snapshot.lead_source);
  frame[payload_offset(AccumulatedLogMessagePayloadOffsets::steering_engaged)] =
      static_cast<uint8_t>(msg->payload.snapshot.steering_engaged);
  frame[payload_offset(AccumulatedLogMessagePayloadOffsets::valid_fix)] =
      static_cast<uint8_t>(msg->payload.error.validFix);

  frame[payload_offset(AccumulatedLogMessagePayloadOffsets::gps_init_status)] =
      msg->payload.runtime_log.gps.init_status;
  wire_detail::write_uint64(frame,
                            payload_offset(AccumulatedLogMessagePayloadOffsets::gps_free_task_stack),
                            static_cast<uint64_t>(msg->payload.runtime_log.gps.free_task_stack));
  frame[payload_offset(AccumulatedLogMessagePayloadOffsets::telemetry_hub_init_status)] =
      msg->payload.runtime_log.tHub.init_status;
  wire_detail::write_uint64(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::telemetry_hub_free_task_stack),
      static_cast<uint64_t>(msg->payload.runtime_log.tHub.free_task_stack));
  frame[payload_offset(AccumulatedLogMessagePayloadOffsets::input_handler_init_status)] =
      msg->payload.runtime_log.inputHandler.init_status;
  wire_detail::write_uint64(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::input_handler_free_task_stack),
      static_cast<uint64_t>(msg->payload.runtime_log.inputHandler.free_task_stack));

  return message_size;
}

inline Message<AccumulatedLogMessage> mp_read_AccumulatedLogMessage_from_buffer(const void* buffer,
                                                                            uint16_t buffer_size)
{
  constexpr uint16_t payload_length = AccumulatedLogMessagePayloadOffsets::payload_length;
  if (!wire_detail::has_expected_frame(buffer, buffer_size,
                                       static_cast<uint8_t>(PayloadType::AccumulatedLogMessage),
                                       payload_length))
  {
    return {};
  }

  const auto* frame = static_cast<const uint8_t*>(buffer);
  const auto payload_offset = [](uint16_t offset)
  { return static_cast<uint16_t>(MessageOffsets::payload + offset); };

  AccumulatedLogMessage telemetry{};
  telemetry.snapshot.compass_hdg_dg = wire_detail::read_uint16_sample(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::compass_hdg_dg));
  telemetry.snapshot.gps_cog_dg = wire_detail::read_uint16_sample(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_cog_dg));
  telemetry.snapshot.gps_sog_kts = wire_detail::read_float_sample(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_sog_kts));
  telemetry.snapshot.target_course = wire_detail::read_uint16(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::target_course));
  telemetry.snapshot.gps_lat =
      wire_detail::read_float(frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_lat));
  telemetry.snapshot.gps_lon =
      wire_detail::read_float(frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_lon));
  telemetry.snapshot.wind_angle_dg = wire_detail::read_uint16_sample(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::wind_angle_dg));
  telemetry.snapshot.stw_kts = wire_detail::read_float_sample(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::stw_kts));
  telemetry.snapshot.lead_source = static_cast<NavigationSource>(
      frame[payload_offset(AccumulatedLogMessagePayloadOffsets::lead_source)]);
  telemetry.snapshot.steering_engaged =
      frame[payload_offset(AccumulatedLogMessagePayloadOffsets::steering_engaged)] != 0;
  telemetry.error.validFix =
      frame[payload_offset(AccumulatedLogMessagePayloadOffsets::valid_fix)] != 0;

  telemetry.runtime_log.gps.init_status =
      frame[payload_offset(AccumulatedLogMessagePayloadOffsets::gps_init_status)];
  telemetry.runtime_log.gps.free_task_stack = static_cast<size_t>(wire_detail::read_uint64(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::gps_free_task_stack)));
  telemetry.runtime_log.tHub.init_status =
      frame[payload_offset(AccumulatedLogMessagePayloadOffsets::telemetry_hub_init_status)];
  telemetry.runtime_log.tHub.free_task_stack = static_cast<size_t>(wire_detail::read_uint64(
      frame, payload_offset(AccumulatedLogMessagePayloadOffsets::telemetry_hub_free_task_stack)));
  telemetry.runtime_log.inputHandler.init_status =
      frame[payload_offset(AccumulatedLogMessagePayloadOffsets::input_handler_init_status)];
  telemetry.runtime_log.inputHandler.free_task_stack =
      static_cast<size_t>(wire_detail::read_uint64(
          frame,
          payload_offset(AccumulatedLogMessagePayloadOffsets::input_handler_free_task_stack)));

  return Message<AccumulatedLogMessage>{&telemetry};
}

inline uint16_t mp_write_InputHandleMessage_to_bytes(void* dest_buffer, uint16_t buffer_size,
                                                     const Message<InputHandleData_t>* msg)
{
  constexpr uint16_t message_size =
      MessageOffsets::payload + InputHandlePayloadOffsets::payload_length;
  if (dest_buffer == nullptr || msg == nullptr || buffer_size < message_size)
  {
    return 0;
  }

  auto* frame = static_cast<uint8_t*>(dest_buffer);
  wire_detail::write_header(frame, static_cast<uint8_t>(CommandType::InputHandleData),
                            InputHandlePayloadOffsets::payload_length);
  frame[MessageOffsets::payload + InputHandlePayloadOffsets::engage] =
      static_cast<uint8_t>(msg->payload.steering_engaged);
  wire_detail::write_uint16(
      frame, MessageOffsets::payload + InputHandlePayloadOffsets::target_course_little_endian,
      msg->payload.target_course);
  return message_size;
}

inline Message<InputHandleData_t> mp_read_InputHandleMessage_from_buffer(const void* buffer,
                                                                         uint16_t buffer_size)
{
  if (!wire_detail::has_expected_frame(buffer, buffer_size,
                                       static_cast<uint8_t>(CommandType::InputHandleData),
                                       InputHandlePayloadOffsets::payload_length))
  {
    return {};
  }

  const auto* frame = static_cast<const uint8_t*>(buffer);
  InputHandleData_t data{};
  data.steering_engaged = frame[MessageOffsets::payload + InputHandlePayloadOffsets::engage] != 0;
  data.target_course = wire_detail::read_uint16(
      frame, MessageOffsets::payload + InputHandlePayloadOffsets::target_course_little_endian);
  return Message<InputHandleData_t>{&data};
}
