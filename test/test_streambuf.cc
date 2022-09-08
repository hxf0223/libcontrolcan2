#include <boost/asio.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

// https://stackoverflow.com/questions/31960010/boost-asio-streambuf

/**
 * @brief read from user and write to socket
 *
 */
TEST(streambuf, write) {
  std::string host_ip_addr = "127.0.0.1";
  constexpr int host_port = 9999;

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket(io_service);
  socket.connect(boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address::from_string(host_ip_addr), host_port));

  boost::asio::streambuf b;

  // prepare() 512 bytes for the output sequence.  The input sequence
  // is empty.
  auto bufs = b.prepare(512);

  // Read from the socket, writing into the output sequence.  The
  // input sequence is empty and the output sequence contains "hello".
  size_t n = socket.receive(bufs);

  // Remove 'n' (5) bytes from output sequence appending them to the
  // input sequence.  The input sequence contains "hello" and the
  // output sequence has 507 bytes.
  b.commit(n);

  // The input and output sequence remain unchanged.
  std::istream is(&b);
  std::string s;

  // Read from the input sequence and consume the read data.  The string
  // 's' contains "hello".  The input sequence is empty, the output
  // sequence remains unchanged.
  is >> s;
}

/**
 * @brief read from socket and read from streambuf
 *
 */
TEST(streambuf, read) {
  std::string host_ip_addr = "127.0.0.1";
  constexpr int host_port = 9999;

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket(io_service);
  socket.connect(boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address::from_string(host_ip_addr), host_port));

  boost::asio::streambuf b;

  // prepare() 512 bytes for the output sequence.  The input sequence
  // is empty.
  auto bufs = b.prepare(512);

  // Read from the socket, writing into the output sequence.  The
  // input sequence is empty and the output sequence contains "hello".
  size_t n = socket.receive(bufs);

  // Remove 'n' (5) bytes from output sequence appending them to the
  // input sequence.  The input sequence contains "hello" and the
  // output sequence has 507 bytes.
  b.commit(n);

  // The input and output sequence remain unchanged.
  std::istream is(&b);
  std::string s;

  // Read from the input sequence and consume the read data.  The string
  // 's' contains "hello".  The input sequence is empty, the output
  // sequence remains unchanged.
  is >> s;
}
