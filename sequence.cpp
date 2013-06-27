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
  namespace {
    class PeriodicEvaluator : public Sched, public std::enable_shared_from_this<PeriodicEvaluator> {
      public:
        PeriodicEvaluator(std::shared_ptr<PeriodicSched> periodic) {
          mPeriodic = periodic;
        }

        virtual void exec(Seq * seq, Parent * parent) {
          if (mPeriodic->exec_periodic(seq, parent)) {
            SchedPtr ref = shared_from_this();
            seq->schedule_absolute(mPeriodic->tick_period(), ref, parent->shared_from_this());
          } else {
            mPeriodic->exec_end(seq, parent);
          }
        }
      private:
        std::shared_ptr<PeriodicSched> mPeriodic;
    };
  }
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
    auto end_obj = exec_start(seq, parent);
    seq->schedule_absolute(mEndOffset, end_obj, parent->shared_from_this());
  }

  StartEndSchedFunc::StartEndSchedFunc(seq_tick_t end_offset, seq_func_t start_func, seq_func_t end_func) :
    StartEndSched(end_offset),
    mStartFunc(start_func), mEndFunc(end_func)
  {
  }

  SchedPtr StartEndSchedFunc::exec_start(Seq * seq, Parent * parent) {
    if (mStartFunc)
      mStartFunc(seq, parent);
    return SchedPtr(new SchedFunc(mEndFunc));
  }

  void PeriodicSched::exec(Seq * seq, Parent * parent) {
    std::shared_ptr<PeriodicSched> ref(clone());
    SchedPtr e(new PeriodicEvaluator(ref));
    ref->exec_start(seq, parent);
    seq->schedule_absolute(ref->tick_period(), e, parent->shared_from_this());
  }

  //by default, do nothing
  void PeriodicSched::exec_start(Seq * seq, Parent * parent) { }
  void PeriodicSched::exec_end(Seq * seq, Parent * parent) { }

  PeriodicSchedFunc::PeriodicSchedFunc(seq_periodic_func_t periodic_func, seq_func_t start_func, seq_func_t end_func) :
    mStartFunc(start_func), mEndFunc(end_func), mPeriodicFunc(periodic_func)
  {
  }

  void PeriodicSchedFunc::exec_start(Seq * seq, Parent * parent) {
    if (mStartFunc)
      mStartFunc(seq, parent);
  }

  void PeriodicSchedFunc::exec_end(Seq * seq, Parent * parent) {
    if (mEndFunc)
      mEndFunc(seq, parent);
  }

  bool PeriodicSchedFunc::exec_periodic(Seq * seq, Parent * parent) {
    return mPeriodicFunc(seq, parent);
  }

  PeriodicSched * PeriodicSchedFunc::clone() {
    return static_cast<PeriodicSched *>(new PeriodicSchedFunc(*this));
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

  void Seq::schedule_absolute(seq_tick_t tick_offset, SchedPtr sched, SchedulePlayerPtr parent) {
    seq_tick_t pos = tick_offset + mTicksAbsolute;
    auto item = abs_sched_t(pos, sched, parent);
    if (mSeqAbsolute.empty() || mSeqAbsolute.back().index < pos)
      mSeqAbsolute.push_back(item);
    else if (mSeqAbsolute.front().index >= pos) {
      mSeqAbsolute.push_front(item);
    } else {
      for (auto it = mSeqAbsolute.begin(); it != mSeqAbsolute.end(); it++) {
        if (it->index >= pos) {
          mSeqAbsolute.insert(it, item);
          break;
        }
      }
    }
  }

  void Seq::schedule_absolute(seq_tick_t tick_offset, seq_func_t func, SchedulePlayerPtr parent) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(tick_offset, sched, parent);
  }

  void Seq::schedule_absolute(double seconds_from_now, SchedPtr sched, SchedulePlayerPtr parent) {
    //unsigned int milliseconds = static_cast<unsigned int>(seconds_from_now * 1000.0);
    //XXX do it!
  }

  void Seq::schedule_absolute(double seconds_from_now, seq_func_t func, SchedulePlayerPtr parent) {
    SchedPtr sched(new SchedFunc(func));
    return schedule_absolute(seconds_from_now, sched, parent);
  }

  void Seq::tick() {
    SchedulePlayer::tick(this);
    for (auto it = mSeqAbsolute.begin(); it != mSeqAbsolute.end(); it++) {
      if (it->index > mTicksAbsolute)
        break;
      it->sched->exec(this, it->parent.get());
      it = mSeqAbsolute.erase(it);
    }
    mTicksAbsolute++;
  }
}
