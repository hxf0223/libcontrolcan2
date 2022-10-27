#include <iostream>

#include "boost/asio.hpp"
#include "server/server.h"

namespace {
constexpr short kLisenPort = 9999;
}

int main(int /*argc*/, char** /*argv*/) {
  try {
    boost::asio::io_context io_context;
    Server s = Server(io_context, kLisenPort);
    s.startAccepting();
    io_context.run();
  } catch (boost::system::system_error& ec) {
    std::cout << ec.what() << std::endl;
  }

  return 0;
}