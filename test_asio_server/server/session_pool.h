#pragma once
#include <iostream>
#include <vector>

#include "session.h"

typedef std::vector<std::weak_ptr<Session>> session_pool;

class SessionPool {
private:
  session_pool pool_;

public:
  SessionPool();

  void add_to_pool(std::shared_ptr<Session> session);

  std::shared_ptr<Session> get_session(int session);

  int get_size();
};