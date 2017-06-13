#pragma once

#include <functional>
#include "socket.hpp"
#include "request.hpp"
#include "response.hpp"
#include "stream.hpp"


namespace godgun {
  namespace ioloop {
    class IOLoop;
  }
  namespace connection {
    class HttpConnection {
    public:
      using HttpConnectionHandler = std::function<void (const request::HttpRequest&, response::HttpResponse&)>;

      HttpConnection(const socket::SocketClient& client, ioloop::IOLoop& loop, const HttpConnectionHandler& handler);
      ~HttpConnection();

      void set_close_callback(const std::function<void (HttpConnection *)>&);
      void close();

    private:
      void __stream_handler_get_header(const std::string&, stream::IOStream&) noexcept;
      void __stream_handler_get_body(const std::string&, stream::IOStream&) noexcept;
      void __stream_handler_on_write(stream::IOStream&) noexcept;

      stream::IOStream _stream;
      HttpConnectionHandler _handler;
      request::HttpRequest _request;

      std::function<void (HttpConnection *)> _close_callback;
      bool _closed;
      bool _close_after_finished;
    };
  }
}
