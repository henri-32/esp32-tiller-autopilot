#include "socket_handling.h"

#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

const socket_context get_socket_context()
{
  static socket_context context;

  const char* display_socket_path = "/tmp/autopilot.sock";
  const char* gateway_socket_path = "/tmp/autopilot-gateway.sock";
  unlink(display_socket_path);

  context.local_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (context.local_fd < 0)
  {
    std::perror("socket");
    context.success = false;
    return context;
  }

  context.local_addr.sun_family = AF_UNIX;

  strncpy(context.local_addr.sun_path, display_socket_path,
          sizeof(context.local_addr.sun_path) - 1);

  if (bind(context.local_fd, reinterpret_cast<sockaddr*>(&context.local_addr),
           sizeof(context.local_addr)) < 0)
  {
    std::perror("bind"), close(context.local_fd);
    context.success = false;
    return context;
  }

  context.gateway_addr.sun_family = AF_UNIX;
  strncpy(context.gateway_addr.sun_path, gateway_socket_path,
          sizeof(context.gateway_addr.sun_path) - 1);
  context.gateway_addr_len =sizeof(sockaddr_un);

  context.success = true;
  return context;
};

void cleanup_sockets(socket_context context)
{
  close(context.local_fd);
  unlink(context.local_addr.sun_path);
}
