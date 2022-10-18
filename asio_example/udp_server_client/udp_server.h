#pragma once

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

namespace asio {
namespace udp {

namespace ip = boost::asio::ip;

class udpServer : public std::enable_shared_from_this<udpServer> {
public:
  udpServer(uint16_t broadcastPort)
      : work_(io_context_)
      , socket_(io_context_, ip::udp::endpoint(ip::udp::v4(), 0))
      , remote_endpoint_(ip::address_v4::broadcast(), broadcastPort)
      , deadline_(io_context_)
      , tx_count_(0) {

    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.set_option(boost::asio::socket_base::broadcast(true));

    auto self = this;
    work_thread_ = std::thread([self]() {
      self->io_context_.run();
    });
  }

  ~udpServer() {
    io_context_.stop();
    work_thread_.join();
  }

  void start_send();

private:
  void handle_send(const boost::system::error_code& ec, std::size_t bytesTransfered);

  boost::asio::io_context io_context_;
  boost::asio::io_context::work work_;
  ip::udp::socket socket_;
  ip::udp::endpoint remote_endpoint_;
  std::thread work_thread_;

  boost::asio::deadline_timer deadline_;
  size_t tx_count_; // for debug only
};

} // namespace udp
} // namespace asio