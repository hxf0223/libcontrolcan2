#include "udp_client.h"

namespace asio {
namespace udp {

void udpClient::send(const std::string &msg) {
  socket_.send_to(boost::asio::buffer(msg, msg.size()), endpoint_);
}

std::string udpClient::receive(boost::posix_time::time_duration timeout,
                               boost::system::error_code &ec) {
  deadline_.expires_from_now(timeout);
  ec = boost::asio::error::would_block;
  std::size_t length = 0;

  socket_.async_receive(
      boost::asio::buffer(recvBuffer_),
      boost::bind(&udpClient::handle_receive, _1, _2, &ec, &length));

  do {
    io_service_.run_one();
  } while (ec == boost::asio::error::would_block);

  data_ = std::string(std::begin(recvBuffer_), std::end(recvBuffer_));
  recvBuffer_.fill(0);
  return data_;
}

void udpClient::check_deadline() {
  if (deadline_.expires_at() <= deadline_timer::traits_type::now()) {
    socket_.cancel();
    deadline_.expires_at(boost::posix_time::pos_infin);
  }

  deadline_.async_wait(boost::bind(&udpClient::check_deadline, this));
}

} // namespace udp
} // namespace asio