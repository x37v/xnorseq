#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

namespace xnorseq {
  obj_id_t ObjectRef::id() const { return mCallable->id(); }

  Callable::Callable(Seq * seq) : mSeq(seq) {}
  Callable::~Callable() { 
    mSeq->clear_data(this);
    cout << "deleting: " << id() << endl;
  }
  obj_id_t Callable::id() const { return mID; }

  void Seq::clear_data(const Callable* c) {
    //XXX this should never happen in the 'exec' thread
    mObjects.erase(c->id());
  }

  void Seq::exec(ObjectRef r) {
    auto it = mObjects.find(r.id());
    if (it == mObjects.end())
      return; //XXX log error?
    CallablePtr ptr = it->second.lock();
    CallData cd(nullptr);
    ptr->call(cd);
  }
}
