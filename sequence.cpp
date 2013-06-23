#include "sequence.h"

namespace xnor {
  SchedFunc::SchedFunc(seq_func_t func) : mFunc(func) { }

  void SchedFunc::exec(Seq * seq, Parent * parent) {
    if (mFunc)
      mFunc(seq, parent);
  }

  StartEndSched::StartEndSched(seq_tick_t end_offset) :
    Sched(),
    mEndOffset(end_offset)
  {
  }

  void StartEndSched::exec(Seq * seq, Parent * parent) {
    exec_start(seq, parent);
    //save a reference so that we don't dealloc before we exec end
    auto ref = shared_from_this(); 
    seq->schedule_absolute(mEndOffset, [ref](Seq * s, Parent * p) { ref->exec_end(s, p); });
  }

  StartEndSchedFunc::StartEndSchedFunc(seq_tick_t end_offset, seq_func_t start_func, seq_func_t end_func) :
    StartEndSched(end_offset),
    mStartFunc(start_func), mEndFunc(end_func)
  {
  }

  void StartEndSchedFunc::exec_start(Seq * seq, Parent * parent) {
    if (mStartFunc)
      mStartFunc(seq, parent);
  }

  void StartEndSchedFunc::exec_end(Seq * seq, Parent * parent) {
    if (mEndFunc)
      mEndFunc(seq, parent);
  }

  void PeriodicSched::exec(Seq * seq, Parent * parent) {
    exec_start(seq, parent);
    auto ref = shared_from_this(); 
    seq_func_t func = [ref, &func](Seq * s, Parent * p) {
      if (ref->exec_periodic(s, p))
        s->schedule_absolute(ref->tick_period(), func);
    };

    seq->schedule_absolute(mTickPeriod, func);
  }

  Scheduler::Scheduler() { }

  seq_tick_t Scheduler::schedule(seq_tick_t location, SchedPtr sched, bool push_front) {
    auto loc = mSchedule.find(location);
    if (loc != mSchedule.end()) {
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
      mSchedule[location] = list;
    }
    return 0; //XXX return some reference so we can remove the item
  }

  seq_tick_t Scheduler::schedule(seq_tick_t location, seq_func_t func, bool push_front) {
    SchedPtr sched(new SchedFunc(func));
    return schedule(location, sched, push_front);
  }

  void Scheduler::tick(Seq * seq) {
    auto cur = mSchedule.find(mCurrentLocation);
    mCurrentLocation++;

    if (cur != mSchedule.end()) {
      for (auto f: cur->second)
        f->exec(seq, this);
    }
  }

  void Scheduler::clear() {
    mSchedule.clear();
  }

  void Scheduler::locate(seq_tick_t location) {
    mCurrentLocation = location;
  }

  void Group::exec_start(Seq * seq, Parent * parent) {
    locate(0);
  }

  bool Group::exec_periodic(Seq * seq, Parent * parent) {
    tick(seq);
  }

  Seq::Seq() { }

  void Seq::schedule_absolute(seq_tick_t tick_offset, SchedPtr sched) {
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

  void Seq::schedule_absolute(seq_tick_t tick_offset, seq_func_t func) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(tick_offset, sched);
  }

  void Seq::schedule_absolute(double seconds_from_now, SchedPtr sched) {
    unsigned int milliseconds = static_cast<unsigned int>(seconds_from_now * 1000.0);
    //XXX do it!
  }

  void Seq::schedule_absolute(double seconds_from_now, seq_func_t func) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(seconds_from_now, sched);
  }


  void Seq::tick() {
    Scheduler::tick(this);
    for (auto it = mSeqAbsolute.begin(); it != mSeqAbsolute.end(); it++) {
      if (it->first > mTicksAbsolute)
        break;
      it->second->exec(this, this);
      it = mSeqAbsolute.erase(it);
    }
    mTicksAbsolute++;
  }
}
