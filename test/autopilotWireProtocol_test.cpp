#include "protocol/autopilotWireProtocol.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace
{
void telemetry_log_message_round_trips()
{
  AccumulatedLogMessage telemetry{};
  telemetry.snapshot.compass_hdg_dg = {.value = 265, .valid = true, .timestamp = 123456700};
  telemetry.snapshot.gps_cog_dg = {.value = 271, .valid = true, .timestamp = 123456789};
  telemetry.snapshot.gps_sog_kts = {.value = 6.75F, .valid = true, .timestamp = 987654321};
  telemetry.snapshot.target_course = 284;
  telemetry.snapshot.gps_lat = 54.321F;
  telemetry.snapshot.gps_lon = 10.123F;
  telemetry.snapshot.wind_angle_dg = {.value = 42, .valid = true, .timestamp = 123456799};
  telemetry.snapshot.stw_kts = {.value = 5.25F, .valid = false, .timestamp = 987654399};
  telemetry.snapshot.lead_source = NavigationSource::Gps;
  telemetry.snapshot.steering_engaged = true;
  telemetry.error.validFix = true;
  telemetry.runtime_log.gps.init_status = static_cast<uint8_t>(InitStatus::OK);
  telemetry.runtime_log.gps.free_task_stack = static_cast<size_t>(12345);
  telemetry.runtime_log.tHub.init_status = static_cast<uint8_t>(InitStatus::FAIL);
  telemetry.runtime_log.tHub.free_task_stack = static_cast<size_t>(67890);
  telemetry.runtime_log.inputHandler.init_status = static_cast<uint8_t>(InitStatus::OK);
  telemetry.runtime_log.inputHandler.free_task_stack = static_cast<size_t>(54321);

  Message<AccumulatedLogMessage> message{&telemetry};
  constexpr size_t message_size =
      MessageOffsets::payload + AccumulatedLogMessagePayloadOffsets::payload_length;
  std::array<uint8_t, message_size> bytes{};

  assert(mp_write_AccumulatedLogMessage_to_bytes(bytes.data(), bytes.size() - 1, &message) == 0);
  assert(mp_write_AccumulatedLogMessage_to_bytes(bytes.data(), bytes.size(), &message) ==
         message_size);
  assert(bytes[MessageOffsets::type] == static_cast<uint8_t>(PayloadType::AccumulatedLogMessage));
  assert(wire_detail::read_uint16(bytes.data(), MessageOffsets::payload_length_little_endian) ==
         AccumulatedLogMessagePayloadOffsets::payload_length);

  const Message<AccumulatedLogMessage> decoded =
      mp_read_AccumulatedLogMessage_from_buffer(bytes.data(), bytes.size());
  assert(decoded.header.type == static_cast<uint8_t>(PayloadType::AccumulatedLogMessage));
  assert(decoded.header.payload_length == AccumulatedLogMessagePayloadOffsets::payload_length);
  assert(decoded.payload.snapshot.compass_hdg_dg.value == telemetry.snapshot.compass_hdg_dg.value);
  assert(decoded.payload.snapshot.compass_hdg_dg.valid == telemetry.snapshot.compass_hdg_dg.valid);
  assert(decoded.payload.snapshot.compass_hdg_dg.timestamp ==
         telemetry.snapshot.compass_hdg_dg.timestamp);
  assert(decoded.payload.snapshot.gps_cog_dg.value == telemetry.snapshot.gps_cog_dg.value);
  assert(decoded.payload.snapshot.gps_cog_dg.valid == telemetry.snapshot.gps_cog_dg.valid);
  assert(decoded.payload.snapshot.gps_cog_dg.timestamp == telemetry.snapshot.gps_cog_dg.timestamp);
  assert(decoded.payload.snapshot.gps_sog_kts.value == telemetry.snapshot.gps_sog_kts.value);
  assert(decoded.payload.snapshot.gps_sog_kts.valid == telemetry.snapshot.gps_sog_kts.valid);
  assert(decoded.payload.snapshot.gps_sog_kts.timestamp ==
         telemetry.snapshot.gps_sog_kts.timestamp);
  assert(decoded.payload.snapshot.target_course == telemetry.snapshot.target_course);
  assert(decoded.payload.snapshot.gps_lat == telemetry.snapshot.gps_lat);
  assert(decoded.payload.snapshot.gps_lon == telemetry.snapshot.gps_lon);
  assert(decoded.payload.snapshot.wind_angle_dg.value == telemetry.snapshot.wind_angle_dg.value);
  assert(decoded.payload.snapshot.wind_angle_dg.valid == telemetry.snapshot.wind_angle_dg.valid);
  assert(decoded.payload.snapshot.wind_angle_dg.timestamp ==
         telemetry.snapshot.wind_angle_dg.timestamp);
  assert(decoded.payload.snapshot.stw_kts.value == telemetry.snapshot.stw_kts.value);
  assert(decoded.payload.snapshot.stw_kts.valid == telemetry.snapshot.stw_kts.valid);
  assert(decoded.payload.snapshot.stw_kts.timestamp == telemetry.snapshot.stw_kts.timestamp);
  assert(decoded.payload.snapshot.lead_source == telemetry.snapshot.lead_source);
  assert(decoded.payload.snapshot.steering_engaged == telemetry.snapshot.steering_engaged);
  assert(decoded.payload.error.validFix == telemetry.error.validFix);
  assert(decoded.payload.runtime_log.gps.init_status == telemetry.runtime_log.gps.init_status);
  assert(decoded.payload.runtime_log.gps.free_task_stack ==
         telemetry.runtime_log.gps.free_task_stack);
  assert(decoded.payload.runtime_log.tHub.init_status == telemetry.runtime_log.tHub.init_status);
  assert(decoded.payload.runtime_log.tHub.free_task_stack ==
         telemetry.runtime_log.tHub.free_task_stack);
  assert(decoded.payload.runtime_log.inputHandler.init_status ==
         telemetry.runtime_log.inputHandler.init_status);
  assert(decoded.payload.runtime_log.inputHandler.free_task_stack ==
         telemetry.runtime_log.inputHandler.free_task_stack);

  bytes[MessageOffsets::type] = 0xff;
  assert(mp_read_AccumulatedLogMessage_from_buffer(bytes.data(), bytes.size()).header.type == 0);
}

void nmea_message_uses_variable_payload_length()
{
  NmeaSentences sentences{};
  sentences.sentence_count = 3;
  std::strcpy(sentences.sentence[0], "$GPGGA,first");
  std::strcpy(sentences.sentence[1], "$GPVTG,second");
  std::strcpy(sentences.sentence[2], "$GPRMC,third");

  Message<NmeaSentences> message{&sentences};
  constexpr size_t buffer_size = MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                                 NmeaSentences::max_sentence_count * NmeaSentences::sentence_length;
  std::array<uint8_t, buffer_size> bytes{};

  const uint16_t written = mp_write_NmeaMessage_to_bytes(bytes.data(), bytes.size(), &message);
  const uint16_t expected_payload_length =
      NmeaPayloadOffsets::sentences + sentences.sentence_count * NmeaSentences::sentence_length;
  assert(written == MessageOffsets::payload + expected_payload_length);
  assert(bytes[MessageOffsets::payload_length_little_endian] ==
         static_cast<uint8_t>(expected_payload_length));
  assert(bytes[MessageOffsets::payload_length_big_endian] ==
         static_cast<uint8_t>(expected_payload_length >> 8));

  const Message<NmeaSentences> decoded = mp_read_NmeaMessage_from_buffer(bytes.data(), written);
  assert(decoded.header.type == static_cast<uint8_t>(PayloadType::NmeaSentences));
  assert(decoded.payload.sentence_count == sentences.sentence_count);
  assert(std::strcmp(decoded.payload.sentence[0], sentences.sentence[0]) == 0);
  assert(std::strcmp(decoded.payload.sentence[1], sentences.sentence[1]) == 0);
  assert(std::strcmp(decoded.payload.sentence[2], sentences.sentence[2]) == 0);
}

void input_command_still_round_trips()
{
  InputHandleData_t input{.steering_engaged = true, .target_course = 359};
  Message<InputHandleData_t> message{&input};
  std::array<uint8_t, MessageOffsets::payload + InputHandlePayloadOffsets::payload_length> bytes{};

  assert(mp_write_InputHandleMessage_to_bytes(bytes.data(), bytes.size(), &message) ==
         bytes.size());
  const Message<InputHandleData_t> decoded =
      mp_read_InputHandleMessage_from_buffer(bytes.data(), bytes.size());
  assert(decoded.header.type == static_cast<uint8_t>(CommandType::InputHandleData));
  assert(decoded.payload.steering_engaged == input.steering_engaged);
  assert(decoded.payload.target_course == input.target_course);
}
} // namespace

int main()
{
  telemetry_log_message_round_trips();
  nmea_message_uses_variable_payload_length();
  input_command_still_round_trips();
}
