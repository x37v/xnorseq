#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

namespace xnorseq {
  ExecContext::ExecContext(timepoint now, timedur exec_period, sched_func_t sched_func, seek_func_t seek_func) :
    mNow(now),
    mPeriod(exec_period),
    mSchedFunc(sched_func),
    mSeekFunc(seek_func) { }

  void ExecContext::seek(timepoint t) {
    if (mSeekFunc)
      mSeekFunc(t);
  }

  void ExecContext::schedule(timepoint t, EventPtr event) {
    if (mSchedFunc)
      mSchedFunc(t, event);
  }


  SchedulePlayer::SchedulePlayer(EventSchedulePtr schedule) :
    mSchedule(schedule)
  {
  }

  void SchedulePlayer::exec(timepoint t, ExecContext context) {
    sched_func_t sched_func = [this](timepoint tp, EventPtr e) {
      auto se = std::make_shared<xnorseq::ScheduleItem<EventPtr>>(e, tp);
      mLocalSchedule.schedule(se);
    };
    seek_func_t seek_func = nullptr;
    ExecContext local_context(mLocalTime, context.period(), sched_func, seek_func);

    auto func = [this, context](Schedule<EventPtr>::ScheduleItemPtr item) {
      item->item()->exec(item->time(), context);
    };

    timepoint time_next = mLocalTime + context.period();
    if (mLocalTime < mSchedule->length())
      mSchedule->each(mLocalTime, std::min(time_next, mSchedule->length() - 1), func);

    while (auto ev = mLocalSchedule.pop_until(time_next)) {
      context.self(ev->item());
      ev->item()->exec(ev->time(), local_context);
    }

    mLocalTime = time_next;

    //XXX remove from schedule when we have nothing left in mSchedule or mLocalSchedule?
    //XXX maybe mSchedule should have a 'length' that can be unrelated to the actual
    //length [if desired] and after that we no longer read new items from it
    //and then just execute the local schedule until it is done then no longer reschedule self
    context.schedule(t + context.period(), context.self());
  }

  StartScheduleEvent::StartScheduleEvent(EventSchedulePtr schedule) : mSchedule(schedule) {
  }

  void StartScheduleEvent::exec(timepoint t, ExecContext context) {
    auto p = std::make_shared<xnorseq::SchedulePlayer>(mSchedule);
    context.schedule(t, p);
  }

  void Seq::schedule(timepoint t, EventPtr e) {
    auto se = std::make_shared<xnorseq::ScheduleItem<EventPtr>>(e, t);
    mLocalSchedule.schedule(se);
  }

  void Seq::exec(timepoint t, timedur period) {
    sched_func_t sched_func = [this, t](timepoint tp, EventPtr e) {
      schedule(tp, e);
    };

    ExecContext context(t, period, sched_func, nullptr); //seek doesn't make sense in this context
    while (auto e = mLocalSchedule.pop_until(t)) {
      context.self(e->item());
      e->item()->exec(e->time(), context);
    }
  }

}
