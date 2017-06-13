#include <sstream>
#include <cstring>
#include <iostream>
#include "include/request.hpp"

namespace godgun {
  namespace request {
    HttpRequest::ParseException::ParseException(const std::string& msg)
      : std::invalid_argument(msg) {}

    HttpRequest::ParseException::ParseException(const char *msg)
      : std::invalid_argument(msg) {}

    void HttpRequest::parse_headers(const std::string& raw_header) throw (ParseException) {
      std::stringstream stream;
      stream.str(raw_header);

      std::string line;
      getline(stream, line);

      headerstr::HeaderMap::iterator header_iter;
      std::string::size_type str_size;

      // Request line
      char *pnt = strtok(const_cast<char*>(line.c_str()), " ");
      if (!pnt) goto ERROR;
      this->method = pnt;
      pnt = strtok(nullptr, " ");
      if (!pnt) goto ERROR;
      this->uri = pnt;
      pnt = strtok(nullptr, " ");
      if (!pnt) goto ERROR;
      this->version = pnt;
      for (char& c : this->version) {
        c = toupper(c);
      }

      // Other headers
      headers.clear();
      while (getline(stream, line)) {
        if (line.length() == 0) break;
        pnt = strtok(const_cast<char *>(line.c_str()), ": ");
        if (!pnt) break;
        char *val = strtok(nullptr, ": ");
        if (!val) break;
        headers[pnt] = val;
      }

      header_iter = headers.find("Content-Type");
      if (header_iter == headers.end()) {
        headers["Content-Type"] = "application/octet-stream";
      }

      // Extract query string
      str_size = this->uri.find("?");
      this->params.clear();
      if (str_size != std::string::npos) {

        char *pnt = const_cast<char *>(this->uri.c_str());
        pnt += str_size + 1;
        pnt = strtok(pnt, "=");
        while (pnt) {
          char *val = strtok(nullptr, "&");
          if (!val) params[pnt] = "";
          // TODO: urldecode
          else params[pnt] = val;

          pnt = strtok(nullptr, "=");
        }

        this->uri.erase(str_size);

      }

      return;
    ERROR:
      throw ParseException("Malformed header");
    }

    void HttpRequest::parse_body(const std::string& raw_body) throw (ParseException) {

      this->raw_body = raw_body;

    }
  }
}
