#include "session.h"
#include <boost/asio/placeholders.hpp>
#include <chrono>
#include <cstring>
#include <thread>

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"

Session::Session(boost::asio::io_context &io_context) : socket_(io_context), tx_buffer_(1024) {}
Session::~Session() { std::cout << "Session terminated." << std::endl; }

void Session::start() {
  auto self = shared_from_this();

#if 0
  socket_.async_read_some(boost::asio::buffer(&rx_buffer_, 1024),
                          boost::bind(&Session::handle_read, self, boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
#else
  write_message();
#endif
}

void Session::handle_write(const boost::system::error_code &ec, std::size_t bytesTransfered) {
  if (!ec) {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    write_message();
  }
}

void Session::write_message() {
  const char *cmd_recv = "VCI_Receive,";
  uint64_t send_count = 0;
  char send_buff[256];

  VCI_CAN_OBJ can_obj{};
  *(uint64_t *)(can_obj.Data) = send_count;
  auto dur = std::chrono::system_clock::now().time_since_epoch();
  uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

  auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj, "\n");
  memcpy(&tx_buffer_[0], send_buff, size);

  auto self = shared_from_this();
  socket_.async_write_some(boost::asio::buffer(tx_buffer_, size),
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