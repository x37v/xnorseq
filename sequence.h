#pragma once

#include <memory>
#include <deque> //XXX tmp
#include <algorithm>

namespace xnorseq {
  typedef unsigned long timepoint;
  typedef unsigned long timedur;

  class Event;
  class EphemeralEvent;
  //class ScheduleItem;
  //class Schedule;
  class SchedulePlayer;
  class EphemeralSchedulePlayer;
  class Seq;

  typedef std::shared_ptr<Event> EventPtr;
  typedef std::shared_ptr<EphemeralEvent> EphemeralEventPtr;
  //typedef std::shared_ptr<ScheduleItem> ScheduleItemPtr;
  //typedef std::shared_ptr<Schedule> SchedulePtr;
  typedef std::shared_ptr<SchedulePlayer> SchedulePlayerPtr;
  typedef std::shared_ptr<EphemeralSchedulePlayer> EphemeralSchedulePlayerPtr;
  typedef std::shared_ptr<Seq> SeqPtr;

  typedef std::function<void(timepoint, EphemeralEventPtr)> ee_sched_func_t;
  typedef std::function<void(timepoint)> seek_func_t;

  //context should include ability to:
  //update parent schedule location ['next' exec]
  //create and add new events to schedule
  //add self back to schedule
  //
  //.. should use methods that might not do anything if it doesn't make
  //  sense.. like an 'exec' called by a trigger method that cannot be
  //  rescheduled etc.... use std function?
  class ExecContext {
    public:
      //XXX pass allocator as well
      ExecContext(timepoint now, ee_sched_func_t sched_func, seek_func_t seek_func);

      timepoint now() const { return mNow; }

      void self(EphemeralEventPtr item) { mItem = item; }
      EphemeralEventPtr self() const { return mItem; }

      //might not do anything
      void seek(timepoint t);
      //can reschedule self
      void schedule(timepoint t, EphemeralEventPtr event);

      //XXX make into realtime safe alloc
      template <typename T, class... Args>
        EphemeralEventPtr make_event(Args&&... args) {
          return std::make_shared<T>(std::forward(args)...);
        }
    private:
      timepoint mNow;
      EphemeralEventPtr mItem = nullptr;
      ee_sched_func_t mSchedFunc;
      seek_func_t mSeekFunc;
  };

  class EphemeralEvent {
    public:
      virtual void exec(timepoint t, ExecContext context) = 0;
  };

  class Event {
    public:
      virtual void exec(timepoint t, ExecContext context) = 0;
  };


  template <typename T>
  class ScheduleItem {
    public:
      ScheduleItem(T item, timepoint t) : mItem(item), mTimepoint(t) {}
      T item() const { return mItem; }
      timepoint time() const { return mTimepoint; }
    private:
      T mItem;
      timepoint mTimepoint;
  };

  template <typename T>
  class Schedule {
    public:
      typedef std::shared_ptr<ScheduleItem<T>> ScheduleItemPtr;

      //add event to schedule
      //XXX realtime safe only
      void schedule(ScheduleItemPtr item) {
        mSchedule.push_back(item);
        std::sort(mSchedule.begin(), mSchedule.end(), [](ScheduleItemPtr a, ScheduleItemPtr b) { return a->time() < b->time(); });
      }

      ScheduleItemPtr pop_until(timepoint t) {
        if (mSchedule.size() == 0 || mSchedule.front()->time() > t)
          return nullptr;
        auto r = mSchedule.front();
        mSchedule.pop_front();
        return r;
      }
    private:
      //XXX tmp
      std::deque<ScheduleItemPtr> mSchedule;
  };

  /*

  class EphemeralSchedulePlayer : public EphemeralEvent {
    public:
      void exec(timepoint t, Context context) {
        //execute from last t until t... reschedule self if needed.
      }
  };

  class SchedulePlayer : public Event {
    public:
      void exec(timepoint t, Context context) {
        //insert an EphemeralSchedulePlayer into 'live' schedule..
        EphemeralSchedulePlayerPtr e = context.make_obj<EphemeralSchedulePlayer>(mSchedule);
        context.schedule(t, e);
      }
  };
  */

  class Seq {
    public:
      void schedule(timepoint t, EphemeralEventPtr e);
      void exec(timepoint t);

#if 0
      //main thread side alloc, no need for realtime
      template <typename T>
      ObjectRef make_obj(Args&&... args);

      //realtime safe alloc
      template <typename T>
      EphemeralEventPtr make_event(Args&&... args);

      template <typename T>
      void update(ObjectRef o, std::string attr, T v) {
        //XXX make sure that object supports attribute? Should be realtime safe
        //queue up update for updating object in realtime thread.
      }
#endif
    private:
      Schedule<EphemeralEventPtr> mSchedule;
  };
}
