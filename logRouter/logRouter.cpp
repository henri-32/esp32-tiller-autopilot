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
#include "logging/message_protocol.h"

static constexpr const char* nmeaPrefix = "@NMEA ";
static constexpr const char* sourcePrefix = "@SOURCE ";

int openSerial(const char* path, speed_t baud)
//{{{
{
  int fd = open(path, O_RDONLY | O_NOCTTY | O_NONBLOCK);
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

void consume_nav_msg(int fd, sockaddr_un* addr, Message<NavigationSnapshot>* msg)
//{{{
{
  uint8_t raw_msg[sizeof(Message<NavigationSnapshot>)];
  uint16_t size = mp_write_NavigationMessage_to_bytes(&raw_msg, msg);
  uint8_t msg_encoded[size + size / 256 + 1];

  cobs_encode_result encode_res = cobs_encode(&msg_encoded, sizeof(msg_encoded), raw_msg, size);

  if (encode_res.status == COBS_ENCODE_OK)
  {
    int send = sendto(fd, msg_encoded, encode_res.out_len, MSG_DONTWAIT,
                      reinterpret_cast<const sockaddr*>(addr), sizeof(*addr));

    if (send < 0 && errno != ENOENT)
    {
      perror("send to unix socket from consume_nav_msg");
    }
  }
};
//}}}

void consume_nmea_msg(int file_descriptor, sockaddr_in* addr, Message<NmeaSentences>* sentences_ptr)
//{{{
{
  for (int i = 0; i < sentences_ptr->payload.sentence_count; ++i)
  {

    size_t length =
        strnlen(sentences_ptr->payload.sentence[i], sizeof(sentences_ptr->payload.sentence[i]));

    ssize_t send = sendto(file_descriptor, sentences_ptr->payload.sentence[i], length, MSG_DONTWAIT,
                          reinterpret_cast<const sockaddr*>(addr), sizeof(*addr));

    if (send < 0)
    {
      perror("send to udp port from consume_nmea_sentences");
      break;
    }
  }
}
//}}}

void process_cobs_msg(int fd_udp_to_opencpn, sockaddr_in opencpn_addr,
                      int fd_unix_socket_to_ap_display, sockaddr_un ap_display_addr,
                      std::vector<uint8_t>* encoded_frame)
//{{{
{
  std::vector<uint8_t> decoded_frame(encoded_frame->size());
  cobs_decode_result dec_res = cobs_decode(decoded_frame.data(), decoded_frame.size(),
                                           encoded_frame->data(), encoded_frame->size());
  uint8_t* decoded_data = decoded_frame.data();

  if (dec_res.status == COBS_DECODE_OK && dec_res.out_len >= MessageOffsets::payload)
  {
    MessageHeader header;
    header.type = decoded_data[0];
    header.payload_length =
        static_cast<uint16_t>(decoded_data[1]) | (static_cast<uint16_t>(decoded_data[2]) << 8);

    switch (header.type)
    {
    case static_cast<uint8_t>(PayloadType::NavigationSnapshot):
      if (header.payload_length == NavigationPayloadOffsets::payload_length &&
          dec_res.out_len >= MessageOffsets::payload + NavigationPayloadOffsets::payload_length)
      {
        Message<NavigationSnapshot> msg{};
        msg = mp_read_NavigationMessage_from_buffer(decoded_data);
        consume_nav_msg(fd_unix_socket_to_ap_display, &ap_display_addr, &msg);
        break;
      }
      break;

    case static_cast<uint8_t>(PayloadType::NmeaSentences):
      if (decoded_data[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] <= 20 &&
          header.payload_length ==
              NmeaPayloadOffsets::sentences +
                  decoded_data[MessageOffsets::payload + NmeaPayloadOffsets::sentence_count] *
                      sizeof(NmeaSentences::sentence[0]) &&
          dec_res.out_len >= MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                                 (header.payload_length - NmeaPayloadOffsets::sentences))
      {
        Message<NmeaSentences> msg{};
        msg = mp_read_NmeaMessage_from_buffer(decoded_data);
        consume_nmea_msg(fd_udp_to_opencpn, &opencpn_addr, &msg);
        break;
      }
      break;
    };
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

  // Accumulate encoded UART bytes until a frame delimiter arrives.
  std::vector<uint8_t> msg_encoded;

  printf("Reached while \n");
  while (true)

  {
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

      if (byte == 0x00)
      {
        if (!msg_encoded.empty())
        {
          process_cobs_msg(nmea_udp_fd, nmea_udp_dest, ap_display_unix_socket_fd, ap_display_dest,
                           &msg_encoded);
          msg_encoded.clear();
        }
      }
      else
      {
        msg_encoded.push_back(byte);
      }
    }
  }

  close(nmea_udp_fd);
  close(ap_display_unix_socket_fd);
  close(serialFd);
}
//}}}
