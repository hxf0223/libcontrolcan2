#include <atomic>
#include <chrono>
#include <memory>
#include <signal.h>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "udp_server.h"

TEST(asioDemo, udpServer) {
  try {
    auto server = std::make_shared<asio::udp::udpServer>(8888);
    server->start_send();
    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  } catch (const std::exception &ex) {
    // std::cerr << ex.what() << std::endl;
    LOG(ERROR) << ex.what();
  }

  LOG(INFO) << "server exit.";
}
