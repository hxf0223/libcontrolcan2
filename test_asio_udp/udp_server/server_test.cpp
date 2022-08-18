#include "UDPServer.h"
#include <atomic>
#include <chrono>
#include <signal.h>
#include <thread>

std::atomic_bool run_flag{true};
static void signal_handler(int sig) {
  if (sig == SIGINT) {
    run_flag = false;
  }
}

int main() {
  signal(SIGINT, signal_handler);

  try {
    UDPServer server("127.0.0.1", "255.255.0.0", 1111);
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << std::endl;
  }

  while (run_flag) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  std::cout << "server exit." << std::endl;
  return 0;
}
