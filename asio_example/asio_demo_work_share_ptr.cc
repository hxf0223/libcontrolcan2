#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

// https://stackoverflow.com/questions/32258890/c-boost-asio-async-send-to-memory-leak
// https://stackoverflow.com/questions/15568100/confused-when-boostasioio-service-run-method-blocks-unblocks

class multicast_sender {
public:
  multicast_sender(const std::string &address,
                   const std::string &multicast_address,
                   const unsigned short multicast_port)
      : work_(io_service_),
        multicast_endpoint_(
            boost::asio::ip::address::from_string(multicast_address),
            multicast_port),
        socket_(io_service_, boost::asio::ip::udp::endpoint(
                                 boost::asio::ip::address::from_string(address),
                                 0 /* any port */)) {
    // Start running the io_service.  The work_ object will keep
    // io_service::run() from returning even if there is no real work
    // queued into the io_service.
    auto self = this;
    work_thread_ = std::thread([self]() { self->io_service_.run(); });
  }

  ~multicast_sender() {
    // Explicitly stop the io_service.  Queued handlers will not be ran.
    io_service_.stop();

    // Synchronize with the work thread.
    work_thread_.join();
  }

  void send(const char *data, const int size) {
    // Caller may delete before the async operation finishes, so copy the
    // buffer and associate it to the completion handler's lifetime.  Note
    // that the completion may not run in the event the io_servie is
    // destroyed, but the the completion handler will be, so managing via
    // a RAII object (std::shared_ptr) is ideal.
    auto buffer = std::make_shared<std::string>(data, size);
    socket_.async_send_to(boost::asio::buffer(*buffer), multicast_endpoint_,
                          [buffer](const boost::system::error_code &error,
                                   std::size_t bytes_transferred) {
                            std::cout << "Wrote " << bytes_transferred
                                      << " bytes with " << error.message()
                                      << std::endl;
                          });
  }

private:
  boost::asio::io_service io_service_;
  boost::asio::io_service::work work_;
  boost::asio::ip::udp::endpoint multicast_endpoint_;
  boost::asio::ip::udp::socket socket_;
  std::thread work_thread_;
};

TEST(asioDemo, asio_worker_and_share_ptr) {
  constexpr size_t SIZE = 8192;
  multicast_sender sender("127.0.0.1", "239.255.0.0", 30000);
  char *data = (char *)malloc(SIZE);
  std::memset(data, 0, SIZE);
  sender.send(data, SIZE);
  free(data);

  // Give some time to allow for the async operation to complete
  // before shutting down the io_service.
  std::this_thread::sleep_for(std::chrono::seconds(2));
}