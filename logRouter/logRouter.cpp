  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>

  #include <cstring>
  #include <iostream>
  #include <string>

  static constexpr const char* kPrefix = "@NMEA ";

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
    const auto pos = line.find(kPrefix);
    if (pos == std::string::npos)
    {
      return {};
    }

    std::string sentence = line.substr(pos + std::strlen(kPrefix));

    while (!sentence.empty() && (sentence.back() == '\n' || sentence.back() == '\r' || sentence.back() == ' '))
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

    int udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpFd < 0)
    {
      perror("socket");
      close(serialFd);
      return 1;
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(udpPort);
    inet_pton(AF_INET, udpHost, &dest.sin_addr);

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
        if (!nmea.empty())
        {
          nmea += "\r\n";
          sendto(udpFd, nmea.data(), nmea.size(), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
        }

        line.clear();
      }
      else if (c != '\r')
      {
        line.push_back(c);
      }
    }

    close(udpFd);
    close(serialFd);
  }


