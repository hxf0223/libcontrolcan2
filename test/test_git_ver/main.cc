
#include "glog/logging.h"
#include "gtest/gtest.h"

// https://github.com/andrew-hardin/cmake-git-version-tracking
// https://blog.csdn.net/qq295109601/article/details/118063009

int main(int argc, char **argv) {
  FLAGS_alsologtostderr = 1;
  FLAGS_colorlogtostderr = true;
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  int ret = RUN_ALL_TESTS();
  return ret;
}
