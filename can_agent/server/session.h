#pragma once

#include <string>
#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "canobj_queue_type.h"

class Session : public std::enable_shared_from_this<Session> {
private:
  boost::asio::ip::tcp::socket socket_;
  boost::asio::deadline_timer deadline_;

  spsc_queue_t<boost::lockfree::capacity<2048>> spsc_queue_;
  boost::asio::streambuf shr_tx_buff_;
  eventpp_queue_t& eventpp_q_;
  eventpp_queue_handle_t ppq_handle_;

public:
  boost::asio::ip::tcp::socket& getSocket() {
    return socket_;
  }
  Session(boost::asio::io_context& ioContext, eventpp_queue_t& ppq);
  ~Session();

  void start();

private:
  void disconnectToEventqq();
  void consumeCanObjHandler(const CanobjQueueNodeT& node);
  void doCanObjTranspose(const boost::system::error_code& ec);

  void writeMessage();
  void handleWrite(const boost::system::error_code& ec, std::size_t bytesTransfered);
  void handleRead(const boost::system::error_code& ec, std::size_t bytesTransfered);
};