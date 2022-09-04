#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "udp_server.h"

namespace asio {
namespace udp {

void udpServer::start_send() {
  auto self = shared_from_this();
  deadline_.expires_from_now(boost::posix_time::milliseconds(500));

  deadline_.async_wait([self](const boost::system::error_code &ec) {
    auto message = std::make_shared<std::string>("hello world from server.\n");
    self->socket_.async_send_to(
        boost::asio::buffer(*message), self->remote_endpoint_,
        boost::bind(&udpServer::handle_send, self,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  });
}

void udpServer::handle_send(const boost::system::error_code &ec,
                            std::size_t bytesTransfered) {
  if (ec) {
    std::cout << "server error: " << ec.what() << std::endl;
    return;
  }

  std::cout << "send count " << ++tx_count_ << std::endl;
  start_send();
}

} // namespace udp
} // namespace asio