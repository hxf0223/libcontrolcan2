#include <chrono>
#include <cstddef>
#include <string>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "eventpp/eventqueue.h"
#include "eventpp/utilities/orderedqueuelist.h"

TEST(evnetQueue, Function) {
  auto f1 = [](const std::string &str) { LOG(INFO) << "function1, value: " << str; };
  auto f2 = [](const std::string &str) { LOG(INFO) << "function2, value: " << str; };
  struct functor1 {
    size_t count_{0};
    void operator()(const std::string &str) { LOG(INFO) << "functor 1, count " << ++count_ << ", value: " << str; }
  };

  functor1 ft1;
  using queue_t = eventpp::EventQueue<int, void(const std::string &)>;
  using queue_handle_t = queue_t::Handle;
  queue_t q;

  queue_handle_t h1 = q.appendListener(1, f1);
  q.appendListener(1, f2);
  q.appendListener(1, ft1);

  q.enqueue(1, "test string 1");
  q.enqueue(1, "test string 2");
  q.process();

  LOG(INFO) << "!!! remove f1";
  q.removeListener(1, h1);
  q.enqueue(1, "test string 3");
  q.process();
}

TEST(evnetQueue, multiThread) {
  using queue_t = eventpp::EventQueue<int, void(const std::string &)>;
  using queue_handle_t = queue_t::Handle;
  queue_t q;

  auto f1 = [](const std::string &str) {
    LOG(INFO) << "thread id: " << std::this_thread::get_id() << ". value: " << str;
  };

  auto queue_func = [](queue_t &q) {
    for (size_t i = 0; i < 100; i++) {
      std::this_thread::sleep_for(std::chrono::microseconds(10));
      LOG(INFO) << "queue thread id: " << std::this_thread::get_id();
      q.enqueue(1, "test string " + std::to_string(i));
      q.process();
    }
  };

  q.appendListener(1, f1);
  std::thread queue_thd(queue_func, std::ref(q));
  queue_thd.join();
}

int main(int argc, char **argv) {
  FLAGS_alsologtostderr = 1;
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  int ret = RUN_ALL_TESTS();
  return ret;
}