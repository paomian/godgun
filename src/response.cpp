#include "include/response.hpp"

#include <sstream>
#include <iostream>

namespace godgun {
  namespace response {
    std::string HttpResponse::make_package() {
      std::stringstream stream;
      stream << version << " " << status_code << " " << response_msg << "\r\n";

      auto itr = headers.find("Content-Type");
      if (itr == headers.end())
        headers["Content-Type"] = "text/plain";

      for (auto& itr : headers) {
        stream << itr.first << ": " << itr.second << "\r\n";
      }

      if (body.size() != 0) {
        stream << "Content-Length: " << body.size() << "\r\n";
      }

      stream << "\r\n";
      stream << body;

      return std::move(stream.str());
    }
  }
}

