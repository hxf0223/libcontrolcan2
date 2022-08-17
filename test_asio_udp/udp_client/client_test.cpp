#include "UDPClient.h"
#include <chrono>
#include <thread>

int main() {
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  UDPClient client(io_service, "127.0.0.1", "1111");

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