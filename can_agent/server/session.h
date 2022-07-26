#pragma once

#include <string>
#define _CRT_SECURE_NO_WARNINGS

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <queue>

class Session : public std::enable_shared_from_this<Session> {
private:
  boost::asio::ip::tcp::socket socket_;

  std::string address_;
  char rx_buffer_[1024]{0};
  std::vector<char> tx_buffer_;
  std::queue<std::string> tx_queue_;

public:
  boost::asio::ip::tcp::socket &get_socket() { return socket_; }
  Session(boost::asio::io_context &io_context);
  ~Session();

  void start();
  void write_message();

  void handle_write(const boost::system::error_code &ec, std::size_t bytesTransfered);
  void handle_read(const boost::system::error_code &ec, std::size_t bytesTransfered);
};