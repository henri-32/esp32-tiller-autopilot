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
#include "logging/message_protocol.h"

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
 * Receives a Message<InputData> from the display program and forwards it
 * to the autopilot over the serial port.
 *
 * @param [in] fd_display
 *     File descriptor of the Unix-domain socket used by the displayProgram.
 *
 * @param [out] display_addr
 *     Address structure that receives the sender's Unix-domain socket address.
 *
 * @param [in,out] display_len
 *     On input, the available size of display_addr in bytes.
 *     On output, the actual size of the received sender address.
 *
 * @param [in] serial_fd
 *     File descriptor of the serial port connected to the autopilot.
 */

void forward_inputData_to_ap(int fd_display, sockaddr_un display_addr, socklen_t display_len,
                             int fd_serial)
//{{{
{

/* Recieve InputData from displayProgram */
  Message<InputHandleData> msg{};
  int receive = recvfrom(fd_display, &msg, sizeof(Message<InputHandleData>), O_NONBLOCK,
                         reinterpret_cast<sockaddr*>(&display_addr), &display_len);

  if (receive <= 0)
  {
    return;
  }

/*Write raw byte buffer from structured InputData */
  uint8_t msg_buffer[sizeof(msg)];
  int size = mp_write_InputHandleMessage_to_bytes(msg_buffer, sizeof(msg_buffer), &msg);
  if (size != sizeof(Message<InputHandleData>))
  {
    return;
  }

/*COBS encode Message */
  uint8_t encoded_buffer[COBS_ENCODE_DST_BUF_LEN_MAX(sizeof(msg_buffer))];
  cobs_encode_result enc_res =
      cobs_encode(&encoded_buffer, sizeof(encoded_buffer), msg_buffer, sizeof(msg_buffer));
  if (enc_res.status != COBS_ENCODE_OK)
  {
    return;
  }

/*Send COBS Message over serial port */
  if (send(fd_serial, encoded_buffer, enc_res.out_len, 0) <= 0)
  {
    perror("send Input to serial");
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

  MessageHeader header;
  header.type = msg_buf[0];
  header.payload_length =
      static_cast<uint16_t>(msg_buf[1]) | (static_cast<uint16_t>(msg_buf[2]) << 8);

  switch (header.type)
  {
  case static_cast<uint8_t>(PayloadType::NavigationSnapshot):
    if (header.payload_length == NavigationPayloadOffsets::payload_length &&
        msg_len >= MessageOffsets::payload + NavigationPayloadOffsets::payload_length)
    {
      Message<NavigationSnapshot> msg{};
      msg = mp_read_NavigationMessage_from_buffer(msg_buf, msg_len);
      consume_nav_msg(fd_display, &display_addr, &msg);
      break;
    }

  case static_cast<uint8_t>(PayloadType::NmeaSentences):
    if (msg_len >= MessageOffsets::payload + NmeaPayloadOffsets::sentences &&
        msg_buf[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] <= 20 &&
        header.payload_length ==
            NmeaPayloadOffsets::sentences +
                msg_buf[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] *
                    sizeof(NmeaSentences::sentence[0]) &&
        msg_len >= MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                       (header.payload_length - NmeaPayloadOffsets::sentences))
    {
      Message<NmeaSentences> msg{};
      msg = mp_read_NmeaMessage_from_buffer(msg_buf, msg_len);
      consume_nmea_msg(fd_opencpn, &opencpn_addr, &msg);
      break;
    }
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

  // Create the autopilot-display UNIX socket.
  int ap_display_unix_socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (ap_display_unix_socket_fd < 0)
  {
    perror("source socket");
    return 1;
  }

  // Configure the autopilot-display socket destination.
  sockaddr_un ap_display_dest{};
  ap_display_dest.sun_family = AF_UNIX;
  std::strncpy(ap_display_dest.sun_path, "/tmp/autopilot.sock",
               sizeof(ap_display_dest.sun_path) - 1);

  std::vector<uint8_t> msg_encoded;
  std::vector<uint8_t> msg_decoded;

  //===================================================================================================================================================
  while (true)

  {
    forward_inputData_to_ap(ap_display_unix_socket_fd, ap_display_dest, sizeof(ap_display_dest),
                            serialFd);

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

      route_mp_msg(nmea_udp_fd, nmea_udp_dest, ap_display_unix_socket_fd, ap_display_dest,
                   msg_decoded.data(), dec_res.out_len);

      msg_encoded.clear();
      msg_decoded.clear();
    }
  }
  close(nmea_udp_fd);
  close(ap_display_unix_socket_fd);
  close(serialFd);
}
//}}}
