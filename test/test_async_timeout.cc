//
// blocking_reader.h - a class that provides basic support for
// blocking & time-outable single character reads from
// boost::asio::serial_port.
//
// use like this:
//
//  blocking_reader reader(port, 500);
//
//  char c;
//
//  if (!reader.read_char(c))
//      return false;
//
// Kevin Godden, www.ridgesolutions.ie
//

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>

class BlockingReader {
  boost::asio::serial_port &port_;
  size_t timeout_;
  char ch_;
  boost::asio::deadline_timer timer_;
  bool read_error_{};

  // Called when an async read completes or has been cancelled
  void readComplete(const boost::system::error_code &error,
                    size_t bytes_transferred) {

    read_error_ = (error || bytes_transferred == 0);

    // Read has finished, so cancel the
    // timer.
    timer_.cancel();
  }

  // Called when the timer's deadline expires.
  void timeOut(const boost::system::error_code &error) {

    // Was the timeout was cancelled?
    if (error) {
      // yes
      return;
    }

    // no, we have timed out, so kill
    // the read operation
    // The read callback will be called
    // with an error
    port_.cancel();
  }

public:
  // Constructs a blocking reader, pass in an open serial_port and
  // a timeout in milliseconds.
  BlockingReader(boost::asio::serial_port &port, size_t timeout)
      : port_(port), timeout_(timeout),
        timer_(static_cast<boost::asio::io_context &>(
            port.get_executor().context())) {}

  // Reads a character or times out
  // returns false if the read times out
  bool readChar(char &val) {
    val = ch_ = '\0';

    // After a timeout & cancel it seems we need
    // to do a restart for subsequent reads to work.
    (static_cast<boost::asio::io_context &>(port_.get_executor().context()))
        .restart();

    // Asynchronously read 1 character.
    boost::asio::async_read(
        port_, boost::asio::buffer(&ch_, 1),
        boost::bind(&BlockingReader::readComplete, this, // NOLINT
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));

    // Setup a deadline time to implement our timeout.
    timer_.expires_from_now(boost::posix_time::milliseconds(timeout_));
    timer_.async_wait(boost::bind(&BlockingReader::timeOut, this, // NOLINT
                                  boost::asio::placeholders::error));

    // This will block until a character is read
    // or until the it is cancelled.
    (static_cast<boost::asio::io_context &>(port_.get_executor().context()))
        .run();

    if (!read_error_) {
      val = ch_;
    }

    return !read_error_;
  }
};