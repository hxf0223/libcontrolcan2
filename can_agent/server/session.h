#pragma once

#include <string>
#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "asio_zsocket.hpp"
#include "zmq.hpp"

class Session : public std::enable_shared_from_this<Session> {
private:
  boost::asio::ip::tcp::socket socket_;
  std::string address_;

  char rx_buffer_[1024]{0};
  std::vector<char> tx_buffer_;
  std::array<char, 1024> zrx_buff_;

  // zmq::context_t *ctx_;
  // zmq::socket_t ctx_sub_;
  asio_zmq::asioZmqReqSocket zsock_;

public:
  boost::asio::ip::tcp::socket &get_socket() { return socket_; }
  Session(boost::asio::io_context &ioContext, zmq::context_t *ctx);
  ~Session();

  void start();

  // write message to client which come from zmq's server
  void write_message(const boost::system::error_code &ec, std::size_t bytesTransfered);

  void handle_write(const boost::system::error_code &ec, std::size_t bytesTransfered);
  void handle_read(const boost::system::error_code &ec, std::size_t bytesTransfered);
  void handle_zmq_recv(const boost::system::error_code &ec, std::size_t bytesTransfered);
};