#include "UDPServer.h"

void UDPServer::startReceive() {
  socket_.async_receive_from(boost::asio::buffer(recvBuffer_), remoteEndpoint_,
                             boost::bind(&UDPServer::handleReceive, this, boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
  data_ = std::string(std::begin(recvBuffer_), std::end(recvBuffer_));
  recvBuffer_.fill(0);
  std::cout << data_ << std::endl;
}

void UDPServer::handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred) {
  if (!error || error == boost::asio::error::message_size) {

    auto message = std::make_shared<std::string>("Hello, World! (Server)\n");

    socket_.send_to(boost::asio::buffer(*message), remoteEndpoint_);
    startReceive();
  }
}