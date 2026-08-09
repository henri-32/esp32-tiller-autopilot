#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
#include "socket_handling.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using std::string;

// Creates a consistently styled value box for the monitor grid.
Fl_Box* make_box(int x, int y, int w, int h)
//{{{
{
  auto* box = new Fl_Box(x, y, w, h, "Waiting for source data");
  box->box(FL_UP_BOX);
  box->labelfont(FL_ITALIC);
  box->labelsize(20);
  box->labeltype(FL_NORMAL_LABEL);
  return box;
}
//}}}

//===================================================================== MAIN WINDOW ==============================================================
// Shared state read and updated by the UI and socket callbacks.
struct WindowContext
//{{{
{
  AccumulatedLogMessage ALM;
  InputHandleData_t ihData{};
  bool ihData_synced = false;
};
//}}}
class Window : public Fl_Window
//{{{
{
public:
  // Builds the window around externally owned application state.
  Window(int x, int y, const char* name, WindowContext* context)
      : Fl_Window(x, y, name), ctx_(context)
  {
    make_boxes();
  };

  // Handles keyboard input for changing the target course.
  int handle(int event) override
  //{{{
  {

    if (event != FL_KEYDOWN && event != FL_SHORTCUT)
    {
      return Fl_Window::handle(event);
    }

    if (Fl::event_key() == '+')
    {
      if (++ctx_->ihData.target_course >= 360)
      {
        ctx_->ihData.target_course %= 360;
      }
      ctx_->ihData_synced = false;
      refresh_target_course_box();
      return 1;
    }
    else if (Fl::event_key() == '-')
    {
      if (--ctx_->ihData.target_course < 0)
      {
        ctx_->ihData.target_course += 360;
      }
      ctx_->ihData_synced = false;
      refresh_target_course_box();
      return 1;
    }
    return Fl_Window::handle(event);
  };
  //}}}

  // Refreshes all telemetry boxes from the current window context.
  void display_AccumulatedLogMessage_values()
  //{{{
  {
    AccumulatedLogMessage msg = ctx_->ALM;

    const auto snapshot = msg.snapshot;
    const auto sog = msg.snapshot.gps_sog_kts;
    const auto cog = msg.snapshot.gps_cog_dg;

    //================================ Check if ihData is synced to autopilot ================
    if (ctx_->ihData.steering_engaged == snapshot.steering_engaged &&
        ctx_->ihData.target_course == snapshot.target_course)
    {
      ctx_->ihData_synced = true;
    }
    else
    {
      ctx_->ihData_synced = false;
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
    sog_box_->copy_label(text);
    sog_box_->redraw();

    //================================ Print COG Box =========================================
    if (cog.valid)
    {
      std::snprintf(text, sizeof(text), "COG: %d deg", cog.value);
    }
    else
    {
      std::snprintf(text, sizeof(text), "COG: invalid");
    }
    cog_box_->copy_label(text);
    cog_box_->redraw();

    //================================ Print LAT/LON Box =========================================
    if (msg.error.validFix)
    {
      format_coordinate_for_display(text, sizeof(text), "LAT", snapshot.gps_lat, 2, 'N', 'S');
    }
    else
    {
      std::snprintf(text, sizeof(text), "LAT: invalid");
    }
    latitude_box_->copy_label(text);
    latitude_box_->redraw();

    if (msg.error.validFix)
    {
      format_coordinate_for_display(text, sizeof(text), "LON", snapshot.gps_lon, 3, 'E', 'W');
    }
    else
    {
      std::snprintf(text, sizeof(text), "LON: invalid");
    }
    longitude_box_->copy_label(text);
    longitude_box_->redraw();

    //================================ Print Target Box =========================================
    refresh_target_course_box();
  }
  //}}}

private:
  // Creates and arranges the monitor's value boxes.
  void make_boxes()
  //{{{
  {
    static auto box = [&](int col, int row)
    {
      return make_box(margin_ + col * (width_ + gap_), margin_ + row * (height_ + gap_), width_,
                      height_);
    };

    sog_box_ = box(0, 0);
    cog_box_ = box(1, 0);
    target_box_ = box(2, 0);
    latitude_box_ = box(0, 1);
    longitude_box_ = box(1, 1);

    add(sog_box_);
    add(cog_box_);
    add(target_box_);
    add(latitude_box_);
    add(longitude_box_);

    show();
  }
  //}}}

  // Formats decimal degrees as degrees and minutes with a hemisphere.
  void format_coordinate_for_display(char* text, std::size_t text_size, const char* label,
                                     float decimal_degrees, int degree_width,
                                     char positive_hemisphere, char negative_hemisphere)
//{{{
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
//}}}
  // Updates the target-course box to reflect the current synchronization state.
  void refresh_target_course_box()
  //{{{
  {
    char text[30];
    if (ctx_->ihData_synced)
    {
      std::snprintf(text, sizeof(text), "TARGET: %d deg \n", ctx_->ihData.target_course);
    }
    else
    {
      std::snprintf(text, sizeof(text), "TARGET: %d deg sync \n", ctx_->ihData.target_course);
    }
    target_box_->copy_label(text);
    target_box_->redraw();
  }
  //}}}

  WindowContext* ctx_;
  static constexpr int margin_ = 15;
  static constexpr int gap_ = 10;
  static constexpr int width_ = 280;
  static constexpr int height_ = 60;

  Fl_Box* sog_box_;
  Fl_Box* cog_box_;
  Fl_Box* target_box_;
  Fl_Box* latitude_box_;
  Fl_Box* longitude_box_;
};
//}}}
//====================================================================================================================================================



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
void cb_datagram_ready(int fd, void* pvParameters)
//{{{
{
  cb_datagram_ready_ctx* ctx = static_cast<cb_datagram_ready_ctx*>(pvParameters);
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
void send_ap_input(void* context)
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

  Fl::repeat_timeout(0.5, send_ap_input, context);
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
  Fl::add_timeout(0.5, send_ap_input, &send_ctx);

  int ret = Fl::run();

  cleanup_sockets(s_context);
  return ret;
}
//}}}
