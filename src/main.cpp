#include "include/request.hpp"
#include "include/request.hpp"
#include "include/master.hpp"

namespace godgun{
  void handler(const request::HttpRequest& req, response::HttpResponse& resp) {
    resp.body = "{\"hello\":1}";
  }
}

int main(int argc, char* argv[]) {
  godgun::master::Master master(argc, argv,godgun::handler);
  master.start();
}
