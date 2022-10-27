#include <boost/asio.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

// NOLINTBEGIN

class Session {
  Session(io::io_context& ioContext)
      : socket_(ioContext)
      , read_(ioContext)
      , write_(ioContext) {}

  void asyncRead() {
    io::async_read(socket_, read_buffer_, io::bind_executor(read_, [&](error_code error, std::size_t /*bytes_transferred*/) {
                     if (!error) {
                       asyncRead();
                     }
                   }));
  }

  void asyncWrite() {
    io::async_read(socket_, write_buffer_, io::bind_executor(write_, [&](error_code error, std::size_t /*bytes_transferred*/) {
                     if (!error) {
                       asyncWrite();
                     }
                   }));
  }

private:
  tcp::socket socket_;
  io::io_context::strand read_;
  io::io_context::strand write_;

  boost::asio::streambuf read_buffer_;
  boost::asio::streambuf write_buffer_;
};

int main2(int /*argc*/, char* /*argv*/[]) {
  io::io_context io_context;
  std::vector<std::thread> threads;
  auto count = std::thread::hardware_concurrency() * 2;

  threads.reserve(count);
  for (int i = 0; i < count; i++) {
    threads.emplace_back([&] {
      io_context.run();
    });
  }

  for (auto& thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  return 0;
}

// NOLINTEND