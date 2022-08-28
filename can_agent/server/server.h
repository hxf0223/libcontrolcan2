#pragma once

#include <boost/asio.hpp>
#include <thread>

#include "canobj_queue_type.h"
#include "session.h"
#include "session_pool.h"

typedef std::shared_ptr<Session> session_ptr;

class Server {
private:
  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::shared_ptr<Session> session_;
  std::shared_ptr<SessionPool> session_pool_;
  std::thread thd_io_ctx_;

  eventpp_queue_t &eventpp_queue_;

public:
  Server(short port, eventpp_queue_t &ppq);
  ~Server();
  void startAccepting();

  void handle_accept(const boost::system::error_code &ec);
  void handle_shutdown();
};
