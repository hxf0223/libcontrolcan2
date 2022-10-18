#include "glog/logging.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = true;
  FLAGS_colorlogtostderr = true;
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  int const ret = RUN_ALL_TESTS();
  return ret;
}