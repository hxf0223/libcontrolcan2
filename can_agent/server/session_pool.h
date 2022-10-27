#pragma once
#include <cstddef>
#include <iostream>
#include <vector>

#include "session.h"

using session_pool = std::vector<std::weak_ptr<Session>>;

class SessionPool {
private:
  session_pool pool_;

public:
  SessionPool() = default;
  SessionPool(const SessionPool&) = default;
  SessionPool(SessionPool&&) = default;
  SessionPool& operator=(const SessionPool&) = default;
  SessionPool& operator=(SessionPool&&) = default;
  explicit SessionPool(session_pool pool)
      : pool_(std::move(pool)) {}
  void addToPool(const std::shared_ptr<Session> session);
  std::shared_ptr<Session> getSession(size_t session);
  size_t getSize();
};