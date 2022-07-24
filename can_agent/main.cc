#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"

#include "boost/asio.hpp"
#include "server/server.h"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;
constexpr short lisenPort = 9999;

int main(int argc, char **argv) {
  try {
    boost::asio::io_context io_context;
    Server s = Server(io_context, lisenPort);
    s.startAccepting();
    io_context.run();
  } catch (boost::system::system_error &ec) {
    std::cout << ec.what() << std::endl;
  }

  return 0;
}