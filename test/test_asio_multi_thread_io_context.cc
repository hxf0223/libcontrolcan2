#include <boost/asio.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

class session {
  session(io::io_context &io_context)
      : socket(io_context), read(io_context), write(io_context) {}

  void async_read() {
    io::async_read(socket, read_buffer,
                   io::bind_executor(read, [&](error_code error,
                                               std::size_t bytes_transferred) {
                     if (!error) {
                       async_read();
                     }
                   }));
  }

  void async_write() {
    io::async_read(socket, write_buffer,
                   io::bind_executor(write, [&](error_code error,
                                                std::size_t bytes_transferred) {
                     if (!error) {
                       async_write();
                     }
                   }));
  }

private:
  tcp::socket socket;
  io::io_context::strand read;
  io::io_context::strand write;

  boost::asio::streambuf read_buffer;
  boost::asio::streambuf write_buffer;
};

int main2(int argc, char *argv[]) {
  io::io_context io_context;
  std::vector<std::thread> threads;
  auto count = std::thread::hardware_concurrency() * 2;

  for (int n = 0; n < count; ++n) {
    threads.emplace_back([&] { io_context.run(); });
  }

  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  return 0;
}