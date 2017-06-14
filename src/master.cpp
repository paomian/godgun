#include "include/master.hpp"
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

namespace godgun {
  namespace master {
    Master::Master(int argc, char* argv[], const ioloop::EPollIOLoop::HttpConnectionHandler& handler) {
      std::memset(&_addr, 0, sizeof(_addr));
      _addr.sin_family = AF_INET;
      _addr.sin_addr.s_addr = INADDR_ANY;
      _addr.sin_port = htons(8080);

      if ((_fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        std::abort();
      }
      int opt{1};
      setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      if (bind(_fd, reinterpret_cast<struct sockaddr*>(&_addr), sizeof(_addr)) < 0) {
        perror("bind");
        std::abort();
      }

      for(int i = 0; i < 16; ++i) {
        auto tmp = new ioloop::EPollIOLoop(argc, argv, handler);
        _loops.push_back(tmp);
      }
      ::listen(_fd, 200);
    }

    Master::~Master() {
      this -> close();
    }

    void Master::start() {
      for(int i = 0; i < 16; ++i) {
        std::thread([this,i] {
            _loops[i] -> start();
          }).detach();
      }
      int cfd{-1},i{0};
      struct sockaddr_in remote_addr;
      socklen_t sin_size = sizeof(struct sockaddr_in);
      while(true) {
        if ((cfd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&remote_addr), &sin_size)) < 0) {
          perror("accept");
        }
        auto x = (i + 1) % 16;
        reinterpret_cast<ioloop::EPollIOLoop*>(_loops[0]) -> push_q(std::make_pair(cfd,remote_addr));
      }
    }
    void Master::close() {
      if (::close(_fd) < 0) {
        perror("close");
      }
    }

    int Master::fd() const {
      return _fd;
    }

    void Master::shutdown(int how) {
      if (::shutdown(_fd, how) < 0) {
        perror("shutdown");
      }
    }
  }
}
