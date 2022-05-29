#pragma once

#include <algorithm> // std::for_each
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <sstream> // std::ostringstream
#include <utility>
#include <vector>
//#include <boost/thread/condition_variable.hpp>

namespace can {
namespace utils {

template <typename T>
struct is_array_or_vector {
  enum { value = false };
};

template <typename T, typename A>
struct is_array_or_vector<std::vector<T, A>> {
  enum { value = true };
};

template <typename T, std::size_t N>
struct is_array_or_vector<std::array<T, N>> {
  enum { value = true };
};

template <class T, class = typename std::enable_if<is_array_or_vector<T>::value>::type>
static std::string bin2hex_dump(std::ostringstream &oss, T &bin) {
  const auto separator = ' ';
  oss << std::hex << std::nouppercase;

  std::for_each(bin.begin(), bin.end(), [&oss, &separator](unsigned char ch) {
    oss << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ch) << separator;
  });

  oss << std::dec << std::setfill(' '); // reset stream to original
  return oss.str();
}

template <class T, class = typename std::enable_if<is_array_or_vector<T>::value>::type>
static std::string bin2hex_dump(T &bin) {
  std::ostringstream oss;
  return bin2hex_dump(oss, bin);
}

static std::string bin2hex_dump(std::ostringstream &oss, unsigned char const *pbin, size_t len) {
  const auto separator = ' ';
  oss << std::hex << std::nouppercase;

  for (size_t i = 0; i < len; i++)
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(*(pbin + i)) << separator;

  oss << std::dec << std::setfill(' '); // reset stream to original
  return oss.str();
}

static std::string bin2hex(std::ostringstream &oss, unsigned char const *pbin, size_t len) {
  oss << std::hex << std::nouppercase;
  for (size_t i = 0; i < len; i++)
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(*(pbin + i));

  oss << std::dec << std::setfill(' '); // reset stream to original
  return oss.str();
}

static std::string bin2hex_dump(unsigned char const *pbin, size_t len) {
  std::ostringstream oss;
  return bin2hex_dump(oss, pbin, len);
}

static std::string bin2hex(unsigned char const *pbin, size_t len) {
  std::ostringstream oss;
  return bin2hex(oss, pbin, len);
}

static const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static std::string bin2hex_fast(void *const p, size_t len) {
  std::string str(len * 2, ' ');
  const auto data = static_cast<unsigned char *>(p);
  auto pstr = const_cast<char *>(str.data());

  for (size_t i = 0; i < len; i++) {
    *pstr++ = hexmap[(data[i] & 0xF0) >> 4];
    *pstr++ = hexmap[data[i] & 0x0F];
  }

  return str;
}

static size_t bin2hex_fast(void *const p, size_t len, const char* dst) {
  const auto data = static_cast<unsigned char *>(p);
  auto pstr = const_cast<char *>(dst);
  for (size_t i = 0; i < len; i++) {
    *pstr++ = hexmap[(data[i] & 0xF0) >> 4];
    *pstr++ = hexmap[data[i] & 0x0F];
  }

  return (len*2);
}

static std::vector<unsigned char> hex_string_to_bin(std::string str) {
  // mapping of ASCII characters to hex values
  static uint8_t hashmap[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
    0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
  };

  std::vector<unsigned char> v;
  const auto len = str.length();
  v.reserve(len / 2);

  for (size_t pos = 0; pos < len; pos += 2) {
    const size_t idx0 = (static_cast<uint8_t>(str[pos + 0]) & 0x1F) ^ 0x10;
    const size_t idx1 = (static_cast<uint8_t>(str[pos + 1]) & 0x1F) ^ 0x10;
    v.push_back(static_cast<uint8_t>(hashmap[idx0] << 4) | hashmap[idx1]);
  }

  return v;
}

static std::vector<unsigned char> hex_string_to_bin_fastest(std::string str) {
  // mapping of ASCII characters to hex values
  static uint8_t hashmap[] = {
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // gap before first hex digit
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // 0123456789
    0,  0,  0,  0,  0,  0,  0,                                           // :;<=>?@ (gap)
    10, 11, 12, 13, 14, 15,                                              // ABCDEF
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,                         // GHIJKLMNOPQRS (gap)
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,                         // TUVWXYZ[/]^_` (gap)
    10, 11, 12, 13, 14, 15                                               // abcdef
  };

  std::vector<unsigned char> v;
  const auto len = str.length();
  v.reserve(len / 2);

  for (size_t pos = 0; pos < len; pos += 2) {
    const size_t idx0 = static_cast<uint8_t>(str[pos + 0]);
    const size_t idx1 = static_cast<uint8_t>(str[pos + 1]);
    v.push_back(static_cast<uint8_t>(hashmap[idx0] << 4) | hashmap[idx1]);
  }

  return v;
}

} // namespace utils
} // namespace can
