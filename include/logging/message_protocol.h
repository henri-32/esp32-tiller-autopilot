#pragma once

#include "types/sensorTypes.h"
#include <cstdint>
#include <cstring>
#include <type_traits>

// In-memory representation of a message header. Do not serialize this struct
// directly: its layout may contain compiler padding.
struct MessageHeader
//{{{
{
  uint8_t type;
  uint16_t payload_length;
};
//}}}

// Navigation values and their validity/timestamp metadata.
struct NavigationSnapshot
//{{{
{
  SensorSample<uint16_t> compass_hdg_dg;
  SensorSample<uint16_t> gps_cog_dg;
  SensorSample<float> gps_sog_kts;
  SensorSample<uint16_t> wind_angle_dg;
  SensorSample<float> stw_kts;
  NavigationSource LeadSource;
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

// Payload types supported by the logging message protocol.
enum class PayloadType : uint8_t
{
  NmeaSentences = 1,
  NavigationSnapshot = 2
};

// In-memory representation of a typed message. The wire format is defined by
// MessageOffsets and the corresponding payload-offset structure below.
//
// The constructor initializes the in-memory header and payload from payload.
template <typename T> struct Message
//{{{
{
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
  };

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

  MessageHeader header;
  T payload;
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
inline uint16_t mp_write_NmeaMessage_to_bytes(void* write_buffer, Message<NmeaSentences>* msg)
//{{{
{

  MessageHeader nmeaMessageHeader = {.type = static_cast<uint8_t>(PayloadType::NmeaSentences),
                                     .payload_length = static_cast<uint16_t>(
                                         NmeaPayloadOffsets::sentences +
                                         msg->payload.sentence_count * sizeof(msg->payload.sentence[0]))};

  static_cast<uint8_t*>(write_buffer)[MessageOffsets::type] =
      static_cast<uint8_t>(nmeaMessageHeader.type);
  static_cast<uint8_t*>(write_buffer)[MessageOffsets::payload_length_little_endian] =
      static_cast<uint8_t>(nmeaMessageHeader.payload_length);
  static_cast<uint8_t*>(write_buffer)[MessageOffsets::payload_length_big_endian] =
      static_cast<uint8_t>(nmeaMessageHeader.payload_length << 8);
  static_cast<uint8_t*>(write_buffer)[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] =
      static_cast<uint8_t>(msg->payload.sentence_count);

  for (int i = 0; i < msg->payload.sentence_count; i++)
  {
    for (int ii = 0; ii < 128; ii++)
    {
      static_cast<uint8_t*>(write_buffer)[MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                                           i * 128 + ii] = msg->payload.sentence[i][ii];
    }
  }
  return MessageOffsets::payload + NmeaPayloadOffsets::sentences +
         msg->payload.sentence_count * 128;
}
//}}}

// Decodes a validated NMEA wire frame from decoded_nmea_message_ptr. Sentence
// counts above the protocol maximum are capped at 20.
inline Message<NmeaSentences> mp_read_NmeaMessage_from_buffer(void* decoded_nmea_message_ptr)
//{{{
{
  const uint8_t* decoded_nmea_message_frame =
      static_cast<const uint8_t*>(decoded_nmea_message_ptr);
  uint8_t sentence_count = decoded_nmea_message_frame[MessageOffsets::payload +
                                                      NmeaPayloadOffsets::sentence_count];

  if (sentence_count > 20)
  {
    sentence_count = 20;
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
inline uint16_t mp_write_NavigationMessage_to_bytes(void* write_buffer, Message<NavigationSnapshot>* msg)
//{{{
{
  MessageHeader navigationHeader = {.type = static_cast<uint8_t>(PayloadType::NavigationSnapshot),
                                    .payload_length = NavigationPayloadOffsets::payload_length};

  uint8_t* navigation_message_frame = static_cast<uint8_t*>(write_buffer);
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

// Decodes a validated navigation wire frame from decoded_navigation_message_ptr.
inline Message<NavigationSnapshot>
mp_read_NavigationMessage_from_buffer(void* decoded_navigation_message_ptr)
//{{{
{
  uint8_t decoded_navigation_message_frame[MessageOffsets::payload +
                                           NavigationPayloadOffsets::payload_length];
  for (uint8_t i = 0; i < sizeof(decoded_navigation_message_frame); i++)
  {
    decoded_navigation_message_frame[i] = static_cast<uint8_t*>(decoded_navigation_message_ptr)[i];
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
      decoded_navigation_message_frame[MessageOffsets::payload + NavigationPayloadOffsets::lead_source]);


  return {&snapshot};
}
//}}}
