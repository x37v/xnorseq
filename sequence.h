#pragma once

#include <memory>
#include <deque> //XXX tmp
#include <algorithm>

namespace xnorseq {
  typedef long timepoint;
  typedef long timedur;

  class Event;
  //class ScheduleItem;
  //class Schedule;
  class SchedulePlayer;
  class Seq;

  typedef std::shared_ptr<Event> EventPtr;
  //typedef std::shared_ptr<ScheduleItem> ScheduleItemPtr;
  //typedef std::shared_ptr<Schedule> SchedulePtr;
  typedef std::shared_ptr<SchedulePlayer> SchedulePlayerPtr;
  typedef std::shared_ptr<Seq> SeqPtr;

  typedef std::function<void(timepoint, EventPtr)> sched_func_t;
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
      ExecContext(timepoint now, timedur exec_period, sched_func_t sched_func, seek_func_t seek_func);

      timepoint now() const { return mNow; }
      timedur period() const { return mPeriod; }

      void self(EventPtr item) { mItem = item; }
      EventPtr self() const { return mItem; }

      //might not do anything
      void seek(timepoint t);
      //can reschedule self
      void schedule(timepoint t, EventPtr event);

      //XXX make into realtime safe alloc
      template <typename T, class... Args>
        std::shared_ptr<T> make_event(Args&&... args) {
          return std::make_shared<T>(std::forward(args)...);
        }
    private:
      timepoint mNow;
      timedur mPeriod;
      EventPtr mItem = nullptr;
      sched_func_t mSchedFunc;
      seek_func_t mSeekFunc;
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

      Schedule(timedur length = std::numeric_limits<timedur>::max()) : mLength(std::max(static_cast<timedur>(0), length)) {
      }

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

      //XXX in the future use an iterator so we can keep track of where we were without 
      //going through all the schedule
      void each(timepoint start, timepoint end, std::function<void(ScheduleItemPtr)> func) {
        auto it = mSchedule.begin();
        while (it != mSchedule.end() && (*it)->time() < start)
          it++;
        while (it != mSchedule.end() && (*it)->time() <= end) {
          func(*it);
          it++;
        }
      }

      timedur length() const { return mLength; }
      void length(timedur v) { mLength = std::max(static_cast<timedur>(0), v); }
    private:
      //XXX tmp
      std::deque<ScheduleItemPtr> mSchedule;
      timedur mLength;
  };

  typedef Schedule<EventPtr> EventSchedule;
  typedef std::shared_ptr<EventSchedule> EventSchedulePtr;

  class SchedulePlayer : public Event {
    public:
      SchedulePlayer(EventSchedulePtr schedule);
      virtual void exec(timepoint t, ExecContext context);
    private:
      EventSchedulePtr mSchedule;
      EventSchedule mLocalSchedule;
      timepoint mLocalTime = 0;
  };

  class StartScheduleEvent : public Event {
    public:
      StartScheduleEvent(EventSchedulePtr schedule);
      virtual void exec(timepoint t, ExecContext context);
    private:
      EventSchedulePtr mSchedule;
  };

  class Seq {
    public:
      void schedule(timepoint t, EventPtr e);
      void exec(timepoint t, timedur period);

#if 0
      //main thread side alloc, no need for realtime
      template <typename T>
      ObjectRef make_obj(Args&&... args);

      //realtime safe alloc
      template <typename T>
      EventPtr make_event(Args&&... args);

      template <typename T>
      void update(ObjectRef o, std::string attr, T v) {
        //XXX make sure that object supports attribute? Should be realtime safe
        //queue up update for updating object in realtime thread.
      }
#endif
    private:
      EventSchedule mLocalSchedule;
  };
}
