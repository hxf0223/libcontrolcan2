#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>

class UDPServer {
public:
  UDPServer(const std::string &address, const std::string &multicast_address, const unsigned short multicast_port)
    : work_(io_service_),
      socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(address), 0)),
      multicast_endpoint_(boost::asio::ip::address::from_string(multicast_address), multicast_port) {
    // startReceive();
    // recvBuffer_.fill(0);

    auto self = this;
    work_thread_ = std::thread([self]() { self->io_service_.run(); });
    start_send();
  }

  ~UDPServer() {
    io_service_.stop();
    work_thread_.join();
  }

private:
  void startReceive();
  void handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred);

  void start_send();
  void handle_send(const boost::system::error_code &ec, std::size_t bytesTransfered);

  boost::asio::io_service io_service_;
  boost::asio::io_service::work work_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint multicast_endpoint_;
  std::thread work_thread_;

  std::array<char, 1024> recvBuffer_{{0}};
  std::string data_;
};
