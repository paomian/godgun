#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <deque>
#include <stdexcept>
#include <thread>
#include <mutex>
#include "ioloop.hpp"
#include "socket.hpp"

namespace godgun {
  namespace stream {
    class IOStreamException : public std::runtime_error {
    public:
      explicit IOStreamException(const char *);
      explicit IOStreamException(const std::string&);
    };

    class IOStream {
    public:
      IOStream(const socket::SocketClient& client, ioloop::IOLoop& ioloop);
      ~IOStream();

      using ReadHandler = std::function<void (const std::string& data, IOStream& stream)>;
      using WriteHandler = std::function<void (IOStream& stream)>;

      void read_bytes(size_t len, const ReadHandler& handler = ReadHandler());
      void read_until(const char *until, const ReadHandler& handler = ReadHandler());

      void write_bytes(const void *buffer, size_t len, const WriteHandler& handler = WriteHandler());

      socket::SocketClient client() const;
      void set_close_callback(const std::function<void (IOStream *)>&);
      void close();
    private:
      void __handler_poll(int fd, int type, void *arg, ioloop::IOLoop& loop);
      size_t __read_to_buffer(socket::SocketClient *client) throw (IOStreamException);
      bool __read_from_buffer();

      socket::SocketClient _client;
      ioloop::IOLoop& _ioloop;
      bool _doing;
      bool _write_buf_freezing;
      int _read_num;
      const char *_read_until;
      ReadHandler _read_handler;
      WriteHandler _write_handler;

      std::function<void (IOStream *)> _close_callback;

      int _send_buf_size;
      int _recv_buf_size;
      char* _send_buf;
      char* _recv_buf;
      std::deque<char> _rdbuf;
      std::deque<char> _wrbuf;
      bool _closed;
    };
  }
}

