#pragma once

#include "protocol/autopilotWireProtocol.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

/**
 * Consumes a AccumulatedLogMessage by forwarding the complete message to the
 * monitor. A future native logger can consume the same message unchanged.
 * - FUTURE write to logfile
 *
 * @param [in] fd_display
 * 		File descriptor of the Unix-domain socket used by the displayProgram
 *
 * @param [in] display_addr
 * 		Ptr to adress of the Unix-domain socked used by the displayProgram
 *
 * @param [in] msg
 * 		Ptr to message to be consumed
 */
void consume_telemetry_log_msg(int fd, sockaddr_un* addr, const Message<AccumulatedLogMessage>* msg);

/**
 * Consumes Messages with NmeaSentences payloads which contains
 * - forwarding to OpenCPN via UDP
 *
 * @param [in] fd_opencpn
 * 		File descriptor of the UDP socket used by OpenCPN
 *
 * @param [in] opencpn_addr
 * 		Ptr to adress of the UDP socket used by OpenCPN
 *
 * @param [in] msg
 *		Ptr to message to be consumed
 */
void consume_nmea_msg(int file_descriptor, sockaddr_in* addr,
                      const Message<NmeaSentences>* sentences_ptr);
