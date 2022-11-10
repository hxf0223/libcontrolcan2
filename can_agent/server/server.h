#pragma once

#include <boost/asio.hpp>
#include <thread>

#include "canobj_queue_type.h"
#include "session.h"
#include "session_pool.h"

using session_ptr = std::shared_ptr<Session>;

class Server {
private:
  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::shared_ptr<Session> session_;
  std::shared_ptr<SessionPool> session_pool_;
  std::thread thd_io_ctx_;

  eventpp_queue_t& eventpp_queue_;

public:
  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(const Server&) = delete;
  Server& operator=(Server&&) = delete;
  Server(short port, boost::asio::io_context& ioContext, eventpp_queue_t& ppq);
  ~Server();

  void startAccepting();
  void handleAccept(const boost::system::error_code& ec);
  void handleShutdown();
};
