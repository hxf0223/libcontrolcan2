#include "session_pool.h"

using std::weak_ptr;

SessionPool::SessionPool() = default;

void SessionPool::addToPool(const std::shared_ptr<Session> session) { // NOLINT
  const std::weak_ptr<Session> wk_session = session;
  pool_.push_back(session);
}

std::shared_ptr<Session> SessionPool::getSession(int session) {
  const weak_ptr<Session> wk_session = pool_[session];
  return wk_session.lock();
}

int SessionPool::getSize() {
  return (int)(pool_.size());
}
