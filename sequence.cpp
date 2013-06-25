#include "sequence.h"
#include <atomic>
#include <algorithm>

#include <iostream>
using std::cerr;
using std::endl;

namespace {
  std::atomic<xnor::sched_id_t> sched_id_cnt = ATOMIC_VAR_INIT(0);
}

namespace xnor {
  Sched::Sched() {
    mID = sched_id_cnt++;
  }

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

  PeriodicSched::PeriodicSched() { }

  void PeriodicSched::exec(Seq * seq, Parent * parent) {
    exec_start(seq, parent);
    if (!mPeriodicEval) {
      auto ref = shared_from_this(); 
      mPeriodicEval = [ref, this](Seq * s, Parent * p) {
        if (ref->exec_periodic(s, p))
          s->schedule_absolute(ref->tick_period(), this->mPeriodicEval);
      };
    }
    seq->schedule_absolute(mTickPeriod, mPeriodicEval);
  }

  PeriodicSchedFunc::PeriodicSchedFunc(seq_periodic_func_t periodic_func, seq_func_t start_func) :
    mStartFunc(start_func), mPeriodicFunc(periodic_func)
  {
  }

  void PeriodicSchedFunc::exec_start(Seq * seq, Parent * parent) {
    if (mStartFunc)
      mStartFunc(seq, parent);
  }

  bool PeriodicSchedFunc::exec_periodic(Seq * seq, Parent * parent) {
    return mPeriodicFunc(seq, parent);
  }

  Schedule::Schedule() { }

  seq_tick_t Schedule::schedule(seq_tick_t location, SchedPtr sched, bool push_front) {
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

  seq_tick_t Schedule::schedule(seq_tick_t location, seq_func_t func, bool push_front) {
    SchedPtr sched(new SchedFunc(func));
    return schedule(location, sched, push_front);
  }

  void Schedule::clear() {
    mSchedule.clear();
  }

  SchedulePlayer::SchedulePlayer(SchedulePtr schedule) :
    mSchedule(schedule)
  {
  }

  SchedulePlayer::SchedulePlayer() {
  }

  void SchedulePlayer::schedule(SchedulePtr schedule) {
    mSchedule = schedule;
  }

  void SchedulePlayer::locate(seq_tick_t location) {
    mCurrentLocation = location;
  }

  void SchedulePlayer::exec_start(Seq * seq, Parent * parent) {
    locate(0);
  }

  bool SchedulePlayer::exec_periodic(Seq * seq, Parent * parent) {
    tick(seq);
    return true; //XXX fix
  }

  void SchedulePlayer::tick(Seq * seq) {
    if (!mSchedule)
      return;

    auto cur = mSchedule->list().find(mCurrentLocation);
    mCurrentLocation++;
    if (cur != mSchedule->list().end()) {
      for (auto f: cur->second)
        f->exec(seq, this);
    }
  }

  Seq::Seq() : SchedulePlayer() {
    mSchedule = SchedulePtr(new Schedule);
    SchedulePlayer::schedule(mSchedule);
  }

  seq_tick_t Seq::schedule(seq_tick_t location, SchedPtr sched, bool push_front) {
    return mSchedule->schedule(location, sched, push_front);
  }

  seq_tick_t Seq::schedule(seq_tick_t location, seq_func_t func, bool push_front) {
    return mSchedule->schedule(location, func, push_front);
  }

  void Seq::clear() {
    mSchedule->clear();
  }

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
    //unsigned int milliseconds = static_cast<unsigned int>(seconds_from_now * 1000.0);
    //XXX do it!
  }

  void Seq::schedule_absolute(double seconds_from_now, seq_func_t func) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(seconds_from_now, sched);
  }

  void Seq::tick() {
    SchedulePlayer::tick(this);
    for (auto it = mSeqAbsolute.begin(); it != mSeqAbsolute.end(); it++) {
      if (it->first > mTicksAbsolute)
        break;
      it->second->exec(this, this);
      it = mSeqAbsolute.erase(it);
    }
    mTicksAbsolute++;
  }
}
