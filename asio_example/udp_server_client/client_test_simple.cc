#include <boost/asio.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

TEST(asioDemo, udpClient2) {
  namespace ip = boost::asio::ip;
  boost::asio::io_service io_service;

  // Client binds to any address on port 8888 (the same port on which broadcast data is sent from server).
  ip::udp::socket socket(io_service, ip::udp::endpoint(ip::udp::v4(), 8888));
  ip::udp::endpoint sender_endpoint;

  size_t rx_count = 0;
  char buf[500] = {0};
  while (1) {
    std::size_t bytes_transferred = socket.receive_from(boost::asio::buffer(buf), sender_endpoint);
    LOG(INFO) << ++rx_count << ". receive " << bytes_transferred << " bytes. data: " << buf;
  }
}
