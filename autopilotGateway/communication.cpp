#include "communication.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <sys/poll.h>
#include <unistd.h>

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

const CommunicationContext getCommContext(int argc, char** argv)
//{{{
{
  CommunicationContext ret{};

  // Read serial and UDP settings from command-line arguments.
  const char* serialPath = argc >= 2 ? argv[1] : "/dev/ttyUSB0";
  const char* udpHost = argc >= 3 ? argv[2] : "127.0.0.1";
  int udpPort = argc >= 4 ? std::stoi(argv[3]) : 10110;

  // Open and configure the serial port.
  int serialFd = openSerial(serialPath, B115200);
  if (serialFd < 0)
  {
    return {.success = false};
  }

  // Create the NMEA UDP socket.
  int udpFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (udpFd < 0)
  {
    perror("nmea socket");
    close(serialFd);
    return {.success = false};
  }

  // Configure the NMEA UDP destination.
  sockaddr_in openCPN_addr{};
  openCPN_addr.sin_family = AF_INET;
  openCPN_addr.sin_port = htons(udpPort);
  inet_pton(AF_INET, udpHost, &openCPN_addr.sin_addr);

  // Create the UNIX socket used for both display output and UI input.
  int unixDSockFd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (unixDSockFd < 0)
  {
    perror("source socket");
    return {.success = false};
  }

  // Bind a dedicated local address for UI input.
  constexpr const char* gateway_socket_path = "/tmp/autopilot-gateway.sock";
  unlink(gateway_socket_path);
  sockaddr_un gateway_addr{};
  gateway_addr.sun_family = AF_UNIX;
  strncpy(gateway_addr.sun_path, gateway_socket_path, sizeof(gateway_addr.sun_path) - 1);
  if (bind(unixDSockFd, reinterpret_cast<sockaddr*>(&gateway_addr), sizeof(gateway_addr)) < 0)
  {
    perror("bind gateway socket");
    close(unixDSockFd);
    close(udpFd);
    close(serialFd);
    return {.success = false};
  }

  // Configure the display destination for navigation snapshots.
  sockaddr_un navigationMonitor_addr{};
  navigationMonitor_addr.sun_family = AF_UNIX;
  strncpy(navigationMonitor_addr.sun_path, "/tmp/autopilot.sock",
          sizeof(navigationMonitor_addr.sun_path) - 1);

  return {.udpFd = udpFd,
          .unixDSockFd = unixDSockFd,
          .serialFd = serialFd,
          .openCPN_addr = openCPN_addr,
          .navigationMonitor_addr = navigationMonitor_addr,
		  .gateway_addr = gateway_addr,
          .success = true};
};
//}}}

void clean_communication_ressources(CommunicationContext ctx) {
//{{{

  close(ctx.udpFd);
  close(ctx.unixDSockFd);
  unlink(ctx.gateway_addr.sun_path);
  close(ctx.serialFd);
}
//}}}
