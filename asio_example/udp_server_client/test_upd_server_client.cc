#include <atomic>
#include <chrono>
#include <memory>
#include <signal.h>
#include <thread>

#include <boost/asio.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "udp_client.h"
#include "udp_server.h"

TEST(asioDemo, udpServer) {
  try {
    auto server = std::make_shared<asio::udp::udpServer>(8888);
    server->start_send();
    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  } catch (const std::exception &ex) {
    LOG(ERROR) << ex.what();
  }

  LOG(INFO) << "server exit.";
}

TEST(asioDemo, udpClient) {
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  asio::udp::udpClient client(io_service, "localhost", "8888");

  while (1) {
    std::string data = client.receive(boost::posix_time::seconds(10), ec);
    if (ec) {
      std::cout << "Receive error: " << ec.message() << "\n";
    } else {
      std::cout << "Received: ";
      std::cout << data;
      std::cout << "\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}