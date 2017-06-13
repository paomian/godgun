#include "include/socket.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

namespace godgun {
  namespace socket {
    SocketError::SocketError(int fd, int code, const std::string& msg)
      : std::runtime_error(msg), _fd(fd), _code(code) {}

    SocketError::SocketError(int fd, int code, const char* msg)
      : std::runtime_error(msg), _fd(fd), _code(code) {}

    int SocketError::code() const {
      return _code;
    }

    int SocketError::fd() const {
      return _fd;
    }

    SocketClient::SocketClient(int fd, const struct sockaddr_in& addr, bool nonblock)
      : _fd(fd), _addr(addr) {
      if (nonblock) {
        int opts{0};
        if ((opts = fcntl(_fd, F_GETFL)) < 0) {
          perror("SocketClient:fcntl");
          std::abort();
        }

        opts |= O_NONBLOCK;

        if (fcntl(_fd, F_SETFL, opts) < 0) {
          perror("SocketClient:fcntl");
          std::abort();
        }
      }
    }

    SocketClient::~SocketClient() {}

    SocketClient::SocketClient(const SocketClient& rhs)
      : _fd(rhs._fd), _addr(rhs._addr) {}

    SocketClient::SocketClient(SocketClient&& rhs)
      : _fd(rhs._fd), _addr(rhs._addr) {}

    int SocketClient::send(const char* buf, int len) throw(SocketError) {
      int ret = ::send(_fd, buf, len, 0);

      if (ret < 0) {
        throw SocketError(_fd, errno, strerror(errno));
      } else if (ret == 0) {
        throw SocketError(_fd, EREMOTEIO, "remote client closed.");
      }
      return ret;
    }

    int SocketClient::recv(char* buf, int len) throw (SocketError) {
      int ret = ::recv(_fd, buf, len, 0);

      if (ret < 0) {
        throw SocketError(_fd, errno, strerror(errno));
      } else if (ret == 0) {
        throw SocketError(_fd, EREMOTEIO, "remote client closed.");
      }
      return ret;
    }

    const char* SocketClient::ip() const {
      return inet_ntoa(_addr.sin_addr);
    }

    uint16_t SocketClient::port() const {
      return ntohs(_addr.sin_port);
    }

    void SocketClient::close() {
      if (::close(_fd) < 0)
        std::cerr << _fd << " ";
        perror("SocketClient:close");
    }

    int SocketClient::fd() const {
      return _fd;
    }

    void SocketClient::shutdown(int how) {
      if (::shutdown(_fd, how) < 0) {
        std::cerr << _fd << " ";
        perror("SocketClient:shutdown");
      }
    }
  }
}
