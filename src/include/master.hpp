#pragma once

#include "socket.hpp"
#include "ioloop.hpp"
#include <sys/types.h>
#include <vector>


namespace godgun {
  namespace master {
    class Master {
    public:
      Master(int argc, char* argv[]);
      Master(const Master&) = delete;
      Master(Maser&&) = delete;
      Master& operator=(const Master&) = delete;
      ~Master();

      void accept() throw (socket::SocketError);

      void close();
      void shutdown();
      int fd();

    private:
      int _fd;
      struct sockaddr_in _addr;
      std::vector<ioloop::IOLoop&> _loops;
    }
  }
}
