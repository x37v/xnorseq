#pragma once

#include <boost/any.hpp>
#include <memory>
#include <map>
#include <iostream>
#include <atomic>
#include <utility>

namespace xnorseq {
  class Seq;
  class Scheduler;
  class Callable;
  class DataStore;

  typedef unsigned int timepoint;
  typedef unsigned int timedur;
  typedef unsigned long obj_id_t; 
  typedef std::shared_ptr<Callable> CallablePtr;


  namespace {
    typedef std::map<obj_id_t, boost::any> data_store_t;
  }

  class ObjectRef {
    public:
      ObjectRef(CallablePtr p) : mCallable(p) {}
    private:
      CallablePtr mCallable;
  };

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
      Callable(Seq * seq) : mSeq(seq) {}
      virtual ~Callable() {}
      virtual void call(CallData cd) const = 0;

      obj_id_t id() const { return mID; }

    protected:
      Seq * mSeq;

    private:
      obj_id_t mID = 0;
      friend class Seq;
  };

  //top level dog
  class Seq {
    public:
      Seq() : mObjectIds(0) {}

      //factory, just gives you an id back
      template<class T, typename D, class... Args >
        ObjectRef make_obj(D data, Args&&... args) {
          const obj_id_t id = mObjectIds++;

          CallablePtr ptr = std::make_shared<T>(this, data, std::forward<Args>(args)...);
          ptr->mID = id;
          mObjects[id] = ptr;
          return ObjectRef(ptr);
        };

      template<typename T>
        void set_data(const Callable* c, T d) {
          mObjectData[c->id()] = d;
        }
      template<typename T>
        T get_data(const Callable* c) {
          return boost::any_cast<T>(mObjectData[c->id()]);
        }
    private:
      std::map<obj_id_t, CallablePtr> mObjects;
      data_store_t mObjectData;
      std::atomic<obj_id_t> mObjectIds;
  };

  //crtp https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
  template <typename T, typename D>
    class Executor : public Callable {
      public:
        Executor(Seq * seq, D data) : Callable(seq) { }

        //override this
        void exec(CallData /*cd*/, D /*arg*/) const { /*XXX compile time assert that this is overridden? */ }

        virtual void call(CallData cd) const { static_cast<const T*>(this)->exec(cd, get_data()); }

        //override this to get other durations
        timedur duration(D arg) const { return static_cast<const T*>(this)->duration(arg); }

        //XXX need an atomic set/get or 
        void set_data(D d) const { mSeq->set_data(this, d); }
        D get_data() const { return mSeq->get_data<D>(this); }

        //XXX need a get based on parent chain and location within parent
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

#if 0
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
#endif
}
