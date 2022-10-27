#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

namespace util {

class ArgBase {
public:
  ArgBase() {}
  virtual ~ArgBase() {}
  virtual void format(std::ostringstream& ss, const std::string& fmt) = 0;
};

template <class T>
class Arg : public ArgBase {
public:
  Arg(T arg)
      : m_arg_(arg) {}
  ~Arg() override {}
  void format(std::ostringstream& ss, const std::string& fmt) override {
    ss << m_arg_;
  }

private:
  T m_arg_;
};

class ArgArray : public std::vector<ArgBase*> {
public:
  ArgArray() {}
  ~ArgArray() {
    std::for_each(begin(), end(), [](ArgBase* p) {
      delete p;
    });
  }
};

static void formatItem(std::ostringstream& ss, const std::string& item, const ArgArray& args) {
  std::string fmt;

  char* endptr = nullptr;
  auto index = strtol(item.data(), &endptr, 10);
  if (index < 0 || index >= args.size()) {
    return;
  }

  if (*endptr == ',') {
    const auto alignment = (int)strtol(endptr + 1, &endptr, 10);
    if (alignment > 0) {
      ss << std::right << std::setw(alignment);
    } else if (alignment < 0) {
      ss << std::left << std::setw(-alignment);
    }
  }

  if (*endptr == ':') {
    fmt = endptr + 1;
  }

  args[index]->format(ss, fmt);
}

template <class T>
static void transfer(ArgArray& argArray, T t) {
  argArray.push_back(new Arg<T>(t));
}

template <class T, typename... Args>
static void transfer(ArgArray& argArray, T t, Args&&... args) {
  Transfer(argArray, t);
  Transfer(argArray, args...);
}

template <typename... Args>
std::string format(const std::string& format, Args&&... args) {
  if (sizeof...(args) == 0) {
    return format;
  }

  ArgArray arg_array;
  Transfer(arg_array, args...);
  size_t start = 0;
  size_t pos = 0;
  std::ostringstream ss;
  while (true) {
    pos = format.find('{', start);
    if (pos == std::string::npos) {
      ss << format.substr(start);
      break;
    }

    ss << format.substr(start, pos - start);
    if (format[pos + 1] == '{') {
      ss << '{';
      start = pos + 2;
      continue;
    }

    start = pos + 1;
    pos = format.find('}', start);
    if (pos == std::string::npos) {
      ss << format.substr(start - 1);
      break;
    }

    formatItem(ss, format.substr(start, pos - start), arg_array);
    start = pos + 1;
  }

  return ss.str();
}

} // namespace util