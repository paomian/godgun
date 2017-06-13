#pragma once

#include <string>
#include <map>
#include "except.hpp"
#include "headerstr.hpp"

namespace godgun {
  namespace response {
    struct HttpResponse {
      unsigned int status_code = 200;
      std::string response_msg = "OK";
      std::string version = "HTTP/1.1";
      headerstr::HeaderMap headers;
      std::string body;
      bool close = false;

      std::string make_package();
    };
  }
}
