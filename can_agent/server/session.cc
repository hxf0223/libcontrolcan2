#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <cstring>
#include <functional>
#include <thread>

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "session.h"

Session::Session(boost::asio::io_context &ioContext, eventpp_queue_t &ppq)
  : socket_(ioContext), deadline_(ioContext), eventpp_q_(ppq) {
  ppq_handle_ = eventpp_q_.appendListener(1, std::bind(&Session::consume_can_obj_handler, this, std::placeholders::_1));
}

Session::~Session() {
  eventpp_q_.removeListener(1, ppq_handle_);
  std::cout << "Session terminated." << std::endl;
}

void Session::consume_can_obj_handler(const canobj_queue_node_t &node) {
  if (!spsc_queue_.push(node)) {
    std::cout << "Session::consume_can_obj_handler fail." << std::endl;
  }
}

void Session::start() {
  boost::system::error_code ec;
  do_can_obj_transpose(ec);
}

void Session::do_can_obj_transpose(const boost::system::error_code &ec) {
  if (spsc_queue_.pop(can_obj_)) {
    write_message();
    return;
  }

  auto self = shared_from_this();
  deadline_.expires_from_now(boost::posix_time::microseconds(10));
  deadline_.async_wait(boost::bind(&Session::do_can_obj_transpose, self, boost::asio::placeholders::error));
}

void Session::handle_write(const boost::system::error_code &ec, std::size_t bytesTransfered) {
  if (ec) {
    std::cout << "Session::handle_write " << ec.what() << std::endl;
    return;
  }

  boost::system::error_code ec2;
  do_can_obj_transpose(ec2);
}

void Session::write_message() {
  auto self = shared_from_this();
  socket_.async_write_some(boost::asio::buffer(can_obj_.can_obj_, can_obj_.len_),
                           boost::bind(&Session::handle_write, self, boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
}

void Session::handle_read(const boost::system::error_code &ec, std::size_t bytesTransfered) {
  if (!ec) {
  }
}
