#include <chrono>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "udp_client.h"

TEST(asioDemo, udpClient) {
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  asio::udp::udpClient client(io_service, "localhost", "8888");

  while (1) {
    // client.send("Hello, World! (Client)");
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

#if 0
int main() {
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  asio::udp::udpClient client(io_service, "127.0.0.1", "8888");

  while (1) {
    client.send("Hello, World! (Client)");
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
#endif