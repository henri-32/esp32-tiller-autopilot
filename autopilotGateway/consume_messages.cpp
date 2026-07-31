#include "consume_messages.h"
#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>

void consume_telemetry_log_msg(int fd_display, sockaddr_un* display_addr,
                               const Message<AccumulatedLogMessage>* msg)
//{{{
{
  uint8_t raw_msg[MessageOffsets::payload + AccumulatedLogMessagePayloadOffsets::payload_length];
  const uint16_t size = mp_write_AccumulatedLogMessage_to_bytes(raw_msg, sizeof(raw_msg), msg);
  if (size == 0)
  {
    return;
  }

  uint8_t msg_encoded[COBS_ENCODE_DST_BUF_LEN_MAX(sizeof(raw_msg))];

  cobs_encode_result encode_res = cobs_encode(msg_encoded, sizeof(msg_encoded), raw_msg, size);

  if (encode_res.status == COBS_ENCODE_OK)
  {
    int send = sendto(fd_display, msg_encoded, encode_res.out_len, MSG_DONTWAIT,
                      reinterpret_cast<const sockaddr*>(display_addr), sizeof(*display_addr));

    if (send < 0 && errno != ENOENT)
    {
      perror("send telemetry to monitor");
    }
  }
};
//}}}

void consume_nmea_msg(int fd_opencpn, sockaddr_in* opencpn_addr, const Message<NmeaSentences>* msg)
//{{{
{
  for (int i = 0; i < msg->payload.sentence_count; ++i)
  {

    size_t length = strnlen(msg->payload.sentence[i], sizeof(msg->payload.sentence[i]));

    ssize_t send = sendto(fd_opencpn, msg->payload.sentence[i], length, MSG_DONTWAIT,
                          reinterpret_cast<const sockaddr*>(opencpn_addr), sizeof(*opencpn_addr));

    if (send < 0)
    {
      perror("send to udp port from consume_nmea_sentences");
      break;
    }
  }
}
//}}}
