//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"

class session : public std::enable_shared_from_this<session> {
public:
  ~session() { std::cout << "session closed." << std::endl; }
  session(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}
  void start() { do_write(0); }

private:
  void do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            [this, self](boost::system::error_code ec, std::size_t length) {
                              if (!ec) {
                                do_write(length);
                              }
                            });
  }

  void do_write(std::size_t length) {
    auto self(shared_from_this());
    // std::cout << "do_write thread id: " << std::this_thread::get_id() << std::endl;
    auto message = std::make_shared<std::string>("hello world from server.\n");
    boost::asio::async_write(socket_, boost::asio::buffer(*message),
                             [self](boost::system::error_code ec, std::size_t /*length*/) {
                               if (!ec) {
                                 std::cout << "write a message count " << ++self->tx_count_ << std::endl;
                                 // std::cout << "handler id: " << std::this_thread::get_id() << std::endl;
                                 std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                 self->do_write(0);
                               }
                             });
  }

private:
  size_t tx_count_{0};
  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server {
public:
  server(boost::asio::io_context &io_context, short port)
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
      if (!ec) {
        std::cout << "accept a client." << std::endl;
        std::make_shared<session>(std::move(socket))->start();
      }

      do_accept();
    });
  }

  boost::asio::ip::tcp::acceptor acceptor_;
};

TEST(asioDemo, asyncTcpServer) {
  try {
    boost::asio::io_context io_context;
    server s(io_context, 8888);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

#if 0
int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;
    server s(io_context, std::atoi(argv[1]));
    io_context.run();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
#endif