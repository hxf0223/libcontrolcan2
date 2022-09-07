
#include "glog/logging.h"
#include "gtest/gtest.h"

#include "git.h"

// https://github.com/andrew-hardin/cmake-git-version-tracking
// https://blog.csdn.net/qq295109601/article/details/118063009

TEST(gitVersion, func) {
  if (!git::IsPopulated()) {
    LOG(WARNING) << "failed to get the current git state.";
    return;
  }

  if (git::AnyUncommittedChanges()) {
    LOG(WARNING) << "there were uncommitted changes at build-time.";
  }

  LOG(INFO) << "commit " << git::CommitSHA1() << " (" << git::Branch() << ")\n"
            << "describe " << git::Describe() << "\n"
            << "Author: " << git::AuthorName() << " <" << git::AuthorEmail()
            << ">\n"
            << "Date: " << git::CommitDate() << "\n\n"
            << git::CommitSubject() << "\n"
            << git::CommitBody();
}

int main(int argc, char **argv) {
  FLAGS_alsologtostderr = 1;
  FLAGS_colorlogtostderr = true;
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  int ret = RUN_ALL_TESTS();
  return ret;
}
