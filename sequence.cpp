#include "sequence.h"
#include <atomic>


namespace {
  std::atomic<xnorseq::object_id> _objectIdCount;
}

namespace xnorseq {
  Object::Object() {
    mID = _objectIdCount.fetch_add(1, std::memory_order_relaxed);
  }
}
