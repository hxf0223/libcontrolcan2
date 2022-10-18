//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/detail/error_code.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

enum { max_length = 1024 };

class match_char {
public:
  explicit match_char(char c)
      : c_(c) {}

  template <typename Iterator>
  std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const {
    Iterator i = begin;
    while (i != end)
      if (c_ == *i++)
        return std::make_pair(i, true);
    return std::make_pair(i, false);
  }

private:
  char c_;
};

class tcpClient : public std::enable_shared_from_this<tcpClient> {
public:
  tcpClient(boost::asio::io_context& ioContext, const std::string& servIp, const std::string& servPort)
      : socket_(ioContext)
      , resolver_(ioContext) {
    boost::asio::connect(socket_, resolver_.resolve(servIp, servPort));
  }

  void start_receive() {
    boost::asio::async_read_until(socket_, rx_sb_, "\n",
                                  boost::bind(&tcpClient::handle_receive, shared_from_this(), boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
  }

  void handle_receive(const boost::system::error_code& ec, std::size_t sz) {
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    std::cout << "received size: " << sz << std::endl;
    start_receive();
  }

private:
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::streambuf rx_sb_;
};

TEST(asioDemo, tcpClient) {
  try {
    boost::asio::io_context io_context;
    auto client = std::make_shared<tcpClient>(io_context, "localhost", "8888");
    client->start_receive();
    io_context.run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

#if 0
int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;
    auto client = std::make_shared<tcpClient>(io_context, argv[1], argv[2]);
    client->start_receive();
    io_context.run();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
#endif