#pragma once
#include "session.h"
#include "session_pool.h"
#include <boost/asio.hpp>

using session_ptr = std::shared_ptr<Session>;

class Server {
private:
  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::shared_ptr<Session> session_;
  std::shared_ptr<SessionPool> session_pool_;

public:
  Server(boost::asio::io_context& ioContext, short port);
  void startAccepting();
  void handleAccept(const boost::system::error_code& ec);
};
