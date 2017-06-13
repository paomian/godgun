#include "include/master.hpp"
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

namespace godgun {
  namespace master {
    Master::Master(int argv, char* argvp[]) {
      std::memset(&_addr, 0, sizeof(_addr));
      _addr.sin_family = AF_INET;
      _addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port = ::htons(port);

      if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        std::abort();
      }
      int opt{1};
      setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      if (bind(_fd, _cast<struct sockaddr*>(&_addr), sizeof(addr)) < 0) {
        perror("bind");
        std::abort();
      }

      for(int i = 0; i < 4; ++i) {
        _loops.push_back()
      }
    }

  }
}
