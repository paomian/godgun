#include "include/ioloop.hpp"
#include "include/connection.hpp"
#include <getopt.h>
#include <signal.h>

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>


namespace godgun {
  namespace ioloop {
    IOLoopException::IOLoopException(const std::string& msg)
      : std::runtime_error(msg) {}

    IOLoopException::IOLoopException(const char* msg)
      : std::runtime_error(msg) {}

    IOLoop::IOLoop(int argc, char* argv[])
      : _started(false) {
      signal(SIGPIPE, SIG_IGN);
    }

    IOLoop::~IOLoop() {}

    void IOLoop::add_handler(int fd, int event, const EventCb& callback, void* arg) throw (IOLoopException) {
      _handlers[fd] = IOEvent(fd, callback, arg);
    }

    void IOLoop::toggle_callback(int fd, int type) {
      auto iter = _handlers.find(fd);
      if (iter != _handlers.end())
        iter -> second.callback(fd, type, iter -> second.arg, *this);
    }

    void IOLoop::remove_handler(int fd) throw (IOLoopException) {
      auto iter = _handlers.find(fd);
      if (iter != _handlers.end())
        _handlers.erase(iter);
    }

    int IOLoop::start() throw (IOLoopException) {
      _started = true;
      return 0;
    }

    void IOLoop::stop() throw (IOLoopException) {
      _started = false;
    }

    IOEvent::IOEvent() {}

    IOEvent::IOEvent(int fd, EventCb cb, void* arg)
      : fd(fd), callback(cb), arg(arg) {}

    EPollIOLoop::EPollIOLoop(int argc, char* argv[], const EPollIOLoop::HttpConnectionHandler& handler)
      : IOLoop(argc, argv),_handler(handler)  {
      if ((_epoll_fd = epoll_create(EPOLL_MAX_EVENT)) < 0) {
        perror("epoll_create");
        std::exit(EXIT_FAILURE);
      }

      if (pipe(_wake_pipe) < 0) {
        perror("pipe");
        std::exit(EXIT_FAILURE);
      }

      struct epoll_event ev;
      ev.events = EPOLLIN | EPOLLET;
      ev.data.fd = _wake_pipe[0];
      if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _wake_pipe[0], &ev)) {
        perror("epoll_ctl pipe");
        std::exit(EXIT_FAILURE);
      }
    }

    EPollIOLoop::~EPollIOLoop() {
      close(_epoll_fd);
      close(_wake_pipe[0]);
      close(_wake_pipe[1]);
    }

    void EPollIOLoop::add_handler(int fd, int type, const EventCb& callback, void* arg) throw (IOLoopException) {
      struct epoll_event epev;
      std::memset(&epev, 0, sizeof(epev));

      if (type & static_cast<int>(EventType::EV_READ)) epev.events |= EPOLLIN;
      if (type & static_cast<int>(EventType::EV_WRITE)) epev.events |= EPOLLOUT;

      epev.events |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLET);

      epev.data.fd = fd;

      if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &epev)) {
        perror("add_handler:epoll_ctl");
        throw IOLoopException("EPoll add error");
      }

      IOLoop::add_handler(fd, type, callback, arg);
    }

    void EPollIOLoop::update_handler(int fd, int type) throw (IOLoopException) {
      struct epoll_event epev;
      memset(&epev, 0, sizeof(epev));

      if (type & static_cast<int>(EventType::EV_READ)) epev.events |= EPOLLIN;
      if (type & static_cast<int>(EventType::EV_WRITE)) epev.events |= EPOLLOUT;

      epev.events |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLET);

      epev.data.fd = fd;

      if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &epev)) {
        perror("update_handler:epoll_ctl");
        throw IOLoopException("EPoll update error");
      }
    }

    void EPollIOLoop::remove_handler(int fd) throw (IOLoopException) {
      if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr)) {
        perror("remove_handler:epoll_ctl");
        throw IOLoopException("EPoll del error");
      }

      IOLoop::remove_handler(fd);
    }

    EPollIOLoop::ClientItem EPollIOLoop::pop_q() {
      std::lock_guard<std::mutex> guard(_queue_mutex);
      auto tmp = _queue.front();
      _queue.pop();
      return tmp;
    }

    bool EPollIOLoop::empty_q() {
      std::lock_guard<std::mutex> guard(_queue_mutex);
      return _queue.empty();
    }



    void EPollIOLoop::push_q(EPollIOLoop::ClientItem&& item) {
      std::lock_guard<std::mutex> guard(_queue_mutex);
      _queue.emplace(item);
    }

    int EPollIOLoop::start() throw (IOLoopException) {
      int active_event{0},wait_time{1};
      while (true) {
        if ((active_event = epoll_wait(_epoll_fd, _events.data(), _events.max_size(), wait_time)) >= 0) {
          if (active_event) {
            wait_time = 0;
            for (int i = 0; i < active_event; ++i) {
              int type{0};

              if (_events[i].data.fd == _wake_pipe[0]) goto FINISHED;
              if (_events[i].events & EPOLLIN) type |= static_cast<int>(EventType::EV_READ);
              if (_events[i].events & EPOLLOUT) type |= static_cast<int>(EventType::EV_WRITE);
              if (_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) type |= static_cast<int>(EventType::EV_ERROR);
              try {
                toggle_callback(_events[i].data.fd, type);
              } catch (std::exception& e) {
                std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << std::endl;
                goto FAILED;
              }
            }
          } else {
            wait_time = 10;
            if (!empty_q()) {
              auto tmp = pop_q();
              auto client = socket::SocketClient(tmp.first,tmp.second,true);
              new connection::HttpConnection(client,*this,_handler);
            }
          }
        } else {
          std::cout << "active:" << active_event << std::endl;
          perror("epoll wait");
          //break;
        }
      }
    FAILED:
      return EXIT_FAILURE;
    FINISHED:
      return EXIT_SUCCESS;
    }

    void EPollIOLoop::stop() throw (IOLoopException) {
      char buf[] = "x";
      write(_wake_pipe[1], buf, sizeof(buf));
    }
  }
}
