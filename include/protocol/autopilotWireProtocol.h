#pragma once

#include "types/controllerTypes.h"
#include "types/inputHandleTypes.h"
#include "types/sensorTypes.h"
#include <cstdint>
#include <cstring>
#include <type_traits>

// In-memory representation of a message header. Do not serialize this struct
// directly: its layout may contain compiler padding.
struct MessageHeader
//{{{
{
  uint8_t type = 0;
  uint16_t payload_length = 0;
};
//}}}
// Up to 20 fixed-width NMEA sentence buffers.
struct NmeaSentences
//{{{
{
  uint8_t sentence_count;
  char sentence[20][128]{};
};
//}}}
// Payload types supported by the autopilot wire protocol.
enum class PayloadType : uint8_t
{
  NmeaSentences = 1,
  NavigationSnapshot = 2,
  InputHandleData = 3
};

// In-memory representation of a typed message. The wire format is defined by
// MessageOffsets and the corresponding payload-offset structure below.
//
// The constructor initializes the in-memory header and payload from payload.
template <typename T> struct Message
//{{{
{
  /**
   * @brief Creates a message from an existing payload.
   *
   * The payload is copied into the message and the header is initialized with
   * the payload type and its size.
   *
   * @param payload Pointer to the payload to copy. Its type must match `T`.
   */
  Message(void* payload)
  {
    if constexpr (std::is_same_v<T, NmeaSentences>)
    {
      header.type = static_cast<uint8_t>(PayloadType::NmeaSentences);
      header.payload_length = sizeof(*(static_cast<NmeaSentences*>(payload)));
      this->payload = *(static_cast<NmeaSentences*>(payload));
    }
    else if constexpr (std::is_same_v<T, NavigationSnapshot>)
    {
      header.type = static_cast<uint8_t>(PayloadType::NavigationSnapshot);
      header.payload_length = sizeof(*(static_cast<NavigationSnapshot*>(payload)));
      this->payload = *(static_cast<NavigationSnapshot*>(payload));
    }
    else if constexpr (std::is_same_v<T, InputHandleData>)
    {
      header.type = static_cast<uint8_t>(PayloadType::InputHandleData);
      header.payload_length = sizeof(*(static_cast<InputHandleData*>(payload)));
      this->payload = *(static_cast<InputHandleData*>(payload));
    }
  };

  /**
   * @brief Creates an empty message with its payload value-initialized.
   *
   * For payload types with a protocol mapping, the header type is initialized
   * accordingly. The payload length remains zero until a payload is assigned
   * or otherwise initialized by the caller.
   */
  Message()
  {
    if constexpr (std::is_same_v<T, NmeaSentences>)
    {
      header.type = static_cast<uint8_t>(PayloadType::NmeaSentences);
    }
    else if constexpr (std::is_same_v<T, NavigationSnapshot>)
    {
      header.type = static_cast<uint8_t>(PayloadType::NavigationSnapshot);
    }
  }
  //}}}

  MessageHeader header{};
  T payload{};
};

// Byte offsets shared by every serialized message. Multi-byte values use
// little-endian byte order. Payload-specific offsets are relative to payload.
struct MessageOffsets
//{{{
{
  static constexpr uint8_t type = 0;
  static constexpr uint8_t payload_length_little_endian = 1;
  static constexpr uint8_t payload_length_big_endian = 2;
  static constexpr uint8_t payload = 3;
};
//}}}

// Byte offsets in an NMEA payload.
struct NmeaPayloadOffsets
//{{{
{
  static constexpr uint8_t sentence_count = 0;
  static constexpr uint8_t sentences = 1;
};
//}}}

struct InputHandlePayloadOffsets
{
  static constexpr uint8_t engage = 0;
  static constexpr uint8_t target_course_little_endian = 1;
  static constexpr uint8_t target_course_big_endian = 2;
  static constexpr uint8_t payload_length = target_course_big_endian + 1;
};

// Byte offsets in a navigation payload. Each sample contains its value,
// validity flag, and timestamp; all multi-byte fields are little-endian.
struct NavigationPayloadOffsets
//{{{
{
  static constexpr uint8_t compass_hdg_dg = 0;
  static constexpr uint8_t gps_cog_dg = compass_hdg_dg + 11;
  static constexpr uint8_t gps_sog_kts = gps_cog_dg + 11;
  static constexpr uint8_t wind_angle_dg = gps_sog_kts + 13;
  static constexpr uint8_t stw_kts = wind_angle_dg + 11;
  static constexpr uint8_t lead_source = stw_kts + 13;
  static constexpr uint8_t payload_length = lead_source + 1;
};
//}}}

// Serializes populated NMEA sentences into write_buffer and returns the number
// of bytes written. The caller must provide space for the 3-byte wire header,
// the sentence count, and 128 bytes for each populated sentence.
/**
 * @brief Serializes an NMEA message into a byte buffer.
 *
 * The serialized frame contains a three-byte header followed by the sentence
 * count and the populated fixed-width sentence buffers.
 *
 * @param dest_buffer Destination buffer for the serialized frame.
 * @param buffer_size Available size of @p dest_buffer in bytes.
 * @param msg Message containing the NMEA sentences to serialize.
 * @return Number of bytes written, or `0` if the buffer is too small.
 */
inline uint16_t mp_write_NmeaMessage_to_bytes(void* dest_buffer, uint16_t buffer_size,
                                              Message<NmeaSentences>* msg)
//{{{
{
  if (buffer_size < sizeof(Message<NmeaSentences>))
  {
    return 0;
  }

  MessageHeader nmeaMessageHeader = {
      .type = static_cast<uint8_t>(PayloadType::NmeaSentences),
      .payload_length =
          static_cast<uint16_t>(NmeaPayloadOffsets::sentences +
                                msg->payload.sentence_count * sizeof(msg->payload.sentence[0]))};

  static_cast<uint8_t*>(dest_buffer)[MessageOffsets::type] =
      static_cast<uint8_t>(nmeaMessageHeader.type);
  static_cast<uint8_t*>(dest_buffer)[MessageOffsets::payload_length_little_endian] =
      static_cast<uint8_t>(nmeaMessageHeader.payload_length);
  static_cast<uint8_t*>(dest_buffer)[MessageOffsets::payload_length_big_endian] =
      static_cast<uint8_t>(nmeaMessageHeader.payload_length << 8);
  static_cast<uint8_t*>(dest_buffer)[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] =
      static_cast<uint8_t>(msg->payload.sentence_count);

  for (int i = 0; i < msg->payload.sentence_count; i++)
  {
    for (int ii = 0; ii < 128; ii++)
    {
      static_cast<uint8_t*>(
          dest_buffer)[MessageOffsets::payload + NmeaPayloadOffsets::sentences + i * 128 + ii] =
          msg->payload.sentence[i][ii];
    }
  }
  return MessageOffsets::payload + NmeaPayloadOffsets::sentences +
         msg->payload.sentence_count * 128;
}
//}}}

// Decodes an NMEA wire frame. buffer_size must be the decoded frame length.
/**
 * @brief Decodes an NMEA message from a byte buffer.
 *
 * The frame type, payload length, sentence count, and available buffer size
 * are validated before the sentences are copied.
 *
 * @param buffer Buffer containing the decoded NMEA frame.
 * @param buffer_size Length of @p buffer in bytes.
 * @return The decoded message, or an empty message if the frame is invalid or
 *         the buffer is too small.
 */
inline Message<NmeaSentences> mp_read_NmeaMessage_from_buffer(void* buffer, uint16_t buffer_size)
//{{{
{
  constexpr uint16_t minimum_message_size = MessageOffsets::payload + NmeaPayloadOffsets::sentences;

  if (buffer_size < minimum_message_size)
  {
    return {};
  }

  const uint8_t* decoded_nmea_message_frame = static_cast<const uint8_t*>(buffer);
  const uint16_t payload_length =
      static_cast<uint16_t>(
          decoded_nmea_message_frame[MessageOffsets::payload_length_little_endian]) |
      (static_cast<uint16_t>(decoded_nmea_message_frame[MessageOffsets::payload_length_big_endian])
       << 8);
  const uint8_t sentence_count =
      decoded_nmea_message_frame[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count];
  const uint16_t expected_payload_length =
      NmeaPayloadOffsets::sentences + sentence_count * sizeof(NmeaSentences::sentence[0]);

  if (decoded_nmea_message_frame[MessageOffsets::type] !=
          static_cast<uint8_t>(PayloadType::NmeaSentences) ||
      sentence_count > 20 || payload_length != expected_payload_length ||
      buffer_size < MessageOffsets::payload + expected_payload_length)
  {
    return {};
  }

  NmeaSentences sentences{};
  sentences.sentence_count = sentence_count;

  for (int i = 0; i < sentence_count; i++)
  {
    for (int ii = 0; ii < 128; ii++)
    {
      sentences.sentence[i][ii] =
          decoded_nmea_message_frame[MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                                     i * 128 + ii];
    }
  }
  return {&sentences};
};
//}}}

// Serializes a navigation snapshot into a wire frame and returns its total
// size. Numeric payload fields are encoded in little-endian byte order.
/**
 * @brief Serializes a navigation snapshot into a byte buffer.
 *
 * Numeric fields and timestamps are encoded in little-endian byte order.
 *
 * @param dest_buffer Destination buffer for the serialized frame.
 * @param buffer_size Available size of @p dest_buffer in bytes.
 * @param msg Message containing the navigation snapshot to serialize.
 * @return Number of bytes written, or `0` if the buffer is too small.
 */
inline uint16_t mp_write_NavigationMessage_to_bytes(void* dest_buffer, uint16_t buffer_size,
                                                    Message<NavigationSnapshot>* msg)
//{{{
{
  if (buffer_size < sizeof(Message<NavigationSnapshot>))
  {
    return 0;
  }

  MessageHeader navigationHeader = {.type = static_cast<uint8_t>(PayloadType::NavigationSnapshot),
                                    .payload_length = NavigationPayloadOffsets::payload_length};

  uint8_t* navigation_message_frame = static_cast<uint8_t*>(dest_buffer);
  navigation_message_frame[MessageOffsets::type] = navigationHeader.type;
  navigation_message_frame[MessageOffsets::payload_length_little_endian] =
      static_cast<uint8_t>(navigationHeader.payload_length);
  navigation_message_frame[MessageOffsets::payload_length_big_endian] =
      static_cast<uint8_t>(navigationHeader.payload_length >> 8);

  auto write_uint16 = [&navigation_message_frame](uint8_t offset, uint16_t value)
  {
    navigation_message_frame[offset] = static_cast<uint8_t>(value);
    navigation_message_frame[offset + 1] = static_cast<uint8_t>(value >> 8);
  };
  auto write_uint64 = [&navigation_message_frame](uint8_t offset, uint64_t value)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      navigation_message_frame[offset + i] = static_cast<uint8_t>(value >> (i * 8));
    }
  };
  auto write_uint32 = [&navigation_message_frame](uint8_t offset, uint32_t value)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      navigation_message_frame[offset + i] = static_cast<uint8_t>(value >> (i * 8));
    }
  };
  auto write_uint16_sample = [&write_uint16, &write_uint64, &navigation_message_frame](
                                 uint8_t offset, SensorSample<uint16_t> sample)
  {
    write_uint16(offset, sample.value);
    navigation_message_frame[offset + 2] = static_cast<uint8_t>(sample.valid);
    write_uint64(offset + 3, sample.timestamp);
  };
  auto write_float_sample = [&write_uint32, &write_uint64,
                             &navigation_message_frame](uint8_t offset, SensorSample<float> sample)
  {
    uint32_t value;
    std::memcpy(&value, &sample.value, sizeof(value));
    write_uint32(offset, value);
    navigation_message_frame[offset + 4] = static_cast<uint8_t>(sample.valid);
    write_uint64(offset + 5, sample.timestamp);
  };

  write_uint16_sample(MessageOffsets::payload + NavigationPayloadOffsets::compass_hdg_dg,
                      msg->payload.compass_hdg_dg);
  write_uint16_sample(MessageOffsets::payload + NavigationPayloadOffsets::gps_cog_dg,
                      msg->payload.gps_cog_dg);
  write_float_sample(MessageOffsets::payload + NavigationPayloadOffsets::gps_sog_kts,
                     msg->payload.gps_sog_kts);
  write_uint16_sample(MessageOffsets::payload + NavigationPayloadOffsets::wind_angle_dg,
                      msg->payload.wind_angle_dg);
  write_float_sample(MessageOffsets::payload + NavigationPayloadOffsets::stw_kts,
                     msg->payload.stw_kts);
  navigation_message_frame[MessageOffsets::payload + NavigationPayloadOffsets::lead_source] =
      static_cast<uint8_t>(msg->payload.LeadSource);

  return MessageOffsets::payload + NavigationPayloadOffsets::payload_length;
}
//}}}

// Decodes a navigation wire frame. buffer_size must be the decoded frame length.
/**
 * @brief Decodes a navigation message from a byte buffer.
 *
 * The frame type and payload length are validated before the snapshot is
 * decoded. Numeric fields and timestamps are interpreted as little-endian.
 *
 * @param buffer Buffer containing the decoded navigation frame.
 * @param buffer_size Length of @p buffer in bytes.
 * @return The decoded message, or an empty message if the frame is invalid or
 *         the buffer is too small.
 */
inline Message<NavigationSnapshot> mp_read_NavigationMessage_from_buffer(void* buffer,
                                                                         uint16_t buffer_size)
//{{{
{
  constexpr uint16_t message_size =
      MessageOffsets::payload + NavigationPayloadOffsets::payload_length;

  if (buffer_size >= message_size)
  {
    const uint8_t* decoded_navigation_message_frame = static_cast<const uint8_t*>(buffer);
    const uint16_t payload_length =
        static_cast<uint16_t>(
            decoded_navigation_message_frame[MessageOffsets::payload_length_little_endian]) |
        (static_cast<uint16_t>(
             decoded_navigation_message_frame[MessageOffsets::payload_length_big_endian])
         << 8);

    if (decoded_navigation_message_frame[MessageOffsets::type] !=
            static_cast<uint8_t>(PayloadType::NavigationSnapshot) ||
        payload_length != NavigationPayloadOffsets::payload_length)
    {
      return {};
    }

    auto read_uint16 = [&decoded_navigation_message_frame](uint8_t offset)
    {
      return static_cast<uint16_t>(decoded_navigation_message_frame[offset]) |
             (static_cast<uint16_t>(decoded_navigation_message_frame[offset + 1]) << 8);
    };
    auto read_uint64 = [&decoded_navigation_message_frame](uint8_t offset)
    {
      uint64_t value = 0;
      for (uint8_t i = 0; i < 8; i++)
      {
        value |= static_cast<uint64_t>(decoded_navigation_message_frame[offset + i]) << (i * 8);
      }
      return value;
    };
    auto read_uint32 = [&decoded_navigation_message_frame](uint8_t offset)
    {
      uint32_t value = 0;
      for (uint8_t i = 0; i < 4; i++)
      {
        value |= static_cast<uint32_t>(decoded_navigation_message_frame[offset + i]) << (i * 8);
      }
      return value;
    };
    auto read_uint16_sample =
        [&read_uint16, &read_uint64, &decoded_navigation_message_frame](uint8_t offset)
    {
      SensorSample<uint16_t> sample{};
      sample.value = read_uint16(offset);
      sample.valid = decoded_navigation_message_frame[offset + 2] != 0;
      sample.timestamp = read_uint64(offset + 3);
      return sample;
    };
    auto read_float_sample =
        [&read_uint32, &read_uint64, &decoded_navigation_message_frame](uint8_t offset)
    {
      SensorSample<float> sample{};
      uint32_t value = read_uint32(offset);
      std::memcpy(&sample.value, &value, sizeof(value));
      sample.valid = decoded_navigation_message_frame[offset + 4] != 0;
      sample.timestamp = read_uint64(offset + 5);
      return sample;
    };

    NavigationSnapshot snapshot{};
    snapshot.compass_hdg_dg =
        read_uint16_sample(MessageOffsets::payload + NavigationPayloadOffsets::compass_hdg_dg);
    snapshot.gps_cog_dg =
        read_uint16_sample(MessageOffsets::payload + NavigationPayloadOffsets::gps_cog_dg);
    snapshot.gps_sog_kts =
        read_float_sample(MessageOffsets::payload + NavigationPayloadOffsets::gps_sog_kts);
    snapshot.wind_angle_dg =
        read_uint16_sample(MessageOffsets::payload + NavigationPayloadOffsets::wind_angle_dg);
    snapshot.stw_kts =
        read_float_sample(MessageOffsets::payload + NavigationPayloadOffsets::stw_kts);
    snapshot.LeadSource = static_cast<NavigationSource>(
        decoded_navigation_message_frame[MessageOffsets::payload +
                                         NavigationPayloadOffsets::lead_source]);

    return {&snapshot};
  }
  // if the given buffer is to small no read operations are done, and the function returns a default
  // initialized Message
  else
  {
    return {};
  }
}
//}}}

// Writes a given InputHandleData Message to a byte buffer
/**
 * @brief Serializes an input-handle message into a byte buffer.
 *
 * The target course is encoded in little-endian byte order.
 *
 * @param dest_buffer Destination buffer for the serialized frame.
 * @param buffer_size Available size of @p dest_buffer in bytes.
 * @param msg Message containing the input-handle data to serialize.
 * @return Number of bytes in the serialized frame.
 */
inline uint16_t mp_write_InputHandleMessage_to_bytes(void* dest_buffer, uint16_t buffer_size,
                                                     Message<InputHandleData>* msg)
//{{{
{
  constexpr uint16_t message_size =
      MessageOffsets::payload + InputHandlePayloadOffsets::payload_length;
  if (buffer_size < message_size)
  {
    return 0;
  }

  uint8_t* inputHandle_message_frame = static_cast<uint8_t*>(dest_buffer);
  inputHandle_message_frame[MessageOffsets::type] =
      static_cast<uint8_t>(PayloadType::InputHandleData);
  inputHandle_message_frame[MessageOffsets::payload_length_little_endian] =
      static_cast<uint8_t>(InputHandlePayloadOffsets::payload_length);
  inputHandle_message_frame[MessageOffsets::payload_length_big_endian] = 0;

  inputHandle_message_frame[MessageOffsets::payload + InputHandlePayloadOffsets::engage] =
      msg->payload.engage;
  inputHandle_message_frame[MessageOffsets::payload +
                            InputHandlePayloadOffsets::target_course_little_endian] =
      msg->payload.target_course;
  inputHandle_message_frame[MessageOffsets::payload +
                            InputHandlePayloadOffsets::target_course_big_endian] =
      static_cast<uint8_t>(msg->payload.target_course >> 8);

  return message_size;
};
//}}}

/**
 * @brief Decodes an input-handle message from a byte buffer.
 *
 * The frame type and payload length are checked before the input-handle data
 * is decoded.
 *
 * @param buffer Buffer containing the decoded input-handle frame.
 * @param buffer_size Length of @p buffer in bytes.
 * @return The decoded message, or an empty message if the frame is invalid or
 *         the buffer is too small.
 */
inline Message<InputHandleData> mp_read_InputHandleMessage_from_buffer(void* buffer,
                                                                       uint16_t buffer_size)
//{{{
{
  uint16_t message_size = MessageOffsets::payload + InputHandlePayloadOffsets::payload_length;
  if (buffer_size < message_size)
  {
    return {};
  }

  uint8_t* frame = static_cast<uint8_t*>(buffer);

  uint16_t payload_length =
      (static_cast<uint16_t>(frame[MessageOffsets::payload_length_little_endian]) |
       (static_cast<uint16_t>(frame[MessageOffsets::payload_length_big_endian]) << 8));

  if (frame[MessageOffsets::type] != static_cast<uint8_t>(PayloadType::InputHandleData) ||
      payload_length != InputHandlePayloadOffsets::payload_length)
  {
    return {};
  }

  InputHandleData data{};
  data.engage = frame[MessageOffsets::payload + InputHandlePayloadOffsets::engage];
  data.target_course =
      (static_cast<uint16_t>(frame[MessageOffsets::payload +
                                  InputHandlePayloadOffsets::target_course_little_endian]) |
       (static_cast<uint16_t>(frame[MessageOffsets::payload +
                                   InputHandlePayloadOffsets::target_course_big_endian]) << 8));

  return {&data};
};
//}}}
