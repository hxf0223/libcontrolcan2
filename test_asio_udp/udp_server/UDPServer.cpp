#include <chrono>
#include <thread>

#include "UDPServer.h"

void UDPServer::startReceive() {
  socket_.async_receive_from(boost::asio::buffer(recvBuffer_), remote_endpoint_,
                             boost::bind(&UDPServer::handleReceive, this, boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
  data_ = std::string(std::begin(recvBuffer_), std::end(recvBuffer_));
  recvBuffer_.fill(0);
  std::cout << data_ << std::endl;
}

void UDPServer::handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred) {
  if (!error || error == boost::asio::error::message_size) {
    auto message = std::make_shared<std::string>("Hello, World! (Server)\n");
    socket_.send_to(boost::asio::buffer(*message), remote_endpoint_);
    startReceive();
  }
}

void UDPServer::start_send() {
  auto message = std::make_shared<std::string>("Hello, World! (Server)\n");
  socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
                        boost::bind(&UDPServer::handle_send, this, boost::asio::placeholders::error));
}

void UDPServer::handle_send(const boost::system::error_code &ec) {
  if (!ec) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    start_send();
  }
}