#include <boost/asio.hpp>
#include <chrono>
#include <string>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

TEST(asio, udpServer) {
  boost::asio::io_service my_io_service;
  boost::asio::ip::udp::endpoint local_endpoint(boost::asio::ip::udp::v4(), 7777);
  boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 2300);
  // don't  fill (ip::udp::v4()) in the first parameter,it will cause that the contents are seny out the failure!
  boost::asio::ip::udp::socket socket(my_io_service, local_endpoint); // create socket and bind the endpoint

  const char *send_data = "hello! my name is Bojie. Can you see me?"; /*the contents to be sent*/

  try {
    while (1) {
      socket.send_to(boost::asio::buffer(send_data, strlen(send_data) + 1), remote_endpoint);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

TEST(asio, udpClient) {
  boost::asio::io_service my_io_service;
  boost::asio::ip::udp::endpoint local_endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 2300);
  boost::asio::ip::udp::endpoint romote_endpoint;
  boost::asio::ip::udp::socket socket(my_io_service, local_endpoint);

  char buffer[40000];
  int nAdd = 0;

  while (1) {
    memset(buffer, 0, 40000);
    nAdd++;
    socket.receive_from(boost::asio::buffer(buffer, 40000), romote_endpoint); // receive data from  remote-computer
    printf("recv %d datapacket:%s\n", nAdd, buffer);
  }
}