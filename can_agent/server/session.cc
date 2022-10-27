#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <cstring>
#include <functional>
#include <ostream>
#include <thread>

#include "glog/logging.h"
#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "session.h"

Session::Session(boost::asio::io_context& ioContext, eventpp_queue_t& ppq)
    : socket_(ioContext)
    , deadline_(ioContext)
    , eventpp_q_(ppq) {
  std::cout << "Session created." << std::endl;
  shr_tx_buff_.prepare(2048);
}

Session::~Session() {
  disconnect_to_eventqq();
  std::cout << "Session terminated." << std::endl;
}

void Session::start() {
  ppq_handle_ = eventpp_q_.appendListener(ppq_can_obj_evt_id,
                                          std::bind(&Session::consume_can_obj_handler, shared_from_this(), std::placeholders::_1));
  const boost::system::error_code ec;
  do_can_obj_transpose(ec);
}

void Session::disconnect_to_eventqq() {
  if (ppq_handle_.use_count() > 0) {
    // remove listener to let this session destroied
    eventpp_q_.removeListener(ppq_can_obj_evt_id, ppq_handle_);
  }
}

void Session::consume_can_obj_handler(const canobj_queue_node_t& node) {
  if (!spsc_queue_.push(node)) {
    LOG(WARNING) << "Session::consume_can_obj_handler fail.";
  }
}

void Session::do_can_obj_transpose(const boost::system::error_code& ec) {
  canobj_queue_node_t node;
  for (size_t i = 0; i < 8; i++) {
    if (!spsc_queue_.pop(node)) {
      break;
    }

    std::ostream os(&shr_tx_buff_);
    os << node.can_obj_;
  }
  if (shr_tx_buff_.size() > 0) {
    writeMessage();
    return;
  }

  auto self = shared_from_this();
  deadline_.expires_from_now(boost::posix_time::milliseconds(10));
  deadline_.async_wait(boost::bind(&Session::do_can_obj_transpose, self, boost::asio::placeholders::error));
}

void Session::writeMessage() {
  auto self = shared_from_this();
  boost::asio::async_write(
      socket_, shr_tx_buff_,
      boost::bind(&Session::handleWrite, self, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Session::handleWrite(const boost::system::error_code& ec, std::size_t bytesTransfered) {
  shr_tx_buff_.consume(bytesTransfered);

  if (ec) {
    LOG(WARNING) << "Session::handle_write " << ec.what();
    disconnect_to_eventqq();
    return;
  }

  const boost::system::error_code ec2;
  do_can_obj_transpose(ec2);
}

void Session::handleRead(const boost::system::error_code& ec, std::size_t /*bytesTransfered*/) {
  if (!ec) {
  }
}
