#pragma once

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <stdexcept>

namespace godgun {
  namespace socket {
    class SocketError : public std::runtime_error {
    public:
      explicit SocketError(int fd, int code, const std::string& msg);
      explicit SocketError(int fd, int code, const char* msg);
      int code() const;
      int fd() const;
    private:
      int _code;
      int _fd;
    };

    class SocketClient {
    public:
      SocketClient(int fd, const struct sockaddr_in& addr, bool nonblock = true);
      ~SocketClient();

      SocketClient(const SocketClient& rhs);
      SocketClient(SocketClient&& rhs);

      int send(const char* buf, int len) throw (SocketError);
      int recv(char* buf, int len) throw(SocketError);

      const char* ip() const;
      int fd() const;
      uint16_t port() const;

      void close();
      void shutdown(int how);

    private:
      int _fd;
      struct sockaddr_in _addr;
    };
  }
}
