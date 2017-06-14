#pragma once

#include "socket.hpp"
#include "ioloop.hpp"
#include "connection.hpp"
#include <sys/types.h>
#include <vector>


namespace godgun {
  namespace master {
    class Master {
    public:
      Master(int argc, char* argv[],const ioloop::EPollIOLoop::HttpConnectionHandler&);
      Master(const Master&) = delete;
      Master(Master&&) = delete;
      Master& operator=(const Master&) = delete;
      ~Master();

      void start();

      void close();
      void shutdown(int);
      int fd() const;

    private:
      int _fd;
      struct sockaddr_in _addr;
      std::vector<ioloop::IOLoop*> _loops;
    };
  }
}
