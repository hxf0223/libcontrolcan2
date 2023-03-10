#pragma once

#include <string>
#define _CRT_SECURE_NO_WARNINGS

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

class Session : public std::enable_shared_from_this<Session> {
private:
  boost::asio::ip::tcp::socket socket_;
  std::string address_;

  char rx_buffer_[1024]{0};
  std::vector<char> tx_buffer_;

public:
  boost::asio::ip::tcp::socket& getSocket() {
    return socket_;
  }
  Session(boost::asio::io_context& ioContext);
  ~Session();

  void start();
  void writeMessage();

  void handleWrite(const boost::system::error_code& ec, std::size_t bytesTransfered);
  void handleRead(const boost::system::error_code& ec, std::size_t bytesTransfered);
};