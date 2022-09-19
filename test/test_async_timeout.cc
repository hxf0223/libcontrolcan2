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

class blocking_reader {
  boost::asio::serial_port &port;
  size_t timeout;
  char c;
  boost::asio::deadline_timer timer;
  bool read_error;

  // Called when an async read completes or has been cancelled
  void read_complete(const boost::system::error_code &error,
                     size_t bytes_transferred) {

    read_error = (error || bytes_transferred == 0);

    // Read has finished, so cancel the
    // timer.
    timer.cancel();
  }

  // Called when the timer's deadline expires.
  void time_out(const boost::system::error_code &error) {

    // Was the timeout was cancelled?
    if (error) {
      // yes
      return;
    }

    // no, we have timed out, so kill
    // the read operation
    // The read callback will be called
    // with an error
    port.cancel();
  }

public:
  // Constructs a blocking reader, pass in an open serial_port and
  // a timeout in milliseconds.
  blocking_reader(boost::asio::serial_port &port, size_t timeout)
      : port(port), timeout(timeout),
        timer(static_cast<boost::asio::io_context &>(
            port.get_executor().context())),
        read_error(true) {}

  // Reads a character or times out
  // returns false if the read times out
  bool read_char(char &val) {
    val = c = '\0';

    // After a timeout & cancel it seems we need
    // to do a restart for subsequent reads to work.
    (static_cast<boost::asio::io_context &>(port.get_executor().context()))
        .restart();

    // Asynchronously read 1 character.
    boost::asio::async_read(
        port, boost::asio::buffer(&c, 1),
        boost::bind(&blocking_reader::read_complete, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));

    // Setup a deadline time to implement our timeout.
    timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    timer.async_wait(boost::bind(&blocking_reader::time_out, this,
                                 boost::asio::placeholders::error));

    // This will block until a character is read
    // or until the it is cancelled.
    (static_cast<boost::asio::io_context &>(port.get_executor().context()))
        .run();

    if (!read_error)
      val = c;

    return !read_error;
  }
};