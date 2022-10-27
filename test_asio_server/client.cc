#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <tuple>

#include "format.hpp"

using std::cout;
using std::endl;
using std::string;
using namespace boost::asio;

void sendMsg(const string& msg, boost::asio::ip::tcp::socket& socket) {
  boost::system::error_code error;
  boost::asio::write(socket, buffer(msg), error);
  if (error) {
    cout << "send failed: " << error.message() << endl;
  }
}

void receiveMsg(boost::asio::ip::tcp::socket& socket) {
  boost::system::error_code error;
  streambuf receive_buffer;
  // read(socket, receive_buffer, transfer_all(), error);
  boost::asio::read_until(socket, receive_buffer, "\n");
  if (error && error != error::eof) {
    cout << "receive failed: " << error.message() << endl;
  } else {
    const char* data = buffer_cast<const char*>(receive_buffer.data());
    cout << "receive len " << strlen(data) << ": " << data;
  }
}

int main(int /*argc*/, char* /*argv*/[]) {
  string host_ip_addr = "127.0.0.1";
  constexpr int kHostPort = 9999;

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket(io_service);
  socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host_ip_addr), kHostPort));

#if 0
size_t msg_id = 0;
  while (1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string msg = util::Format("client message {0}", msg_id++);
    send_msg(msg, socket);
  }
#else
  while (true) {
    receiveMsg(socket);
  }
#endif

  return EXIT_SUCCESS;
}