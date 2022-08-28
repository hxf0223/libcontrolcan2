#include <boost/asio.hpp>
#include <thread>

#include "server.h"

using boost::asio::ip::tcp;

// Server constructor constructs private acceptor_ variable and gives it io_context object along with endpoint
// information Without acceptor_, we would not be able to accept and create new sessions with clients
Server::Server(short port, eventpp_queue_t &ppqs)
  : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)), eventpp_queue_(ppqs) {
  session_pool_ = std::make_shared<SessionPool>();
  thd_io_ctx_ = std::thread([this]() { io_context_.run(); });

  std::cout << "STARTING TCP SERVER" << std::endl;
  std::cout << "Listening on port ::" << port << std::endl;
}

Server::~Server() {
  io_context_.stop();
  thd_io_ctx_.join();
}

// Called when server is ready to start accepting clients
void Server::startAccepting() {
  session_ = std::make_shared<Session>(io_context_, eventpp_queue_);

  for (int i = 0; i < session_pool_->get_size(); ++i) {
    std::shared_ptr<Session> selected_session_ = session_pool_->get_session(i);
  }

  // async_accept is blocking and the app will not progress unless a client attempts to connect
  acceptor_.async_accept(session_->get_socket(),
                         boost::bind(&Server::handle_accept, this, boost::asio::placeholders::error));
}

// If client attempts to connect, handle acception here
// Add current session to pool, start session, and start accepting new client
void Server::handle_accept(const boost::system::error_code &ec) {
  if (!ec) {
    std::cout << "New session created" << std::endl;
    session_pool_->add_to_pool(session_);
    session_->start();
    startAccepting();
  } else {
    std::cout << ec.message() << std::endl;
  }
}