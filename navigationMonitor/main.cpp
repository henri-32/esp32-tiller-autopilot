#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
#include "socket_handling.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using std::string;

// TODO Propably should not be global
static InputHandleData_t ihData{};
static bool ihData_synced = false;

struct fltk_handle
{
  Fl_Window* window;
  Fl_Box* sog_box;
  Fl_Box* cog_box;
  Fl_Box* target_cog_box;
  Fl_Box* latitude_box;
  Fl_Box* longitude_box;
};

Fl_Box* make_box(int x, int y, int w, int h)
{
  auto* box = new Fl_Box(x, y, w, h, "Waiting for source data");
  box->box(FL_UP_BOX);
  box->labelfont(FL_ITALIC);
  box->labelsize(20);
  box->labeltype(FL_NORMAL_LABEL);
  return box;
}

void refresh_target_box(fltk_handle handle)
{
  char text[30];
  if (ihData_synced)
  {
    std::snprintf(text, sizeof(text), "TARGET: %d deg \n", ihData.target_course);
  }
  else
  {
    std::snprintf(text, sizeof(text), "TARGET: %d deg sync \n", ihData.target_course);
  }
  handle.target_cog_box->copy_label(text);
  handle.target_cog_box->redraw();
}

struct WindowContext
{
};

class Window : public Fl_Window
{
public:
  Window(int x, int y, const char* name) : Fl_Window(x, y, name) {};

  void set_handle(fltk_handle handle)
  {
    handle_ = handle;
  }

  int handle(int event) override
  {

    if (event != FL_KEYDOWN && event != FL_SHORTCUT)
    {
      return Fl_Window::handle(event);
    }

    if (Fl::event_key() == '+')
    {
      if (++ihData.target_course >= 360)
      {
        ihData.target_course %= 360;
      }
      ihData_synced = false;
      refresh_target_box(handle_);
      return 1;
    }
    else if (Fl::event_key() == '-')
    {
      if (--ihData.target_course < 0)
      {
        ihData.target_course += 360;
      }
      ihData_synced = false;
      refresh_target_box(handle_);
      return 1;
    }
    return Fl_Window::handle(event);
  };

private:
  fltk_handle handle_{};
  WindowContext ctx_;
};

fltk_handle fltk_setup()
{
  static Window* window = new Window(900, 250, "Autopilot source data");
  constexpr int margin = 15;
  constexpr int gap = 10;
  constexpr int width = 280;
  constexpr int height = 60;
  auto box = [&](int col, int row)
  { return make_box(margin + col * (width + gap), margin + row * (height + gap), width, height); };

  auto* sog = box(0, 0);
  auto* cog = box(1, 0);
  auto* target_cog = box(2, 0);
  auto* latitude = box(0, 1);
  auto* longitude = box(1, 1);

  window->end();

  fltk_handle handle{.window = window,
                     .sog_box = sog,
                     .cog_box = cog,
                     .target_cog_box = target_cog,
                     .latitude_box = latitude,
                     .longitude_box = longitude};
  window->set_handle(handle);
  window->show();
  return handle;
};

void format_coordinate_for_display(char* text, std::size_t text_size, const char* label,
                                   float decimal_degrees, int degree_width,
                                   char positive_hemisphere, char negative_hemisphere)
{
  const double absolute_degrees = std::fabs(static_cast<double>(decimal_degrees));
  int degrees = static_cast<int>(absolute_degrees);
  double decimal_minutes = (absolute_degrees - degrees) * 60.0;

  // Keep rounding from ever producing an invalid "60.000 minutes" display.
  decimal_minutes = std::round(decimal_minutes * 1000.0) / 1000.0;
  if (decimal_minutes >= 60.0)
  {
    decimal_minutes = 0.0;
    ++degrees;
  }

  const char hemisphere = std::signbit(decimal_degrees) ? negative_hemisphere : positive_hemisphere;
  std::snprintf(text, text_size, "%s: %0*d° %06.3f' %c", label, degree_width, degrees,
                decimal_minutes, hemisphere);
}

void process_AccumulatedLogMessage(const void* data, uint16_t size, fltk_handle handle)
//{{{
{
  const Message<AccumulatedLogMessage> msg = mp_read_AccumulatedLogMessage_from_buffer(data, size);
  if (msg.header.type != static_cast<uint8_t>(PayloadType::AccumulatedLogMessage))
  {
    return;
  }

  const auto snapshot = msg.payload.snapshot;
  const auto sog = msg.payload.snapshot.gps_sog_kts;
  const auto cog = msg.payload.snapshot.gps_cog_dg;

  //================================ Check if ihData is synced to autopilot ================
  if (ihData.steering_engaged == snapshot.steering_engaged &&
      ihData.target_course == snapshot.target_course)
  {
    ihData_synced = true;
  }
  else
  {
    ihData_synced = false;
  }

  char text[128];

  //================================ Print SOG Box =========================================
  if (sog.valid)
  {
    std::snprintf(text, sizeof(text), "SOG: %.1f kts", sog.value);
  }
  else
  {
    std::snprintf(text, sizeof(text), "SOG: invalid");
  }
  handle.sog_box->copy_label(text);
  handle.sog_box->redraw();

  //================================ Print COG Box =========================================
  if (cog.valid)
  {
    std::snprintf(text, sizeof(text), "COG: %d deg", cog.value);
  }
  else
  {
    std::snprintf(text, sizeof(text), "COG: invalid");
  }
  handle.cog_box->copy_label(text);
  handle.cog_box->redraw();

  //================================ Print LAT/LON Box =========================================
  if (msg.payload.error.validFix)
  {
    format_coordinate_for_display(text, sizeof(text), "LAT", snapshot.gps_lat, 2, 'N', 'S');
  }
  else
  {
    std::snprintf(text, sizeof(text), "LAT: invalid");
  }
  handle.latitude_box->copy_label(text);
  handle.latitude_box->redraw();

  if (msg.payload.error.validFix)
  {
    format_coordinate_for_display(text, sizeof(text), "LON", snapshot.gps_lon, 3, 'E', 'W');
  }
  else
  {
    std::snprintf(text, sizeof(text), "LON: invalid");
  }
  handle.longitude_box->copy_label(text);
  handle.longitude_box->redraw();

  //================================ Print Target Box =========================================
  refresh_target_box(handle);
}
//}}}

void process_datagram(const void* data, uint16_t size, fltk_handle handle)
//{{{
{
  if (data == nullptr || size == 0)
  {
    return;
  }

  std::vector<uint8_t> decoded(size);
  const cobs_decode_result decode_result = cobs_decode(decoded.data(), decoded.size(), data, size);
  if (decode_result.status != COBS_DECODE_OK || decode_result.out_len < MessageOffsets::payload)
  {
    return;
  }

  switch (decoded[MessageOffsets::type])
  {
  case static_cast<uint8_t>(PayloadType::AccumulatedLogMessage):
    process_AccumulatedLogMessage(decoded.data(), static_cast<uint16_t>(decode_result.out_len),
                                  handle);
    break;

  default:
    break;
  }
}
//}}}

struct cb_datagram_ready_ctx
{
  fltk_handle fl_handle;
};

void cb_datagram_ready(int fd, void* pvParameters)
//{{{
{
  cb_datagram_ready_ctx* context = static_cast<cb_datagram_ready_ctx*>(pvParameters);

  char buffer[2048];

  const ssize_t received = recvfrom(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT, nullptr, nullptr);
  if (received > 0)
  {
    process_datagram(buffer, received, context->fl_handle);
  }
  else
  {
    perror("receive Datagram");
  }
}
//}}}

struct send_ap_input_ctx
{
  socket_context s_ctx;
  InputHandleData_t* ihData;
};

void send_ap_input(void* context)
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

  Fl::repeat_timeout(0.5, send_ap_input, context);
}
//==============================================================================
int main()
{
  fltk_handle fl_handle = fltk_setup();
  socket_context s_context = get_socket_context();
  if (s_context.success == false)
  {
    return 1;
  }

  cb_datagram_ready_ctx dgram_context{.fl_handle = fl_handle};
  send_ap_input_ctx send_ctx{.s_ctx = s_context, .ihData = &ihData};

  Fl::add_fd(s_context.local_fd, FL_READ, cb_datagram_ready, &dgram_context);
  Fl::add_timeout(0.5, send_ap_input, &send_ctx);

  int ret = Fl::run();

  cleanup_sockets(s_context);
  return ret;
}
