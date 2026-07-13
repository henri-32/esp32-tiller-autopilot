#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

struct fltk_handle
{
  Fl_Window* window;
  Fl_Box* sog_box;
  Fl_Box* cog_box;
  Fl_Box* sog_valid_box;
  Fl_Box* cog_valid_box;
  Fl_Box* position_box;
  Fl_Box* position_valid_box;
};

std::string extract_field(const std::string& buffer, const std::string& prefix)
{
  const size_t prefix_pos = buffer.find(prefix);
  if (prefix_pos == std::string::npos)
    return " ";

  const size_t value_pos = prefix_pos + prefix.size();
  const size_t value_end = buffer.find(' ', value_pos);
  return buffer.substr(value_pos, value_end == std::string::npos ? std::string::npos
                                                                   : value_end - value_pos);
}

std::string strip_sog(const std::string& buffer) { return extract_field(buffer, "&GPS -SOG:"); }
std::string strip_cog(const std::string& buffer) { return extract_field(buffer, "&GPS -COG:"); }
std::string validity_label(const std::string& value)
{
  if (value == "0") return "INVALID";
  if (value == "1") return "VALID";
  return value;
}

std::string check_sog_valid(const std::string& buffer)
{
  return extract_field(buffer, "&GPS -SOG:") == " " ? " " : validity_label(extract_field(buffer, "-valid:"));
}

std::string check_cog_valid(const std::string& buffer)
{
  return extract_field(buffer, "&GPS -COG:") == " " ? " " : validity_label(extract_field(buffer, "-valid:"));
}

void update_box(Fl_Box* box, std::string& last, const std::string& value, const std::string& label)
{
  if (value == " " || value == "nan" || value == last)
    return;
  last = value;
  box->copy_label((label + value).c_str());
  box->redraw();
}

void update_sog_box(const std::string& value, fltk_handle handle) { static std::string last; update_box(handle.sog_box, last, value, "SOG: "); }
void update_cog_box(const std::string& value, fltk_handle handle) { static std::string last; update_box(handle.cog_box, last, value, "COG: "); }
void update_sog_valid_box(const std::string& value, fltk_handle handle) { static std::string last; update_box(handle.sog_valid_box, last, value, "SOG valid: "); }
void update_cog_valid_box(const std::string& value, fltk_handle handle) { static std::string last; update_box(handle.cog_valid_box, last, value, "COG valid: "); }
void update_position_valid_box(const std::string& value, fltk_handle handle) { static std::string last; update_box(handle.position_valid_box, last, value, "Position valid: "); }

void update_position_box(const std::string& buffer, fltk_handle handle)
{
  static std::string last;
  const std::string lat = extract_field(buffer, "&GPS -LAT:");
  const std::string lon = extract_field(buffer, "&GPS -LON:");
  if (lat == " " || lon == " ") return;
  const std::string value = "LAT/LON: " + lat + " / " + lon;
  if (value != last) { last = value; handle.position_box->copy_label(value.c_str()); handle.position_box->redraw(); }
}

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
  auto box = [&](int col, int row) { return make_box(margin + col * (width + gap), margin + row * (height + gap), width, height); };

  auto* sog = box(0, 0); auto* cog = box(1, 0); auto* position = box(2, 0);
  auto* sog_valid = box(0, 1); auto* cog_valid = box(1, 1); auto* position_valid = box(2, 1);
  window->end();
  window->show();
  return {.window = window, .sog_box = sog, .cog_box = cog, .sog_valid_box = sog_valid,
          .cog_valid_box = cog_valid, .position_box = position,
          .position_valid_box = position_valid};
}

int main()
{
  const char* socket_path = "/tmp/autopilot.sock";
  unlink(socket_path);
  const int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (fd < 0) { std::perror("socket"); return 1; }
  sockaddr_un local_addr{}; local_addr.sun_family = AF_UNIX;
  std::strncpy(local_addr.sun_path, socket_path, sizeof(local_addr.sun_path) - 1);
  if (bind(fd, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) < 0)
  { std::perror("bind"); close(fd); return 1; }

  fltk_handle handle = fltk_setup(); char buffer[1024];
  while (handle.window->shown())
  {
    Fl::wait(0.01);
    const ssize_t received = recvfrom(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT, nullptr, nullptr);
    if (received < 0) { if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue; break; }
    buffer[received] = '\0'; const std::string data(buffer);
    update_sog_box(strip_sog(data), handle); update_cog_box(strip_cog(data), handle);
    update_sog_valid_box(check_sog_valid(data), handle); update_cog_valid_box(check_cog_valid(data), handle);
    update_position_box(data, handle);
    update_position_valid_box(check_sog_valid(data), handle);
  }
  close(fd); unlink(socket_path);
}
