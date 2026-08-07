#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
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
static bool ihData_changed = false;
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

fltk_handle fltk_setup()
{
  static Fl_Window* window = new Fl_Window(900, 250, "Autopilot source data");
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
  window->show();
  return {.window = window,
          .sog_box = sog,
          .cog_box = cog,
          .target_cog_box = target_cog,
          .latitude_box = latitude,
          .longitude_box = longitude};
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

void process_AccumulatedLogMessage(const void* data, uint16_t size, fltk_handle handle)
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

void process_datagram(const void* data, uint16_t size, fltk_handle handle)
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

int keyboard_handler(int event)
{

  if (event != FL_KEYDOWN && event != FL_SHORTCUT)
  {
    return 0;
  }
  if (Fl::event_key() == '+')
  {
    if (++ihData.target_course >= 360)
    {
      ihData.target_course %= 360;
    }
    ihData_changed = true;
    return 1;
  }
  else if (Fl::event_key() == '-')
  {
    if (--ihData.target_course < 0)
    {
      ihData.target_course += 360;
    }
    ihData_changed = true;
    return 1;
  }
  return 0;
};

void send_ap_input(const int fd, sockaddr_un* addr, socklen_t len, InputHandleData_t* data,
                   fltk_handle* handle)
{
  Message<InputHandleData_t> msg{data};
  uint8_t msg_buffer[MessageOffsets::payload + InputHandlePayloadOffsets::payload_length];
  const uint16_t msg_size =
      mp_write_InputHandleMessage_to_bytes(msg_buffer, sizeof(msg_buffer), &msg);
  if (msg_size == 0)
  {
    return;
  }

  if (sendto(fd, msg_buffer, msg_size, MSG_DONTWAIT, reinterpret_cast<const sockaddr*>(addr), len) <
      0)
  {
    std::perror("send input to gateway");
    return;
  }
  printf("%d \n", data->target_course);
};

int main()
{
  const char* display_socket_path = "/tmp/autopilot.sock";
  const char* gateway_socket_path = "/tmp/autopilot-gateway.sock";
  unlink(display_socket_path);

  const int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (fd < 0)
  {
    std::perror("socket");
    return 1;
  }
  sockaddr_un local_addr{};
  local_addr.sun_family = AF_UNIX;

  std::strncpy(local_addr.sun_path, display_socket_path, sizeof(local_addr.sun_path) - 1);

  if (bind(fd, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) < 0)
  {
    std::perror("bind");
    close(fd);
    return 1;
  }

  fltk_handle handle = fltk_setup();
  Fl::add_handler(keyboard_handler);

  sockaddr_un gateway_addr{};
  gateway_addr.sun_family = AF_UNIX;
  std::strncpy(gateway_addr.sun_path, gateway_socket_path, sizeof(gateway_addr.sun_path) - 1);

  char buffer[2048];

  while (handle.window->shown())
  {
    Fl::wait(0.01);
    if (ihData_changed)
    {
      refresh_target_box(handle);
      send_ap_input(fd, &gateway_addr, sizeof(gateway_addr), &ihData, &handle);
    }
    ihData_changed = false;

    const ssize_t received =
        recvfrom(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT, nullptr, nullptr);
    if (received < 0)
    {
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        continue;
      break;
    }
    process_datagram(buffer, received, handle);
  }

  close(fd);
  unlink(display_socket_path);
}
