#include "include/connection.hpp"
#include "include/except.hpp"
#include "include/ioloop.hpp"

#include <functional>
#include <iostream>
#include <ctime>

namespace godgun {
  namespace connection {
    HttpConnection::HttpConnection(const socket::SocketClient& client, ioloop::IOLoop& loop, const HttpConnectionHandler& handler)
      : _stream(client, loop), _handler(handler), _closed(false), _close_after_finished(false) {

      _stream.set_close_callback([this] (stream::IOStream *) { this->close(); });

      _stream.read_until("\r\n\r\n", [this] (const std::string& data, stream::IOStream& stream) {
          __stream_handler_get_header(data, stream); });
    }

    HttpConnection::~HttpConnection() {
      this->close();
    }

    void HttpConnection::__stream_handler_get_header(const std::string& data, stream::IOStream& stream) noexcept {
      long len;
      _request.remote.ip = stream.client().ip();
      _request.remote.port = stream.client().port();
      try {
        _request.parse_headers(data);
      }
      catch (request::HttpRequest::ParseException& except) {

        std::clog << __FILE__ << ":" << __LINE__ << " ";
        std::clog << except.what() << std::endl;

        goto restart;
      }

      len = strtol(_request.headers["Content-Length"].c_str(), nullptr, 10);
      stream.read_bytes(len, [this] (const std::string& data, stream::IOStream& stream) {
          __stream_handler_get_body(data, stream); });
      return;
    restart:
      if (!_close_after_finished)
        stream.read_until("\r\n\r\n", [this] (const std::string& data, stream::IOStream& stream) {
            __stream_handler_get_header(data, stream); });
    }

    void HttpConnection::__stream_handler_get_body(const std::string& data, stream::IOStream& stream) noexcept {
      response::HttpResponse response;
      char timebuf[32] = {0};
      time_t rawtime;
      struct tm *timeinfo;
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      strftime(timebuf, sizeof(timebuf), "%FT%TZ%z", timeinfo);

      try {
        this -> _request.parse_body(data);

        this -> _handler(_request, response);

        try {
          if (headerstr::ci_equal(_request.headers["Connection"], "Close")) {
            _close_after_finished = true;
          }
        }
        catch (std::out_of_range& except) {}

        if (response.close)
          _close_after_finished = true;

        std::clog << "[" << timebuf << "] " << _request.remote.ip << ":" << _request.remote.port << " ";
        std::clog << _request.method << " " << _request.uri << " ";
        std::clog << response.status_code << " " << response.response_msg << std::endl;
        std::string resp = response.make_package();
        stream.write_bytes(resp.c_str(), resp.length(), [this] (stream::IOStream& stream) { this->__stream_handler_on_write(stream); });
      }
      catch (except::HttpError& error) {
        std::clog << "[" << timebuf << "] " << _request.remote.ip << ":" << _request.remote.port << " ";
        std::clog << _request.method << " " << _request.uri << " ";
        std::string resp = error.make_package();
        std::clog << error.status_code() << " " << error.msg() << std::endl;
        stream.write_bytes(resp.c_str(), resp.length());
      }
      catch (request::HttpRequest::ParseException& except) {

        std::clog << __FILE__ << ":" << __LINE__ << " ";
        std::clog << except.what() << std::endl;

      }

      if (!_close_after_finished)
        stream.read_until("\r\n\r\n", [this] (const std::string& data, stream::IOStream& stream) {
            __stream_handler_get_header(data, stream); });
    }

    void HttpConnection::close() {
      if (_closed) return;
      _closed = true;
      _stream.close();

      if (_close_callback)
        _close_callback(this);
    }

    void HttpConnection::set_close_callback(const std::function<void (HttpConnection *)>& cb) {
      _close_callback = cb;
    }

    void HttpConnection::__stream_handler_on_write(stream::IOStream& stream) noexcept {
      if (_close_after_finished)
        this->close();
    }
  }
}
