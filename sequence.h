#pragma once

#include <memory>
#include <deque> //XXX tmp
#include <algorithm>

namespace xnorseq {
  typedef long timepoint;
  typedef long timedur;

  class Clock;
  class Event;
  //class ScheduleItem;
  //class Schedule;
  class SchedulePlayer;
  class Seq;

  typedef std::shared_ptr<Clock> ClockPtr;
  typedef std::shared_ptr<Event> EventPtr;
  typedef std::shared_ptr<SchedulePlayer> SchedulePlayerPtr;
  typedef std::shared_ptr<Seq> SeqPtr;

  typedef std::function<void(timepoint, EventPtr, ClockPtr)> sched_func_t;
  typedef std::function<void(timepoint)> seek_func_t;

  class Clock {
    public:
      virtual void update(timedur dur);
      virtual timepoint now() const;
      virtual void now(timepoint v);

      virtual timedur ticks_per_second() const;
      virtual void ticks_per_second(timedur v);

      //XXX need to figure out if a duration will happen in the next X system ticks
      //so we can reschedule for the next period or not..
      virtual timedur to_system_ticks(timedur v);

      virtual timedur system_to_ticks(timedur system_ticks);
    private:
      timepoint mNow;
      timedur mTicksPerSecond;
  };

  //XXX need to be able to schedule events with a clock
  //
  //need 'countdown' events that always happen after X amount of 'time' [beats, measures, seconds]
  //no matter where the seek happens.. and timepoint events which happen AT time X, and maybe get
  //removed from the schedule on a seek?
  //
  //'countdown' would be like notes with duration
  //'at' would be like loop points
  //
  //
  //
  //
  
  /*
   * XXX for each period, for each clock, calculate the current tick and the tick at the start of the next period.
   * then for each thing evaluated with that clock during that period you can delay until the next period if
   * it isn't within that tick range.. 
   */


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
      void schedule(timepoint t, EventPtr event, ClockPtr clock = nullptr);

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

      size_t size() const { return mSchedule.size(); }
    private:
      std::deque<ScheduleItemPtr> mSchedule; //XXX TEMP, replace with non allocating double linked list
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
      timepoint mLocalTimeAbsolute = 0;
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
      Seq();

      void schedule(timepoint t, EventPtr e, ClockPtr clock = nullptr);
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
      std::vector<ClockPtr> mClocks; //XXX make realtime safe
      ClockPtr mDefaultClock;
  };
}
