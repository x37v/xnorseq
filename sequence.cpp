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

  void Seq::schedule(timepoint t, EphemeralEventPtr e) {
    auto se = std::make_shared<xnorseq::ScheduleItem<EphemeralEventPtr>>(e, t);
    mSchedule.schedule(se);
  }

  void Seq::exec(timepoint t) {
    ee_sched_func_t sched_func = [this, t](timepoint tp, EphemeralEventPtr e) {
      //don't allow any schedules in the current block
      if (tp <= t)
        tp = t + 1;
      schedule(tp, e);
    };

    ExecContext context(t, sched_func, nullptr); //seek doesn't make sense in this context
    while (auto e = mSchedule.pop_until(t)) {
      context.self(e->item());
      e->item()->exec(e->time(), context);
    }
  }

}
