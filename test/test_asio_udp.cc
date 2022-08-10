#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <chrono>
#include <string>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

constexpr short udpPort = 9999;

class udpBroadCastServer {
public:
  // https://stackoverflow.com/questions/49346350/read-boostasio-udp-broadcast-response
  udpBroadCastServer(boost::asio::io_context &service, unsigned int port)
    : broadcast_endpoint_(boost::asio::ip::address_v4::broadcast(), port), socket_(service) {
    socket_.open(boost::asio::ip::udp::v4());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.set_option(boost::asio::socket_base::broadcast(true));

    async_send_data();
  }

  void async_send_data() {
    socket_.async_send_to(boost::asio::buffer(buffer_, 2), broadcast_endpoint_,
                          boost::bind(&udpBroadCastServer::handle_send, this, boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
  }

  void handle_receive(const boost::system::error_code &error, std::size_t bytes_transferred) {
    std::cout << "Received Data" << bytes_transferred << std::endl;
  }

  void handle_send(const boost::system::error_code &error, std::size_t bytes_transferred) {
    std::cout << "Sent byte num: " << bytes_transferred << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    async_send_data();
  }

private:
  char buffer_[128];
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint broadcast_endpoint_;
};

TEST(asio, udpServer) {
  boost::asio::io_context service;
  udpBroadCastServer ser(service, udpPort);
  service.run();
}

class listener {
  using ec_t = boost::system::error_code;

public:
  listener(const boost::asio::ip::udp::endpoint &listen_endpoint)
    : socket_(io_service_, listen_endpoint), deadline_(io_service_) {
    deadline_.expires_at(boost::posix_time::pos_infin);
    check_deadline();
  }

  std::size_t receive(const boost::asio::mutable_buffer &buffer, boost::posix_time::time_duration timeout,
                      boost::system::error_code &ec) {
    deadline_.expires_from_now(timeout);
    ec = boost::asio::error::would_block;
    std::size_t length = 0;
    boost::asio::ip::udp::endpoint unused_sender_endpoint;
    socket_.async_receive_from(boost::asio::buffer(buffer), unused_sender_endpoint,
                               boost::bind(&listener::handle_receive, _1, _2, &ec, &length));

    // TODO: The following do/while is hinky. Does run_one() need to happen before the comparison?
    do {
      io_service_.run_one();
    } while (ec == boost::asio::error::would_block);

    return length;
  }

private:
  void check_deadline() {
    if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      // cancel() won't work on XP. Something about using close() instead... Look it up. I'm doing this on Win10.
      socket_.cancel();
      deadline_.expires_at(boost::posix_time::pos_infin);
    }
    deadline_.async_wait(boost::bind(&listener::check_deadline, this));
  }

  static void handle_receive(const ec_t &ec, std::size_t length, ec_t *out_ec, std::size_t *out_length) {
    *out_ec = ec;
    *out_length = length;
  }

private:
  boost::asio::io_service io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::deadline_timer deadline_;
};

TEST(asio, udpClient) {
  boost::asio::ip::udp::endpoint listen_endpoint(boost::asio::ip::address::from_string("0.0.0.0"), udpPort);
  listener c(listen_endpoint);

  for (;;) {
    char data[1024];
    boost::system::error_code ec;
    auto n = c.receive(boost::asio::buffer(data), boost::posix_time::milliseconds{100}, ec);
    if (!ec) {
      std::cout << "Received " << n << " bytes." << std::endl;
    }
  }
}