#include "session.h"
#include <boost/asio/placeholders.hpp>
#include <chrono>
#include <cstring>
#include <thread>

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "zmq_addon.hpp"

Session::Session(boost::asio::io_context &ioContext) : socket_(ioContext), tx_buffer_(1024) {}
Session::~Session() { std::cout << "Session terminated." << std::endl; }

void Session::start() {
  auto self = shared_from_this();

  socket_.async_read_some(boost::asio::buffer(&rx_buffer_, 1024),
                          boost::bind(&Session::handle_read, self, boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

void Session::handle_zmq_recv(const boost::system::error_code &ec, std::size_t bytesTransfered) {}

void Session::handle_write(const boost::system::error_code &ec, std::size_t bytesTransfered) {
  if (!ec) {
  }
}

void Session::write_message(const boost::system::error_code &ec, std::size_t bytesTransfered) {
  auto self = shared_from_this();
  socket_.async_write_some(boost::asio::buffer(tx_buffer_, bytesTransfered),
                           boost::bind(&Session::handle_write, self, boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
}

void Session::handle_read(const boost::system::error_code &ec, std::size_t bytesTransfered) {
  if (!ec) {
    std::cout << "Bytes: " << bytesTransfered << std::endl;
    rx_buffer_[bytesTransfered] = '\0';

    std::cout << "Message: " << rx_buffer_ << std::endl;
    start();
  }
}
