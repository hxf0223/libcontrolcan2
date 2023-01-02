#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& opt) {
  return opt ? os << opt.value() : os;
}

} // namespace

static std::atomic_bool run_flag{true};
static void signalHandler(int /*signal*/) {
  run_flag.store(false);
}

TEST(zmq, server) {
  zmq::context_t ctx;
  signal(SIGINT, signalHandler);

  auto pub_func = [](std::atomic_bool* runFlag, zmq::context_t* ctx) -> void {
    zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
    publisher.bind("inproc://#1");

    // Give the subscribers a chance to connect, so they don't lose any messages
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int32_t msg_id{0};
    char send_buff[256];
    while (runFlag->load()) {
      sprintf(send_buff, "publish message %d", msg_id++);
      const auto size = strlen(send_buff);
      publisher.send(zmq::buffer(std::string_view(send_buff, size)));
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  };

  auto sub_func = [](std::atomic_bool* runFlag, zmq::context_t* ctx) -> void {
    zmq::socket_t ctx_sub(*ctx, ZMQ_SUB);
    ctx_sub.connect("inproc://#1");
    ctx_sub.set(zmq::sockopt::subscribe, ""); // subscribe all message types
    ctx_sub.set(zmq::sockopt::rcvtimeo, 10);  // receive timeout in ms

    while (runFlag->load()) {
      std::vector<zmq::message_t> recv_msgs;
      zmq::recv_result_t const result = zmq::recv_multipart(ctx_sub, std::back_inserter(recv_msgs));
      // if (!result.has_value()) continue;
      for (auto& msg : recv_msgs) {
        std::string const rxstr(static_cast<const char*>(msg.data()), msg.size());
        std::cout << "receive message: " << rxstr << std::endl;
      }
    }
  };

  std::thread pub_thd(pub_func, &run_flag, &ctx);
  std::thread sub_thd(sub_func, &run_flag, &ctx);
  pub_thd.join();
  sub_thd.join();
}