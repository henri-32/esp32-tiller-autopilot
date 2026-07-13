#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

static constexpr const char* nmeaPrefix = "@NMEA ";
static constexpr const char* sourcePrefix = "@SOURCE ";

int openSerial(const char* path, speed_t baud)
{
  int fd = open(path, O_RDONLY | O_NOCTTY);
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
  tty.c_cc[VTIME] = 10; // 1s timeout

  if (tcsetattr(fd, TCSANOW, &tty) != 0)
  {
    perror("tcsetattr");
    close(fd);
    return -1;
  }

  return fd;
}

std::string extractNmea(const std::string& line)
{
  const auto pos = line.find(nmeaPrefix);
  if (pos == std::string::npos)
  {
    return {};
  }

  std::string sentence = line.substr(pos + std::strlen(nmeaPrefix));

  while (!sentence.empty() &&
         (sentence.back() == '\n' || sentence.back() == '\r' || sentence.back() == ' '))
  {
    sentence.pop_back();
  }

  if (sentence.empty() || (sentence.front() != '$' && sentence.front() != '!'))
  {
    return {};
  }

  const auto star = sentence.find('*');
  if (star != std::string::npos && sentence.size() >= star + 3)
  {
    sentence = sentence.substr(0, star + 3);
  }

  return sentence;
}

std::string extractSource(const std::string& line)
{
  const auto pos = line.find(sourcePrefix);
  if (pos == std::string::npos)
  {
    return {};
  }

  std::string sentence = line.substr(pos + std::strlen(sourcePrefix));

  while (!sentence.empty() &&
         (sentence.back() == '\n' || sentence.back() == '\r' || sentence.back() == ' '))
  {
    sentence.pop_back();
  }

  if (sentence.empty() || sentence.front() != '&')
  {
    sentence = "";
    return {};
  }

  return sentence;
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " /dev/ttyUSB0 [udp_host] [udp_port]\n";
    return 1;
  }

  const char* serialPath = argv[1];
  const char* udpHost = argc >= 3 ? argv[2] : "127.0.0.1";
  int udpPort = argc >= 4 ? std::stoi(argv[3]) : 10110;

  int serialFd = openSerial(serialPath, B115200);
  if (serialFd < 0)
  {
    return 1;
  }

  int nmea_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (nmea_udp_fd < 0)
  {
    perror("nmea socket");
    close(serialFd);
    return 1;
  }

  sockaddr_in nmea_udp_dest{};
  nmea_udp_dest.sin_family = AF_INET;
  nmea_udp_dest.sin_port = htons(udpPort);
  inet_pton(AF_INET, udpHost, &nmea_udp_dest.sin_addr);

  int source_display_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (source_display_fd < 0)
  {
    perror("source socket");
    return 1;
  }
  sockaddr_un source_display_dest{};
  source_display_dest.sun_family = AF_UNIX;
  std::strncpy(source_display_dest.sun_path, "/tmp/autopilot.sock",
               sizeof(source_display_dest.sun_path) - 1);

  std::string line;
  char c;

  while (true)
  {
    ssize_t n = read(serialFd, &c, 1);
    if (n <= 0)
    {
      continue;
    }

    if (c == '\n')
    {
      std::cout << line << "\n";

      std::string nmea = extractNmea(line);
      std::string source = extractSource(line);
      if (!nmea.empty())
      {
        nmea += "\r\n";
        if (sendto(nmea_udp_fd, nmea.data(), nmea.size(), 0,
                   reinterpret_cast<sockaddr*>(&nmea_udp_dest), sizeof(nmea_udp_dest)) < 0)
        {
          perror("sendto nmea");
        }
      }
      else if (!source.empty())
      {
        source += "\n";
        if (sendto(source_display_fd, source.data(), source.size(), 0,
                   reinterpret_cast<sockaddr*>(&source_display_dest),
                   sizeof(source_display_dest)) < 0)
        {
          perror("sendto source display");
        }
      }

      line.clear();
    }
    else if (c != '\r')
    {
      line.push_back(c);
    }
  }

  close(nmea_udp_fd);
  close(source_display_fd);
  close(serialFd);
}
