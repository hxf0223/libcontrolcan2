#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace {
constexpr short kUdpPort = 9987;
}

using boost::asio::ip::udp;
using namespace std;

class Server {
public:
  Server(boost::asio::io_context& ioContext, short port)
      : socket_(ioContext, udp::endpoint(udp::v4(), port)) {
    socket_.async_receive_from(boost::asio::buffer(data_, kMaxLength), sender_endpoint_,
                               bind(&Server::handleReceive, this, placeholders::_1, placeholders::_2)); // NOLINT
  }

  void handleReceive(const boost::system::error_code& error, size_t bytesRecvd) {
    if (!error && bytesRecvd > 0) {
      printf("Received length: %zu, content: [%s]\n", bytesRecvd, data_);
      socket_.async_send_to(boost::asio::buffer(data_, bytesRecvd), sender_endpoint_,
                            bind(&Server::handleSend, this, placeholders::_1, placeholders::_2)); // NOLINT
    } else {
      socket_.async_receive_from(boost::asio::buffer(data_, kMaxLength), sender_endpoint_,
                                 bind(&Server::handleReceive, this, placeholders::_1, placeholders::_2)); // NOLINT
    }
  }

  void handleSend(const boost::system::error_code& /*error*/, size_t /*bytes_sent*/) {
    socket_.async_receive_from(boost::asio::buffer(data_, kMaxLength), sender_endpoint_,
                               bind(&Server::handleReceive, this, placeholders::_1, placeholders::_2)); // NOLINT
  }

private:
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  enum { kMaxLength = 1024 };
  char data_[kMaxLength];
};

int main(int /*argc*/, char* /*argv*/[]) {
  try {
    boost::asio::io_context io_context;
    Server s(io_context, kUdpPort);
    io_context.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
