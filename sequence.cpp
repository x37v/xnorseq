#include "sequence.h"

#include <iostream>
using std::cout;
using std::endl;


namespace xnor {
  SchedFunc::SchedFunc(seq_func_t func) : mFunc(func) { }

  void SchedFunc::exec(Sequence * seq) {
    if (mFunc)
      mFunc(seq);
  }

  StartEndSched::StartEndSched(seq_tick_t end_offset) :
    Sched(),
    mEndOffset(end_offset)
  {
  }

  void StartEndSched::exec(Sequence * seq) {
    exec_start(seq);
    //save a reference so that we don't dealloc before we exec end
    auto ref = shared_from_this(); 
    seq->schedule_absolute(mEndOffset, [ref](Sequence * s) { ref->exec_end(s); });
  }

  StartEndSchedFunc::StartEndSchedFunc(seq_tick_t end_offset, seq_func_t start_func, seq_func_t end_func) :
    StartEndSched(end_offset),
    mStartFunc(start_func), mEndFunc(end_func)
  {
  }

  void StartEndSchedFunc::exec_start(Sequence * seq) {
    if (mStartFunc)
      mStartFunc(seq);
  }

  void StartEndSchedFunc::exec_end(Sequence * seq) {
    if (mEndFunc)
      mEndFunc(seq);
  }

  Sequence::Sequence() { }

  seq_tick_t Sequence::schedule(seq_tick_t location, SchedPtr sched, bool push_front) {
    auto loc = mSeq.find(location);
    if (loc != mSeq.end()) {
      if (push_front)
        loc->second.push_front(sched);
      else
        loc->second.push_back(sched);
    } else {
      std::list<SchedPtr> list;
      if (push_front)
        list.push_front(sched);
      else
        list.push_back(sched);
      mSeq[location] = list;
    }
    return 0; //XXX return some reference so we can remove the item
  }

  seq_tick_t Sequence::schedule(seq_tick_t location, seq_func_t func, bool push_front) {
    SchedPtr sched(new SchedFunc(func));
    return schedule(location, sched, push_front);
  }

  void Sequence::schedule_absolute(seq_tick_t tick_offset, SchedPtr sched) {
    seq_tick_t pos = tick_offset + mTicksAbsolute;
    auto item = std::make_pair(pos, sched);
    if (mSeqAbsolute.empty() || mSeqAbsolute.back().first < pos)
      mSeqAbsolute.push_back(item);
    else if (mSeqAbsolute.front().first >= pos) {
      mSeqAbsolute.push_front(item);
    } else {
      for (auto it = mSeqAbsolute.begin(); it != mSeqAbsolute.end(); it++) {
        if (it->first >= pos) {
          mSeqAbsolute.insert(it, item);
          break;
        }
      }
    }
  }

  void Sequence::schedule_absolute(seq_tick_t tick_offset, seq_func_t func) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(tick_offset, sched);
  }

  void Sequence::schedule_absolute(double seconds_from_now, SchedPtr sched) {
    unsigned int milliseconds = static_cast<unsigned int>(seconds_from_now * 1000.0);
    //XXX do it!
  }

  void Sequence::schedule_absolute(double seconds_from_now, seq_func_t func) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(seconds_from_now, sched);
  }


  void Sequence::tick() {
    auto cur = mSeq.find(mCurrentLocation);
    mCurrentLocation++;

    if (cur != mSeq.end()) {
      for (auto f: cur->second)
        f->exec(this);
    }
    for (auto it = mSeqAbsolute.begin(); it != mSeqAbsolute.end(); it++) {
      if (it->first > mTicksAbsolute)
        break;
      it->second->exec(this);
      it = mSeqAbsolute.erase(it);
    }

    mTicksAbsolute++;
  }

  void Sequence::clear() {
    mSeq.clear();
  }

  void Sequence::locate(seq_tick_t location) {
    mCurrentLocation = location;
  }
}

class Rando {
  public:
    void exec(xnor::Sequence * s) {
      cout << "rando exec" << endl;
    }
};

int main(int argc, char * argv[]) {
  xnor::Sequence seq;
  auto s = [](xnor::Sequence * s) {
    cout << "start func" << endl;
  };

  auto e = [](xnor::Sequence * s) {
    cout << "end func" << endl;
  };

  {
    xnor::SchedPtr p(new xnor::StartEndSchedFunc(2, s, e));
    seq.schedule(2, p);
  }

  seq.schedule(5, [](xnor::Sequence *s) { s->locate(2); });

  Rando r;
  seq.schedule(3, std::bind(&Rando::exec, r, std::placeholders::_1));

  for (int i = 0; i < 20; i++) {
    cout << i << endl;
    if (i == 8)
      seq.clear();
    seq.tick();
  }
}
