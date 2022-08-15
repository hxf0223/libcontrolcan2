#ifndef UDPSERVER_H_
#define UDPSERVER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::udp;

class UDPServer {
public:
  UDPServer(boost::asio::io_service &io_service, const std::string &port)
    : socket_(io_service, udp::endpoint(udp::v4(), std::stoi(port))) {
    startReceive();
    recvBuffer_.fill(0);
  }

private:
  void startReceive();
  void handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred);

  udp::socket socket_;
  udp::endpoint remoteEndpoint_;
  std::array<char, 1024> recvBuffer_{{0}};
  std::string data_;
};

#endif