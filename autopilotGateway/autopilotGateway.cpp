#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include <cstring>
#include <string>

#include "cobs-c/cobs.h"
#include "consume_messages.h"
#include "protocol/autopilotWireProtocol.h"

int openSerial(const char* path, speed_t baud)
//{{{
{
  int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0)
  {
    perror("open serial");
    return -1;
  }

  termios tty{};
  if (tcgetattr(fd, &tty) != 0)
  {
    perror("tcgetattr");
    close(fd);
    return -1;
  }

  cfmakeraw(&tty);
  cfsetispeed(&tty, baud);
  cfsetospeed(&tty, baud);

  tty.c_cflag |= CLOCAL | CREAD;
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 10; // Read timeout in deciseconds.

  if (tcsetattr(fd, TCSANOW, &tty) != 0)
  {
    perror("tcsetattr");
    close(fd);
    return -1;
  }

  return fd;
}
//}}}

/**
 * Receives a serialized Message<InputHandleData_t> from the control panel and
 * forwards it as a COBS-framed message to the autopilot over the serial port.
 *
 * @param [in] fd_gateway
 *     File descriptor of the Unix-domain socket bound by this gateway.
 *
 * @param [in] serial_fd
 *     File descriptor of the serial port connected to the autopilot.
 */

void forward_inputData_to_ap(int fd_gateway, int fd_serial)
//{{{
{
  uint8_t msg_buffer[MessageOffsets::payload + InputHandlePayloadOffsets::payload_length];
  const int received = recv(fd_gateway, msg_buffer, sizeof(msg_buffer), MSG_DONTWAIT);

  if (received <= 0)
  {
    return;
  }

  Message<InputHandleData_t> msg =
      mp_read_InputHandleMessage_from_buffer(msg_buffer, static_cast<uint16_t>(received));
  if (msg.header.type != static_cast<uint8_t>(CommandType::InputHandleData))
  {
    return;
  }

  uint8_t encoded_buffer[COBS_ENCODE_DST_BUF_LEN_MAX(sizeof(msg_buffer)) + 1];
  cobs_encode_result enc_res = cobs_encode(encoded_buffer, sizeof(encoded_buffer), msg_buffer,
                                           static_cast<size_t>(received));
  if (enc_res.status != COBS_ENCODE_OK)
  {
    return;
  }

  encoded_buffer[enc_res.out_len] = 0x00;
  if (write(fd_serial, encoded_buffer, enc_res.out_len + 1) !=
      static_cast<ssize_t>(enc_res.out_len + 1))
  {
    perror("write input to serial");
  }
};
//}}}

/**
 * Routes the message protocol Message towards different files/programs depending
 * on the message type
 *
 * @param [in] fd_opencpn
 * 		File descriptor of the UDP socket used by opencpn
 *
 * @param [in] opencpn_addr
 * 		Adress of the UDP socket used by opencpn
 *
 * @param [in] fd_display
 * 		File descriptor of the Unix-domain socket used by the displayProgram
 *
 * @param [in] display_addr
 * 		Adress of the Unix-domain socket used by the displayProgram
 *
 * @param [in] msg_buf
 * 		Ptr to the buffer where the message is stored
 *
 * @param [in] msg_len
 *		Length of the msg
 */
void route_mp_msg(int fd_opencpn, sockaddr_in opencpn_addr, int fd_display,
                  sockaddr_un display_addr, uint8_t* msg_buf, int msg_len)
//{{{
#include <netinet/in.h>
{
  if (msg_buf == nullptr || msg_len < MessageOffsets::payload)
  {
    return;
  }

  MessageHeader header;
  header.type = msg_buf[MessageOffsets::type];
  header.payload_length =
      wire_detail::read_uint16(msg_buf, MessageOffsets::payload_length_little_endian);

  switch (header.type)
  {
  case static_cast<uint8_t>(PayloadType::AccumulatedLogMessage):
    if (header.payload_length == AccumulatedLogMessagePayloadOffsets::payload_length &&
        msg_len >= MessageOffsets::payload + AccumulatedLogMessagePayloadOffsets::payload_length)
    {
      const Message<AccumulatedLogMessage> msg =
          mp_read_AccumulatedLogMessage_from_buffer(msg_buf, static_cast<uint16_t>(msg_len));
      if (msg.header.type == static_cast<uint8_t>(PayloadType::AccumulatedLogMessage))
      {
        consume_telemetry_log_msg(fd_display, &display_addr, &msg);
      }
    }
    break;

  case static_cast<uint8_t>(PayloadType::NmeaSentences):
    if (msg_len >= MessageOffsets::payload + NmeaPayloadOffsets::sentences)
    {
      const Message<NmeaSentences> msg =
          mp_read_NmeaMessage_from_buffer(msg_buf, static_cast<uint16_t>(msg_len));
      if (msg.header.type == static_cast<uint8_t>(PayloadType::NmeaSentences))
      {
        consume_nmea_msg(fd_opencpn, &opencpn_addr, &msg);
      }
    }
    break;

  default:
    break;
  }
};
//}}}

int main(int argc, char** argv)
//{{{
{
  // Read serial and UDP settings from command-line arguments.
  const char* serialPath = argc >= 2 ? argv[1] : "/dev/ttyUSB0";
  const char* udpHost = argc >= 3 ? argv[2] : "127.0.0.1";
  int udpPort = argc >= 4 ? std::stoi(argv[3]) : 10110;

  // Open and configure the serial port.
  int serialFd = openSerial(serialPath, B115200);
  if (serialFd < 0)
  {
    return 1;
  }

  // Create the NMEA UDP socket.
  int nmea_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (nmea_udp_fd < 0)
  {
    perror("nmea socket");
    close(serialFd);
    return 1;
  }

  // Configure the NMEA UDP destination.
  sockaddr_in nmea_udp_dest{};
  nmea_udp_dest.sin_family = AF_INET;
  nmea_udp_dest.sin_port = htons(udpPort);
  inet_pton(AF_INET, udpHost, &nmea_udp_dest.sin_addr);

  // Create the UNIX socket used for both display output and UI input.
  int ap_display_unix_socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (ap_display_unix_socket_fd < 0)
  {
    perror("source socket");
    return 1;
  }

  // Bind a dedicated local address for UI input.
  constexpr const char* gateway_socket_path = "/tmp/autopilot-gateway.sock";
  unlink(gateway_socket_path);
  sockaddr_un gateway_addr{};
  gateway_addr.sun_family = AF_UNIX;
  std::strncpy(gateway_addr.sun_path, gateway_socket_path, sizeof(gateway_addr.sun_path) - 1);
  if (bind(ap_display_unix_socket_fd, reinterpret_cast<sockaddr*>(&gateway_addr),
           sizeof(gateway_addr)) < 0)
  {
    perror("bind gateway socket");
    close(ap_display_unix_socket_fd);
    close(nmea_udp_fd);
    close(serialFd);
    return 1;
  }

  // Configure the display destination for navigation snapshots.
  sockaddr_un display_addr{};
  display_addr.sun_family = AF_UNIX;
  std::strncpy(display_addr.sun_path, "/tmp/autopilot.sock", sizeof(display_addr.sun_path) - 1);

  std::vector<uint8_t> msg_encoded;
  std::vector<uint8_t> msg_decoded;

  //===================================================================================================================================================
  printf("gateway running... \n");
  while (true)

  {
    forward_inputData_to_ap(ap_display_unix_socket_fd, serialFd);

    // Read serial data into the receive buffer.
    uint8_t read_buffer[256];
    ssize_t n = read(serialFd, &read_buffer, sizeof(read_buffer));
    if (n <= 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        continue;
      }
      perror("read serial");
      break;
    }

    // Split the stream into zero-delimited COBS frames.
    for (int i = 0; i < n; ++i)
    {
      const uint8_t byte = read_buffer[i];

      if (byte != 0x00)
      {
        msg_encoded.push_back(byte);
        continue;
      }

      if (msg_encoded.empty())
      {
        continue;
      }

      msg_decoded.resize(msg_encoded.size());
      cobs_decode_result dec_res = cobs_decode(msg_decoded.data(), msg_decoded.size(),
                                               msg_encoded.data(), msg_encoded.size());

      if (dec_res.status != COBS_DECODE_OK)
      {
        msg_decoded.clear();
        msg_encoded.clear();
        continue;
      }

      route_mp_msg(nmea_udp_fd, nmea_udp_dest, ap_display_unix_socket_fd, display_addr,
                   msg_decoded.data(), dec_res.out_len);

      msg_encoded.clear();
      msg_decoded.clear();
    }
  }
  close(nmea_udp_fd);
  close(ap_display_unix_socket_fd);
  unlink(gateway_socket_path);
  close(serialFd);
}
//}}}
