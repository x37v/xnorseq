#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

namespace xnorseq {

  void Clock::update(timedur dur) { mNow += dur; }
  timepoint Clock::now() const { return mNow; }
  void Clock::now(timepoint v) { mNow = v; }
  timedur Clock::ticks_per_second() const { return mTicksPerSecond; }
  void Clock::ticks_per_second(timedur v) { mTicksPerSecond = v; }

  //XXX
  timedur Clock::to_system_ticks(timedur v) { return v; } 
  timedur Clock::system_to_ticks(timedur system_ticks) { return system_ticks; }

  ExecContext::ExecContext(timepoint now, timedur exec_period, sched_func_t sched_func, seek_func_t seek_func) :
    mNow(now),
    mPeriod(exec_period),
    mSchedFunc(sched_func),
    mSeekFunc(seek_func) { }

  void ExecContext::seek(timepoint t) {
    if (mSeekFunc)
      mSeekFunc(t);
  }

  void ExecContext::schedule(timepoint t, EventPtr event, ClockPtr clock) {
    if (mSchedFunc)
      mSchedFunc(t, event, clock);
  }


  SchedulePlayer::SchedulePlayer(EventSchedulePtr schedule) :
    mSchedule(schedule)
  {
  }

  void SchedulePlayer::exec(timepoint t, ExecContext context) {
    sched_func_t sched_func = [this](timepoint tp, EventPtr e, ClockPtr clock) {
      auto se = std::make_shared<xnorseq::ScheduleItem<EventPtr>>(e, tp);
      mLocalSchedule.schedule(se);
    };
    seek_func_t seek_func = nullptr;
    ExecContext local_context(mLocalTime, context.period(), sched_func, seek_func);

    auto func = [this, context](Schedule<EventPtr>::ScheduleItemPtr item) {
      item->item()->exec(item->time(), context);
    };

    timepoint time_next = mLocalTime + context.period();
    bool schedule_done = false;
    if (mLocalTime < mSchedule->length())
      mSchedule->each(mLocalTime, std::min(time_next, mSchedule->length() - 1), func);
    else
      schedule_done = true;
    mLocalTime = time_next;

    //deal with absolute time
    time_next = mLocalTimeAbsolute + context.period();
    while (auto ev = mLocalSchedule.pop_until(time_next)) {
      context.self(ev->item());
      ev->item()->exec(ev->time(), local_context);
    }
    mLocalTimeAbsolute = time_next;

    //XXX remove from schedule when we have nothing left in mSchedule or mLocalSchedule?
    //XXX maybe mSchedule should have a 'length' that can be unrelated to the actual
    //length [if desired] and after that we no longer read new items from it
    //and then just execute the local schedule until it is done then no longer reschedule self
    if (!schedule_done || mLocalSchedule.size() != 0) {
      context.schedule(t + context.period(), context.self());
    } else {
      cout << "removing from schedule" << endl;
    }
  }

  StartScheduleEvent::StartScheduleEvent(EventSchedulePtr schedule) : mSchedule(schedule) {
  }

  void StartScheduleEvent::exec(timepoint t, ExecContext context) {
    auto p = std::make_shared<xnorseq::SchedulePlayer>(mSchedule);
    context.schedule(t, p);
  }

  Seq::Seq() {
    mDefaultClock = std::make_shared<Clock>();
    mClocks.push_back(mDefaultClock);
  }

  void Seq::schedule(timepoint t, EventPtr e, ClockPtr clock) {
    if (!clock)
      clock = mDefaultClock;

    auto se = std::make_shared<xnorseq::ScheduleItem<EventPtr>>(e, t);
    mLocalSchedule.schedule(se);
  }

  void Seq::exec(timepoint t, timedur period) {
    sched_func_t sched_func = [this, t](timepoint tp, EventPtr e, ClockPtr clock) {
      schedule(tp, e, clock);
    };

    ExecContext context(t, period, sched_func, nullptr); //seek doesn't make sense in this context
    while (auto e = mLocalSchedule.pop_until(t)) {
      context.self(e->item());
      e->item()->exec(e->time(), context);
    }

    for (auto c: mClocks)
      c->update(period);
  }

}
