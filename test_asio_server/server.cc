#include <iostream>

#include "boost/asio.hpp"
#include "server/server.h"

#include "zmq.hpp"
#include "zmq_addon.hpp"

namespace {
constexpr short lisenPort = 9999;
}

int main(int argc, char **argv) {
  zmq::context_t ctx;

  try {
    boost::asio::io_context io_context;
    Server s = Server(io_context, lisenPort, &ctx);
    s.startAccepting();
    io_context.run();
  } catch (boost::system::system_error &ec) {
    std::cout << ec.what() << std::endl;
  }

  return 0;
}