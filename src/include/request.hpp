#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include "headerstr.hpp"

namespace godgun {
  namespace request {
    struct Remote {
      std::string ip;
      uint16_t port;
    };

    struct HttpRequest {
      class ParseException : public std::invalid_argument {
      public:
        explicit ParseException(const std::string& msg);
        explicit ParseException(const char* msg);
      };

      std::string version = "HTTP/1.1";
      std::string method = "GET";
      std::string uri = "/";
      headerstr::HeaderMap headers;
      std::string raw_body;
      Remote remote;

      std::map<std::string, std::string> form;
      std::map<std::string, std::string> params;

      void parse_headers(const std::string& raw_header) throw (ParseException);
      void parse_body(const std::string& raw_body) throw (ParseException);
    };
  }
}
