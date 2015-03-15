#include "sequence.h"
#include <atomic>
#include <algorithm>
#include <cassert>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace {
  std::atomic<xnor::sched_id_t> sched_id_cnt = ATOMIC_VAR_INIT(0);

}

namespace xnor {
  namespace {

    /*
    class GroupPlayer : public PeriodicSched {
      public:
        GroupPlayer(SchedulePtr schedule) :
          mPlayer(new SchedulePlayer(schedule)) {
          }

        virtual void exec_start(Seq * seq, Parent * parent) {
          mParentOffset = parent->location();
          mPlayer->locate(0);
        }

        virtual void exec_end(Seq * seq, Parent * parent) {
          //XXX remove dependents
        }

        virtual bool exec_periodic(Seq * seq, Parent * parent) {
          mPlayer->tick(seq);
          seq_tick_t parent_loc = mPlayer->location() + mParentOffset;
          return parent_loc > parent->location() && mParentOffset + mPlayer->schedule()->length() < parent->location();
        }

        virtual PeriodicSched * clone() {
          return static_cast<PeriodicSched *>(new GroupPlayer(*this));
        }
      private:
        SchedulePlayerPtr mPlayer;
        seq_tick_t mParentOffset = 0;
    };
    */
  }

  Sched::Sched() {
    mID = sched_id_cnt++;
  }

  SchedFunc::SchedFunc(seq_func_t func) : mFunc(func) { }

  void SchedFunc::exec(Seq * seq, Parent * parent) {
    if (mFunc)
      mFunc(seq, this, parent);
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

  /*
  StartEndSchedFunc::StartEndSchedFunc(seq_tick_t end_offset, start_end_func_t func) :
    StartEndSched(end_offset),
    mFunc(func)
  {
    assert(mFunc);
  }

  SchedPtr StartEndSchedFunc::exec_start(Seq * seq, Parent * parent) {
    mFunc(SE_START, seq, this, parent);

    std::weak_ptr<StartEndSchedFunc> weak_self(shared_from_this());
    seq_func_t end_func = [weak_self](Seq * seq, Sched * owner, Parent * parent) {
      auto self = weak_self.lock();
      if (self)
        self->mFunc(SE_END, seq, owner, parent);
    };

    return SchedPtr(new SchedFunc(end_func));
  }
  */

  /*
  PeriodicSchedFunc::PeriodicSchedFunc(seq_periodic_func_t periodic_func) :
    mPeriodicFunc(periodic_func)
  {
    assert(mPeriodicFunc);
  }

  void PeriodicSchedFunc::exec_start(Seq * seq, Parent * parent) {
    mPeriodicFunc(P_START, seq, this, parent);
  }

  void PeriodicSchedFunc::exec_end(Seq * seq, Parent * parent) {
    mPeriodicFunc(P_END, seq, this, parent);
  }

  bool PeriodicSchedFunc::exec_periodic(Seq * seq, Parent * parent) {
    return mPeriodicFunc(P_PERIODIC, seq, this, parent);
  }

  PeriodicSched * PeriodicSchedFunc::clone() {
    return static_cast<PeriodicSched *>(new PeriodicSchedFunc(*this));
  }
  */

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

  seq_tick_t Schedule::length() const {
    //XXX fix for items that have length
    if (mSchedule.empty())
      return 0;
    return mSchedule.rbegin()->first;
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
    seq_tick_t offset = location - mCurrentLocation;
    mCurrentLocation = location;

    for (auto& kv: mObservers) {
      std::shared_ptr<SchedulePlayerObserver> ob = kv.second.lock();
      if (ob) {
        ob->location_jumped(this, offset);
      } else {
        cerr << "couldn't get pointer" << endl;
        //XXX delete observer?
      }
    }
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

  /*
  Group::Group() :
    mSchedule(new Schedule)
  {
  }

  void Group::exec(Seq * seq, Parent * parent) {
    //create player and add dependencies
    std::shared_ptr<GroupPlayer> player = std::make_shared<GroupPlayer>(mSchedule);
    seq->add_dependency(id(), player->id());
    player->exec(seq, parent);
  }

  seq_tick_t Group::schedule(seq_tick_t location, SchedPtr sched, bool push_front) {
    return mSchedule->schedule(location, sched, push_front);
  }

  seq_tick_t Group::schedule(seq_tick_t location, seq_func_t func, bool push_front) {
    return mSchedule->schedule(location, func, push_front);
  }
  */

  void SchedulePlayer::add_observer(std::shared_ptr<SchedulePlayerObserver> observer) {
    assert(observer);
    auto it = mObservers.find(observer.get());
    if (it != mObservers.end())
      return;
    mObservers[observer.get()] = observer; //weak
  }

  void SchedulePlayer::remove_observer(std::shared_ptr<SchedulePlayerObserver> observer) {
    assert(observer);
    remove_observer(observer.get());
  }

  void SchedulePlayer::remove_observer(SchedulePlayerObserver * observer) {
    auto it = mObservers.find(observer);
    if (it == mObservers.end())
      return;
    mObservers.erase(it);
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
