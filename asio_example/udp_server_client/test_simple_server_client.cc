#include <boost/asio.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

TEST(asioDemo, udpServer2) {
  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket socket(io_service);
  boost::asio::ip::udp::endpoint local_endpoint;
  boost::asio::ip::udp::endpoint remote_endpoint;

  socket.open(boost::asio::ip::udp::v4());
  socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
  socket.set_option(boost::asio::socket_base::broadcast(true));
  local_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), 3999);
  remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), 8888);

  try {
    socket.bind(local_endpoint);
    while (true) {
      socket.send_to(boost::asio::buffer("abc", 3), remote_endpoint);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  } catch (boost::system::system_error e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(asioDemo, udpClient2) {
  namespace ip = boost::asio::ip;
  boost::asio::io_service io_service;

  // Client binds to any address on port 8888 (the same port on which broadcast data is sent from server).
  ip::udp::socket socket(io_service, ip::udp::endpoint(ip::udp::v4(), 8888));
  LOG(INFO) << "create socket success.";
  ip::udp::endpoint sender_endpoint;

  size_t rx_count = 0;
  char buf[500] = {0};
  while (1) {
    std::size_t bytes_transferred = socket.receive_from(boost::asio::buffer(buf), sender_endpoint);
    LOG(INFO) << ++rx_count << ". receive " << bytes_transferred << " bytes. data: " << buf;
  }
}