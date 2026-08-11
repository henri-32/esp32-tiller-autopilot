#pragma once

#include <netinet/in.h>
#include <sys/un.h>
#include <termios.h>

/**
 * Owns the file descriptors and peer addresses required by the gateway's
 * serial, UDP and Unix-domain communication channels.
 */
struct CommunicationContext
{
  /** UDP socket used to forward NMEA sentences to OpenCPN. */
  int udpFd{-1};
  /** Unix-domain datagram socket for monitor output and UI input. */
  int unixDSockFd{-1};
  /** Configured serial connection to the autopilot. */
  int serialFd{-1};

  /** Destination address of the OpenCPN UDP receiver. */
  sockaddr_in openCPN_addr{};
  /** Destination address of the navigation monitor. */
  sockaddr_un navigationMonitor_addr{};
  /** Local Unix-domain address bound by this gateway. */
  sockaddr_un gateway_addr{};

  /** True only when every required communication channel was initialized. */
  bool success = false;
};

/**
 * Opens a serial device and configures it for raw, non-blocking 8N1 I/O.
 *
 * @param path Path to the serial device.
 * @param baud POSIX baud-rate constant to apply to the device.
 * @return The configured file descriptor, or -1 if opening or configuration
 *         fails.
 */
int openSerial(const char* path, speed_t baud);

/**
 * Creates the communication channels used by the gateway.
 *
 * Command-line arguments optionally override the serial device, UDP host and
 * UDP port; otherwise the implementation defaults are used.
 *
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument values.
 * @return A populated context with success set to true, or a context with
 *         success set to false if initialization fails.
 */
const CommunicationContext getCommContext(int argc, char** argv);

/**
 * Closes the communication file descriptors and removes the gateway's local
 * Unix-domain socket path.
 *
 * @param ctx Context whose resources are to be released.
 */
void clean_communication_ressources(CommunicationContext ctx);
