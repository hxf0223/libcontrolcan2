#include "session_pool.h"

void SessionPool::addToPool(std::shared_ptr<Session> session) {
  std::weak_ptr<Session> wk_session = session;
  pool_.push_back(session);
}

std::shared_ptr<Session> SessionPool::getSession(size_t session) {
  std::weak_ptr<Session> wk_session = pool_[session];
  return wk_session.lock();
}

size_t SessionPool::getSize() {
  return pool_.size();
}
