#pragma once

#include <sys/un.h>
#include <sys/socket.h>

struct socket_context
{
  sockaddr_un local_addr{};
  int local_fd = -1;
  sockaddr_un gateway_addr{};
  socklen_t gateway_addr_len;
  bool success = false;
};

const socket_context get_socket_context();
void cleanup_sockets(socket_context context);
