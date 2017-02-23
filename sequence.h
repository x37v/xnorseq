#pragma once

#include <memory>
#include <vector>
#include <map>
#include <atomic>
#include <utility>

namespace xnorseq {
  class Seq;
  class Scheduler;
  class Callable;

  typedef unsigned int timepoint;
  typedef unsigned int timedur;
  typedef unsigned long obj_id_t; 
  typedef std::shared_ptr<Callable> CallablePtr;


  class ObjectRef {
    public:
      ObjectRef(CallablePtr p) : mCallable(p) {}
      obj_id_t id() const;
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
      Callable(Seq * seq);
      virtual ~Callable();
      obj_id_t id() const;

      virtual void call(CallData cd) = 0;

    protected:
      Seq * mSeq;

    private:
      obj_id_t mID = 0;
      friend class Seq;
  };

  //crtp https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
  template <typename T, typename ObjData>
    class Executor : public Callable {
      public:
        Executor(Seq * seq, ObjData data = ObjData()) : Callable(seq), mData(data) { }

        //override this
        void exec(CallData /*cd*/, ObjData& /*arg*/) const { /*XXX compile time assert that this is overridden? */ }

        virtual void call(CallData cd) { static_cast<T*>(this)->exec(cd, mData); }

        //override this to get other durations
        timedur duration(ObjData arg) const { return static_cast<const T*>(this)->duration(arg); }

      private:
        ObjData mData;
    };

  template <typename ObjData>
    class FuncExec : public xnorseq::Executor<FuncExec<ObjData>, ObjData> {
    public:
      FuncExec(xnorseq::Seq* seq, std::function<void(xnorseq::CallData, ObjData&)> f, ObjData d = ObjData()) : 
        xnorseq::Executor<FuncExec<ObjData>, ObjData>(seq, d), mFunc(f) {}

      void exec(xnorseq::CallData cd, ObjData& arg) const { mFunc(cd, arg); }
    private:
      std::function<void(xnorseq::CallData, ObjData&)> mFunc;
  };

  //top level dog
  class Seq {
    public:
      Seq() : mObjectIds(0) {}

      //factory
      template<class T, class... Args >
        ObjectRef make_obj(Args&&... args) {
          const obj_id_t id = mObjectIds++;

          CallablePtr ptr = std::make_shared<T>(this, std::forward<Args>(args)...);
          ptr->mID = id;
          mObjects[id] = ptr;
          return ObjectRef(ptr);
        };

      template<class T>
        ObjectRef make_func_obj(std::function<void(CallData, T &d)> f, T d = T()) {
          return make_obj<FuncExec<T>>(f, d);
        };

      void clear_data(const Callable* c);

      void exec(ObjectRef r);
    private:
      std::map<obj_id_t, std::weak_ptr<Callable>> mObjects;
      std::atomic<obj_id_t> mObjectIds;
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
