#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>

class UDPServer {
public:
  UDPServer(boost::asio::io_service &io_service, const std::string &port)
    : socket_(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), std::stoi(port))) {
    // startReceive();
    // recvBuffer_.fill(0);
    start_send();
  }

private:
  void startReceive();
  void handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred);

  void start_send();
  void handle_send(const boost::system::error_code &ec);

  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint remote_endpoint_;
  std::array<char, 1024> recvBuffer_{{0}};
  std::string data_;
};
