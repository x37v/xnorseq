#pragma once

#include <boost/any.hpp>
#include <map>
#include <iostream>
#include <atomic>

namespace xnorseq {
  class Scheduler;
  class Callable;

  typedef unsigned int timepoint;
  typedef unsigned int timedur;
  typedef unsigned long obj_id_t; 
  typedef Callable * CallablePtr;


  namespace {
    std::map<obj_id_t, boost::any> obj_data;
    std::atomic<obj_id_t> obj_ids;
  }

  class CallData {
    public:
      CallData(Scheduler * s) : mScheduler(s) {}
      Scheduler * scheduler() const;
      timepoint time_now() const;
      void time_now(timepoint t) { mTimeNow = t; }
    private:
      Scheduler * mScheduler = nullptr;
      timepoint mTimeNow = 0;
  };

  class Callable {
    public:
      virtual ~Callable() {}
      virtual void call(CallData cd) const = 0;
  };

  //crtp https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
  template <typename T, typename D>
    class Executor : public Callable {
      public:
        Executor() { mID = obj_ids++; }
        virtual ~Executor() { obj_data.erase(id()); }

        unsigned int id() { return mID; }

        //override this
        void exec(CallData /*cd*/, D /*arg*/) const { /*XXX compile time assert that this is overridden? */ }

        virtual void call(CallData cd) const { static_cast<const T*>(this)->exec(cd, get()); }

        //override this to get other durations
        timedur duration(D arg) const { return static_cast<const T*>(this)->duration(arg); }

        //XXX need an atomic set/get or 
        void set(D d) const { obj_data[id()] = d; }
        D get() const { return boost::any_cast<D>(obj_data[id()]); }

        //XXX need a get based on parent chain and location within parent

        obj_id_t id() const { return mID; }
      private:
        obj_id_t mID = 0;
    };


  class ScheduleData {
    public:
      void schedule(timepoint t, CallablePtr c) {
        //XXX use better, realtime safe structure and insert sorted
        mSchedule.push_back(CallableAt(t, c));
      }
    private:
      struct CallableAt {
        CallableAt(timepoint t, CallablePtr c) : mTime(t), mCallable(c) {}
        timepoint time() { return mTime; }
        CallablePtr callable() { return mCallable; }
        timepoint mTime;
        CallablePtr mCallable;
      };
      std::vector<CallableAt> mSchedule;
  };

  class Scheduler : public Executor<Scheduler, ScheduleData> {
    public:
      void schedule(timepoint t, CallablePtr c) const {
        //XXX needs to happen in the thread that runs the schedule
        ScheduleData sd = get();
        sd.schedule(t, c);
      };

    //get items from timepoint to up to t + d
    //needs instance data.. based on parent and location scheuled..
    //std::tuple<start, end> iterator(timepoint t, duration d);
  };
}
