#pragma once

#include <sys/epoll.h>
#include <sys/types.h>

#include <functional>
#include <array>
#include <stdexcept>
#include <string>
#include <memory>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <utility>

namespace godgun {
  namespace ioloop {
    class IOLoop;
    using EventCb = std::function<void (int, int, void*, IOLoop&)>;
    class IOLoopException : public std::runtime_error {
    public:
      explicit IOLoopException(const std::string& msg);
      explicit IOLoopException(const char* msg);
    };

    enum class EventType:int {
      EV_READ  = 0x1,
      EV_WRITE = 0x2,
      EV_ERROR = 0x4
    };

    enum class EventFlag:int {
      EV_INITED  = 0x1,
      EV_ADDED   = 0x2,
      EV_ACTIVE  = 0x4,
      EV_DELETEd = 0x8
    };

    struct IOEvent {
      int fd;
      EventCb callback;
      void* arg;

      IOEvent();
      IOEvent(int, EventCb, void*);
    };

    class IOLoop {
    public:
      IOLoop(int argc, char* argv[]);
      virtual ~IOLoop();

      virtual void add_handler(int fd, int event, const EventCb& callback, void* arg = nullptr) throw (IOLoopException);
      virtual void update_handler(int fd, int evnet) throw (IOLoopException) = 0;
      virtual void remove_handler(int fd) throw (IOLoopException);

      virtual int start() throw (IOLoopException);
      virtual void stop() throw (IOLoopException);

    protected:
      void toggle_callback(int fd, int type);

    private:
      std::unordered_map<int, IOEvent> _handlers;
      bool _started;
    };

    class EPollIOLoop : public IOLoop {
    public:

      using ClientItem = std::pair<int, std::shared_ptr<struct sockaddr_in>>;
      EPollIOLoop(int argc, char* argv[]);
      virtual ~EPollIOLoop();

      virtual void add_handler(int fd, int event, const EventCb& callback, void* arg = nullptr) throw (IOLoopException) override;
      virtual void update_handler(int fd, int evnet) throw (IOLoopException) override;
      virtual void remove_handler(int fd) throw (IOLoopException);

      virtual int start() throw (IOLoopException) override;
      virtual void stop() throw (IOLoopException) override;
      ClientItem& pop();
      void push(ClientItem& item);

    private:
      int _epoll_fd;

      static const size_t EPOLL_MAX_EVENT = 1024;
      std::array<struct epoll_event, EPOLL_MAX_EVENT> _events;
      std::queue<ClientItem> _queue;
      std::mutex _queue_mutex;


      int _wake_pipe[2];
    };
  }
}
