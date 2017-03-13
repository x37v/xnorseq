#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

namespace xnorseq {
  ExecContext::ExecContext(timepoint now, ee_sched_func_t sched_func, seek_func_t seek_func) :
    mNow(now), mSchedFunc(sched_func), mSeekFunc(seek_func) { }

  void ExecContext::seek(timepoint t) {
    if (mSeekFunc)
      mSeekFunc(t);
  }

  void ExecContext::schedule(timepoint t, EphemeralEventPtr event) {
    if (mSchedFunc)
      mSchedFunc(t, event);
  }


  EphemeralSchedulePlayer::EphemeralSchedulePlayer(EventSchedulePtr schedule) : mSchedule(schedule) {
  }

  void EphemeralSchedulePlayer::exec(timepoint t, ExecContext context) {
    auto func = [this, context](Schedule<EventPtr>::ScheduleItemPtr item) {
      item->item()->exec(item->time(), context);
    };
    mSchedule->each(mTimeLast, t, func);
    mTimeLast = t + 1;
  }

  SchedulePlayer::SchedulePlayer(EventSchedulePtr s) : Event(), mSchedule(s) {
  }

  void SchedulePlayer::exec(timepoint t, ExecContext context) {
    //XXX? auto e = context.make_event<EphemeralSchedulePlayer>(mSchedule);
    auto e = std::make_shared<EphemeralSchedulePlayer>(mSchedule);
    context.schedule(t, e);
  }

  void Seq::schedule(timepoint t, EphemeralEventPtr e) {
    auto se = std::make_shared<xnorseq::ScheduleItem<EphemeralEventPtr>>(e, t);
    mSchedule.schedule(se);
  }

  void Seq::exec(timepoint t) {
    ee_sched_func_t sched_func = [this, t](timepoint tp, EphemeralEventPtr e) {
      schedule(tp, e);
    };

    ExecContext context(t, sched_func, nullptr); //seek doesn't make sense in this context
    while (auto e = mSchedule.pop_until(t)) {
      context.self(e->item());
      e->item()->exec(e->time(), context);
    }
  }

}
