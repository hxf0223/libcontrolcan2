#pragma once

#include <algorithm> // std::for_each
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <sstream> // std::ostringstream
#include <type_traits>
#include <utility>
#include <vector>

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

struct bin2hex {
  template <typename T>
  static const T ptr_of(const T &v, std::true_type) {
    return v;
  }

  template <class T>
  static const T *ptr_of(const T &v, std::false_type) {
    return &v;
  }

  static size_t bin2hex_fast(const char *dst) {
    auto ptr_dst = const_cast<char *>(dst);
    *ptr_dst = '\0';
    return 0;
  }

  template <typename... Args>
  static size_t bin2hex_fast(const char *dst, const char *const src, Args... rest) {
    auto ptr_dst = const_cast<char *>(dst);
    size_t src_bytes = ::strlen(src);
    ::memcpy(ptr_dst, src, src_bytes);

    auto rest_size = bin2hex_fast(ptr_dst + src_bytes, rest...);
    return (src_bytes + rest_size);
  }

  template <typename T, typename... Args>
  static size_t bin2hex_fast(const char *dst, const T &head, Args... rest) {
    using rm_ref_t = std::remove_reference_t<T>;
    using decay2_t = std::remove_const_t<std::remove_reference_t<T>>;
    static_assert(std::is_pod<decay2_t>::value, "Not a POD type."); // do NOT use std::decay<T>::type
    uint8_t *psrc = (uint8_t *)ptr_of(head, typename std::is_pointer<rm_ref_t>::type());
    const size_t bytes = sizeof(std::remove_const_t<std::remove_reference_t<std::remove_pointer_t<T>>>);

    auto pdst = const_cast<char *>(dst);
    do_conv(pdst, psrc, bytes);

    auto rest_size = bin2hex_fast(pdst + bytes * 2, rest...);
    return (bytes * 2 + rest_size);
  }

  static std::string bin2hex_fast2(void *const p, size_t len) {
    std::string str(len * 2, ' ');
    const auto psrc = static_cast<unsigned char *>(p);
    do_conv(const_cast<char *>(str.data()), psrc, len);

    return str;
  }

private:
  static void do_conv(char *pdst, const uint8_t *psrc, size_t len) {
    static char hexmap[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (size_t i = 0; i < len; i++) {
      *pdst++ = hexmap[(psrc[i] & 0xF0) >> 4];
      *pdst++ = hexmap[psrc[i] & 0x0F];
    }
  }
}; // struct bin2hex

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

static inline void hex_string_to_bin_fastest(const char *src, size_t srcLen, uint8_t *out) {
  const static uint8_t hashmap[] = {
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // gap before first hex digit
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // 0123456789
    0,  0,  0,  0,  0,  0,  0,                                           // :;<=>?@ (gap)
    10, 11, 12, 13, 14, 15,                                              // ABCDEF
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,                         // GHIJKLMNOPQRS (gap)
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,                         // TUVWXYZ[/]^_` (gap)
    10, 11, 12, 13, 14, 15                                               // abcdef
  };

  for (size_t pos = 0; pos < srcLen; pos += 2) {
    const size_t idx0 = static_cast<uint8_t>(src[pos + 0]);
    const size_t idx1 = static_cast<uint8_t>(src[pos + 1]);
    out[pos >> 1] = (static_cast<uint8_t>(hashmap[idx0] << 4) | hashmap[idx1]);
  }
}

static inline void hex_string_to_bin_fastest(const std::string &str, uint8_t *out) {
  return hex_string_to_bin_fastest(str.data(), str.length(), out);
}

static inline std::vector<unsigned char> hex_string_to_bin_fastest(const std::string &str) {
  std::vector<uint8_t> v(str.length() / 2);
  hex_string_to_bin_fastest(str, v.data());
  return v;
}

} // namespace utils
} // namespace can
