#pragma once
#include <iostream>
#include <vector>

#include "session.h"

using session_pool = std::vector<std::weak_ptr<Session>>;

class SessionPool {
private:
  session_pool pool_;

public:
  SessionPool();

  void addToPool(std::shared_ptr<Session> session);
  std::shared_ptr<Session> getSession(int session);
  int getSize();
};