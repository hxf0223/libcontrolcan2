#include "glog/logging.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  FLAGS_alsologtostderr = 1;
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  int ret = RUN_ALL_TESTS();
  return ret;
}