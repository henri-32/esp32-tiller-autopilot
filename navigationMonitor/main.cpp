#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
#include "socket_handling.h"
#include "window_handling.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <cstdio>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using std::string;
// Decodes an accumulated-log datagram and stores its payload.
bool encode_and_store_datagram(const void* data, uint16_t size,
                               AccumulatedLogMessage* msg_destination)
//{{{
{
  if (data == nullptr || size == 0)
  {
    return false;
  }

  std::vector<uint8_t> decoded(size);
  const cobs_decode_result decode_result = cobs_decode(decoded.data(), decoded.size(), data, size);
  if (decode_result.status != COBS_DECODE_OK || decode_result.out_len < MessageOffsets::payload)
  {
    return false;
  }

  switch (decoded[MessageOffsets::type])
  {
  case static_cast<uint8_t>(PayloadType::AccumulatedLogMessage):
  {
    Message<AccumulatedLogMessage> msg =
        mp_read_AccumulatedLogMessage_from_buffer(decoded.data(), decode_result.out_len);
    *msg_destination = msg.payload;
    return true;
    break;
  }

  default:
    return false;
    break;
  }
}
//}}}

struct cb_datagram_ready_ctx
//{{{
{
  Window* window;
  AccumulatedLogMessage* msg_destination;
};
//}}}
// Receives one datagram, updates the shared state, and refreshes the window.
void cb_datagram_ready(int fd, void* context)
//{{{
{
  cb_datagram_ready_ctx* ctx = static_cast<cb_datagram_ready_ctx*>(context);
  char dgram_buffer[2048];

  const ssize_t received =
      recvfrom(fd, dgram_buffer, sizeof(dgram_buffer) - 1, MSG_DONTWAIT, nullptr, nullptr);
  if (received > 0)
  {
    if (encode_and_store_datagram(dgram_buffer, received, ctx->msg_destination))
    {
      ctx->window->display_AccumulatedLogMessage_values();
    }
  }
  else
  {
    perror("receive Datagram");
  }
}
//}}}

struct send_ap_input_ctx
//{{{
{
  socket_context s_ctx;
  InputHandleData_t* ihData;
};
//}}}
// Sends the current autopilot input and schedules the next transmission.
void cb_send_ap_input(void* context)
//{{{
{
  send_ap_input_ctx* ctx = static_cast<send_ap_input_ctx*>(context);

  Message<InputHandleData_t> msg{ctx->ihData};

  uint8_t msg_buffer[MessageOffsets::payload + InputHandlePayloadOffsets::payload_length];
  const uint16_t msg_size =
      mp_write_InputHandleMessage_to_bytes(msg_buffer, sizeof(msg_buffer), &msg);

  if (msg_size > 0)
  {
    if (sendto(ctx->s_ctx.local_fd, msg_buffer, msg_size, MSG_DONTWAIT,
               reinterpret_cast<const sockaddr*>(&ctx->s_ctx.gateway_addr),
               ctx->s_ctx.gateway_addr_len) < 0)
    {
      perror("send input to gateway");
    }
  }

  Fl::repeat_timeout(0.5, cb_send_ap_input, context);
}
//}}}
//==============================================================================

// Initializes the UI and socket callbacks, then starts the FLTK event loop.
int main()
//{{{
{
  WindowContext wdw_ctx{};

  Window* window = new Window(900, 250, "Navigation Monitor", &wdw_ctx);
  socket_context s_context = get_socket_context();
  if (s_context.success == false)
  {
    return 1;
  }

  cb_datagram_ready_ctx dgram_context{.window = window, .msg_destination = &wdw_ctx.ALM};
  send_ap_input_ctx send_ctx{.s_ctx = s_context, .ihData = &wdw_ctx.ihData};

  Fl::add_fd(s_context.local_fd, FL_READ, cb_datagram_ready, &dgram_context);
  Fl::add_timeout(4, cb_send_ap_input, &send_ctx);

  int ret = Fl::run();

  cleanup_sockets(s_context);
  return ret;
}
//}}}
