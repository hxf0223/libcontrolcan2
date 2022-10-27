#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <thread>

#include "server.h"

using boost::asio::ip::tcp;

// Server constructor constructs private acceptor_ variable and gives it
// io_context object along with endpoint information Without acceptor_, we would
// not be able to accept and create new sessions with clients
Server::Server(short port, boost::asio::io_context& ioContext, eventpp_queue_t& ppqs)
    : io_context_(ioContext)
    , acceptor_(ioContext, tcp::endpoint(tcp::v4(), port))
    , eventpp_queue_(ppqs) {
  session_pool_ = std::make_shared<SessionPool>();
  // thd_io_ctx_ = std::thread([this]() { io_context_.run(); });

  std::cout << "STARTING TCP SERVER" << std::endl;
  std::cout << "Listening on port ::" << port << std::endl;
}

Server::~Server() { // NOLINT
  // io_context_.stop();
  // thd_io_ctx_.join();
}

// Called when server is ready to start accepting clients
void Server::startAccepting() {
  session_ = std::make_shared<Session>(io_context_, std::ref(eventpp_queue_));

  for (size_t i = 0; i < session_pool_->getSize(); i++) {
    std::shared_ptr<Session> selected_session = session_pool_->getSession(i);
  }

  // async_accept is blocking and the app will not progress unless a client
  // attempts to connect
  acceptor_.async_accept(session_->getSocket(), boost::bind(&Server::handleAccept, this, boost::asio::placeholders::error)); // NOLINT
}

// If client attempts to connect, handle acception here
// Add current session to pool, start session, and start accepting new client
void Server::handleAccept(const boost::system::error_code& ec) {
  if (!ec) {
    std::cout << "New session created." << std::endl;
    session_pool_->addToPool(session_);
    session_->start();
    startAccepting();
  } else {
    std::cout << ec.message() << std::endl;
  }
}