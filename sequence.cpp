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

  void ExecContext::schedule(timepoint t, EventPtr event) {
    if (mSchedFunc)
      mSchedFunc(t, event);
  }


  SchedulePlayer::SchedulePlayer(EventSchedulePtr schedule, timepoint start_offset) :
    mSchedule(schedule),
    mParentTimeOffset(start_offset)
  {
  }

  void SchedulePlayer::exec(timepoint t, ExecContext context) {
    auto func = [this, context](Schedule<EventPtr>::ScheduleItemPtr item) {
      item->item()->exec(item->time(), context);
    };
    auto s = mTimeLast - mParentTimeOffset;
    auto e = t - mParentTimeOffset;

    mSchedule->each(s, e, func);
    mTimeLast = t + 1;

    context.schedule(context.now() + 1, context.self());
  }

  StartScheduleEvent::StartScheduleEvent(EventSchedulePtr schedule) : mSchedule(schedule) {
  }

  void StartScheduleEvent::exec(timepoint t, ExecContext context) {
    auto p = std::make_shared<xnorseq::SchedulePlayer>(mSchedule, t);
    context.schedule(t, p);
  }

  void Seq::schedule(timepoint t, EventPtr e) {
    auto se = std::make_shared<xnorseq::ScheduleItem<EventPtr>>(e, t);
    mSchedule.schedule(se);
  }

  void Seq::exec(timepoint t) {
    ee_sched_func_t sched_func = [this, t](timepoint tp, EventPtr e) {
      schedule(tp, e);
    };

    ExecContext context(t, sched_func, nullptr); //seek doesn't make sense in this context
    while (auto e = mSchedule.pop_until(t)) {
      context.self(e->item());
      e->item()->exec(e->time(), context);
    }
  }

}
