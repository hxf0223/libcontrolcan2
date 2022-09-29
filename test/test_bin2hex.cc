#include "glog/logging.h"
#include "gtest/gtest.h"

#include "hex_dump.hpp"
#include "usbcan.h"

using namespace can::utils;

TEST(CAN, bin2hex_template) {
  char buffer[256]{0};
  const char *pszcstr = ".const string.";
  uint64_t const data64 = 0x0102030405060708;
  uint32_t const data32 = 0x090A0B0C;

  auto size = bin2hex::bin2hex_fast(buffer, "const string literal.\n");
  LOG(INFO) << "bin2hex size: " << size << ", char* literal: " << buffer;

  size = bin2hex::bin2hex_fast(buffer, data64);
  LOG(INFO) << "bin2hex size: " << size << ", data64: " << buffer;

  size = bin2hex::bin2hex_fast(buffer, data64, data32);
  LOG(INFO) << "bin2hex size: " << size << ", data64 + data32: " << buffer;

  size = bin2hex::bin2hex_fast(buffer, pszcstr, data64, data32);
  LOG(INFO) << "bin2hex size: " << size
            << ", const char* + data64 + data32: " << buffer;

  const auto &data64_ref = data64;
  size = bin2hex::bin2hex_fast(buffer, data64_ref);
  LOG(INFO) << "bin2hex size: " << size << ", data64 ref: " << buffer;

  size = bin2hex::bin2hex_fast(buffer, &data64);
  LOG(INFO) << "bin2hex size: " << size << ", data64 address: " << buffer;

  VCI_CAN_OBJ can_obj{};
  size = bin2hex::bin2hex_fast(buffer, can_obj);
  LOG(INFO) << "bin2hex size: " << size << ", can_obj: " << buffer;
  LOG(INFO) << "VCI_CAN_OBJ size: " << sizeof(VCI_CAN_OBJ);

  size = bin2hex::bin2hex_fast(buffer, ".const string.", &can_obj);
  LOG(INFO) << "bin2hex size: " << size
            << ", const char* + can_obj address: " << buffer;
}