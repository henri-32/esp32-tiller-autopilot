#include "consume_messages.h"
#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>

void consume_nav_msg(int fd_display, sockaddr_un* display_addr, Message<NavigationSnapshot>* msg)
//{{{
{
  uint8_t raw_msg[sizeof(Message<NavigationSnapshot>)];
  int size = mp_write_NavigationMessage_to_bytes(raw_msg, sizeof(raw_msg), msg);
  uint8_t msg_encoded[size + size / 256 + 1];

  cobs_encode_result encode_res = cobs_encode(&msg_encoded, sizeof(msg_encoded), raw_msg, size);

  if (encode_res.status == COBS_ENCODE_OK)
  {
    int send = sendto(fd_display, msg_encoded, encode_res.out_len, MSG_DONTWAIT,
                      reinterpret_cast<const sockaddr*>(display_addr), sizeof(*display_addr));

    if (send < 0 && errno != ENOENT)
    {
      perror("send to unix socket from consume_nav_msg");
    }
  }
};
//}}}

void consume_nmea_msg(int fd_opencpn, sockaddr_in* opencpn_addr, Message<NmeaSentences>* msg)
//{{{
{
  for (int i = 0; i < msg->payload.sentence_count; ++i)
  {

    size_t length =
        strnlen(msg->payload.sentence[i], sizeof(msg->payload.sentence[i]));

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
