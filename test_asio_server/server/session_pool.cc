#include "session_pool.h"

SessionPool::SessionPool() {}

void SessionPool::add_to_pool(std::shared_ptr<Session> session) {
  std::weak_ptr<Session> wk_session = session;
  pool_.push_back(session);
}

std::shared_ptr<Session> SessionPool::get_session(int session) {
  std::weak_ptr<Session> wk_session = pool_[session];
  return wk_session.lock();
}

int SessionPool::get_size() {
  return pool_.size();
}
