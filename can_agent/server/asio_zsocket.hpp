#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <zmq.h>

namespace asio_zmq {

class zmq_exception : public std::exception {
public:
  zmq_exception(std::string __reason) : reason_(__reason) {}
  const char *what() const noexcept { return reason_.c_str(); }
  std::string reason_;
};

class zmqContext {
public:
  typedef void *zmq_context_handle;
  zmqContext() {
    void *tmp = zmq_ctx_new();
    if (tmp == NULL) throw zmq_exception("could not obtain a zmq context");
    handle_ = tmp;
  }
  zmq_context_handle handle() { return handle_; }

private:
  zmq_context_handle handle_;
};

static inline void *get_zmq_context() {
  static zmqContext zmq_ctx_;
  return zmq_ctx_.handle();
}

template <typename ConstBufferSequence, typename Handler>
class zsock_send_op {
public:
  zsock_send_op(const void *zmqSock, const ConstBufferSequence &Buffers, Handler handler)
    : buffers_(Buffers), handler_(handler), zmq_sock_(zmqSock) {}

  void operator()(const boost::system::error_code &ec, std::size_t bytes_transferred) {
    int send_rc = 0, flags = 0;
    boost::system::error_code zec;
    size_t fsize = sizeof(flags);
    if (ec) {
      handler_(ec, send_rc);
      return;
    }

    zmq_getsockopt(const_cast<void *>(zmq_sock_), ZMQ_EVENTS, &flags, &fsize);
    if (flags & ZMQ_POLLOUT) {
      const void *buf = boost::asio::buffer_cast<const void *>(buffers_);
      int buf_size = boost::asio::buffer_size(buffers_);
      send_rc = zmq_send(const_cast<void *>(zmq_sock_), const_cast<void *>(buf), buf_size, 0);
      if (send_rc == -1) {
        zec.assign(errno, boost::system::system_category());
      }
    }
    handler_(zec, send_rc);
  }

private:
  ConstBufferSequence buffers_;
  Handler handler_;
  const void *zmq_sock_;
};

template <typename MutableBufferSequence, typename Handler>
class zsock_recv_op {
public:
  zsock_recv_op(void *__zmq_sock, const MutableBufferSequence &__buffers, Handler __handler)
    : buffers_(__buffers), handler_(__handler), zmq_sock_(__zmq_sock) {}

  void operator()(const boost::system::error_code &ec, std::size_t bytes_transferred) {
    if (ec) {
      // something wrong happened with
      // the underlying socket report the error
      handler_(ec, bytes_transferred);
      return;
    }

    boost::system::error_code zec; // defaut ctor no error
    int recvd = 0, flags = 0;
    size_t fsize = sizeof(flags);
    zmq_getsockopt(const_cast<void *>(zmq_sock_), ZMQ_EVENTS, &flags, &fsize);
    if (flags & ZMQ_POLLIN) { // we can read from the zmq socket
      void *buf = boost::asio::buffer_cast<void *>(buffers_);
      int buf_size = boost::asio::buffer_size(buffers_);
      recvd = zmq_recv(zmq_sock_, buf, buf_size, 0);
      if (recvd == -1) zec.assign(errno, boost::system::system_category());
    }
    // if we were not able to read it is not an error the application should try
    // again later
    handler_(zec, recvd);
  }

private:
  MutableBufferSequence buffers_;
  Handler handler_;
  void *zmq_sock_;
};

class asioZmqReqSocket {
public:
  /**
   * @brief Construct a new asio Zmq Req Socket object
   *
   * @param ioService the boost asio io_context object.
   * @param zmqCtxHandle handle of zmqContext or zmq::context_t
   * @param srvAddr zmq server address
   */
  asioZmqReqSocket(boost::asio::io_context &ioService, void *zmqCtxHandle, std::string srvAddr) : sock_(ioService) {
    zmq_sock_ = zmq_socket(zmqCtxHandle, ZMQ_SUB); // ZMQ_REQ

    int opt = 0;
    std::size_t optlen = sizeof(opt);
    zmq_setsockopt(zmq_sock_, ZMQ_LINGER, &opt, optlen);
    if (!zmq_sock_) throw zmq_exception("could not create a zmq REQ socket");

    int timeout = 100, rc;
    rc = zmq_setsockopt(zmq_sock_, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    rc = zmq_setsockopt(zmq_sock_, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    rc = zmq_setsockopt(zmq_sock_, ZMQ_SUBSCRIBE, "", 0);

    int zfd;
    optlen = sizeof(zfd);
    zmq_getsockopt(zmq_sock_, ZMQ_FD, &zfd, &optlen);
    sock_.assign(boost::asio::ip::tcp::v4(), zfd);

    rc = zmq_connect(zmq_sock_, srvAddr.c_str());
    if (rc) throw zmq_exception("zmq socket: could not connect to " + srvAddr);
  }

  template <typename ConstBufferSequence, typename Handler>
  void async_send(const ConstBufferSequence &buffer, Handler handler) {
    zsock_send_op<ConstBufferSequence, Handler> send_op(zmq_sock_, buffer, handler);
    sock_.async_write_some(boost::asio::null_buffers(), send_op);
  }

  template <typename MutableBufferSequence, typename Handler>
  void async_recv(const MutableBufferSequence &buffer, Handler handler) {
    zsock_recv_op<MutableBufferSequence, Handler> recv_op(zmq_sock_, buffer, handler);
    sock_.async_read_some(boost::asio::null_buffers(), recv_op);
  }

  ~asioZmqReqSocket() {
    sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    sock_.close();
    zmq_close(zmq_sock_);
  }

private:
  void *zmq_sock_ = nullptr;
  boost::asio::ip::tcp::socket sock_;
};

} // namespace asio_zmq

class my_zmq_req_client : public std::enable_shared_from_this<my_zmq_req_client> {
public:
  my_zmq_req_client(boost::asio::io_service &__io_service, const std::string &__addr)
    : zsock_(__io_service, asio_zmq::get_zmq_context(), __addr) {}

  void send(const std::string &__message) {
    // print("my_zmq_req_client: sending message:", __message);
    zsock_.async_send(boost::asio::buffer(__message),
                      boost::bind(&my_zmq_req_client::handle_send, shared_from_this(), boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred));
  }

private:
  void handle_send(const boost::system::error_code &ec, std::size_t bytes_transferred) {
    if (ec) {
      std::cout << "failed to send buffer\n";
    } else {
      // print("my_zmq_req_client: message sent");
      zsock_.async_recv(boost::asio::buffer(recv_buffer_),
                        boost::bind(&my_zmq_req_client::handle_recv, shared_from_this(),
                                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
  }

  void handle_recv(const boost::system::error_code &ec, std::size_t bytes_transferred) {

    if (!ec) {
      // if no error happened but no bytes were received because the zeromq socket
      // is not POLL_IN ready (check the zero_mq zmq_getsockopt: it says that
      // a socket can be reported as writable by the OS but not yet for zeromq)
      // schedule another async recv operation. This is less likely to happen for send operations
      // and in practice never had to do it.
      if (bytes_transferred == 0)
        zsock_.async_recv(boost::asio::buffer(recv_buffer_),
                          boost::bind(&my_zmq_req_client::handle_recv, shared_from_this(),
                                      boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
      else {
        printf("my_zmq_req_client: zmq REP: %s\n", recv_buffer_.data());
      }
    }
  }

  asio_zmq::asioZmqReqSocket zsock_;
  std::array<char, 256> recv_buffer_;
};