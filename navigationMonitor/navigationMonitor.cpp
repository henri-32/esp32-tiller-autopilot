#include "cobs-c/cobs.h"
#include "protocol/autopilotWireProtocol.h"
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

using std::string;

/*TODO
Globale inputDaten überprüfen. aktuell müssten die lokalen Daten verändert werden und an autopilotGateway
geschrieben werden. Der müsste zukünftig an esp (rdwr muss noch gesetzt werden) senden, da in
systemcontroller und wieder raus den ganzen weg zurück, damit das interne target angezeigt wird,
nicht das was ich hier setze.
*/

struct fltk_handle
{
  Fl_Window* window;
  Fl_Box* sog_box;
  Fl_Box* cog_box;
  Fl_Box* target_cog_box;
  Fl_Box* sog_valid_box;
  Fl_Box* cog_valid_box;
  Fl_Box* position_box;
  Fl_Box* position_valid_box;
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
  auto* target_cog = box(0, 1);

  //  auto* position = box(2, 0);
  // auto* sog_valid = box(0, 1);
  // auto* cog_valid = box(1, 1);
  // auto* position_valid = box(2, 1);
  window->end();
  window->show();
  return {
      .window = window, .sog_box = sog, .cog_box = cog, .target_cog_box = target_cog
      //       .sog_valid_box = sog_valid,
      //      .cog_valid_box = cog_valid,
      //     .position_box = position,
      //    .position_valid_box = position_valid
  };
};

void process_NavigationMessage(void* data, uint16_t size, fltk_handle handle)
{
  std::vector<uint8_t> buf_dec(size);

  cobs_decode_result dec_res = cobs_decode(buf_dec.data(), buf_dec.size(), data, size);

  if (dec_res.status == COBS_DECODE_OK)
  {
    Message<NavigationSnapshot> msg =
        mp_read_NavigationMessage_from_buffer(buf_dec.data(), buf_dec.size());
    const auto sog = msg.payload.gps_sog_kts;
    const auto cog = msg.payload.gps_cog_dg;
    const auto hdg = msg.payload.compass_hdg_dg;

    char text[64];

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

    if (hdg.valid)
    {
      std::snprintf(text, sizeof(text), "HDG: %d deg", hdg.value);
    }
    else
    {
      std::snprintf(text, sizeof(text), "HDG : invalid");
    }
  }
}

void process_datagram(void* data, uint16_t size, fltk_handle handle)
{
  uint8_t type;

  if (size >= 2 && static_cast<uint8_t*>(data)[0] > 1)
  {
    uint8_t type = static_cast<uint8_t*>(data)[1];
    switch (type)
    {

    case static_cast<uint8_t>(PayloadType::NavigationSnapshot):
    {
      process_NavigationMessage(data, size, handle);
    }
    break;
    }
  }
}

static InputHandleData ihData{};

int keyboard_handler(int event)
{

  printf("FLTK event: %d\n", event);
  if (event != FL_KEYDOWN && event != FL_SHORTCUT)
  {
    return 0;
  }
  printf("key=%d text = %s \n", Fl::event_key(), Fl::event_text());
  if (Fl::event_key() == '+')
  {
    ihData.target_course++;
    printf("+ \n");
    return 1;
  }
  else if (Fl::event_key() == '-')
  {

    ihData.target_course--;
    printf("- \n");
    return 1;
  }
  return 0;
};

void send_ap_input(const int fd, sockaddr_un* addr, socklen_t len, InputHandleData* data,
                   fltk_handle* handle)
{
  Message<InputHandleData> msg{data};
  uint8_t msg_buffer[MessageOffsets::payload + InputHandlePayloadOffsets::payload_length];
  const uint16_t msg_size = mp_write_InputHandleMessage_to_bytes(msg_buffer, sizeof(msg_buffer), &msg);
  if (msg_size == 0)
  {
    return;
  }

  if (sendto(fd, msg_buffer, msg_size, MSG_DONTWAIT, reinterpret_cast<const sockaddr*>(addr), len) < 0)
  {
    std::perror("send input to gateway");
    return;
  }
  char text[100];
  std::snprintf(text, sizeof(text), "TARGET: %d deg \n", data->target_course);
  handle->target_cog_box->copy_label(text);
  handle->target_cog_box->redraw();
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

    const ssize_t received =
        recvfrom(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT, nullptr, nullptr);
    if (received < 0)
    {
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        continue;
      break;
    }
    process_datagram(buffer, received, handle);
    send_ap_input(fd, &gateway_addr, sizeof(gateway_addr), &ihData, &handle);
  }

  close(fd);
  unlink(display_socket_path);
}
