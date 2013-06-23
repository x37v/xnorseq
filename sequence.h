#ifndef XNOR_SEQUENCE_H
#define XNOR_SEQUENCE_H

#include <functional>
#include <map>
#include <list>
#include <memory>

namespace xnor {
  class Seq;
  class Scheduler;
  class Group;
  class Sched;
  typedef Scheduler Parent;

  typedef std::function<void(Seq * seq, Parent * parent)> seq_func_t;
  typedef std::function<bool(Seq * seq, Parent * parent)> seq_periodic_func_t;
  typedef std::shared_ptr<Sched> SchedPtr;
  typedef int seq_tick_t;
  typedef unsigned int sched_id_t;

  class Sched {
    public:
      Sched();
      virtual void exec(Seq * seq, Parent * parent) = 0;
      virtual ~Sched() {}
      sched_id_t id() const;
    private:
      sched_id_t mID;
  };

  class SchedFunc : public Sched {
    public:
      SchedFunc(seq_func_t func);
      virtual ~SchedFunc() {}
      virtual void exec(Seq * seq, Parent * parent);
    private:
      seq_func_t mFunc;
  };

  class StartEndSched : public Sched, public std::enable_shared_from_this<StartEndSched> {
    public:
      StartEndSched(seq_tick_t end_offset);
      virtual ~StartEndSched() {}
      virtual void exec(Seq * seq, Parent * parent);
      virtual void exec_start(Seq * seq, Parent * parent) = 0;
      virtual void exec_end(Seq * seq, Parent * parent) = 0;
    private:
      seq_tick_t mEndOffset;
  };

  class StartEndSchedFunc : public StartEndSched {
    public:
      StartEndSchedFunc(seq_tick_t end_offset, seq_func_t start_func, seq_func_t end_func);
      virtual ~StartEndSchedFunc() {}
      virtual void exec_start(Seq * seq, Parent * parent);
      virtual void exec_end(Seq * seq, Parent * parent);
    private:
      seq_func_t mStartFunc;
      seq_func_t mEndFunc;
  };

  class PeriodicSched : public Sched, public std::enable_shared_from_this<PeriodicSched> {
    public:
      PeriodicSched();
      virtual ~PeriodicSched() { }
      virtual void exec(Seq * seq, Parent * parent);
      virtual void exec_start(Seq * seq, Parent * parent) = 0;
      virtual bool exec_periodic(Seq * seq, Parent * parent) = 0; //true to keep in schedule
      seq_tick_t tick_period() const { return mTickPeriod; }
    private:
      seq_tick_t mTickPeriod = 1;
      seq_func_t mPeriodicEval = nullptr;
  };

  class PeriodicSchedFunc : public PeriodicSched {
    public:
      PeriodicSchedFunc(seq_periodic_func_t periodic_func, seq_func_t start_func = seq_func_t());
      virtual void exec_start(Seq * seq, Parent * parent);
      virtual bool exec_periodic(Seq * seq, Parent * parent);
    private:
      seq_func_t mStartFunc = nullptr;
      seq_periodic_func_t mPeriodicFunc = nullptr;
  };

  class Scheduler {
    public:
      Scheduler();
      virtual ~Scheduler() {}

      //schedule at time location given
      seq_tick_t schedule(seq_tick_t location, SchedPtr sched, bool push_front = false);
      seq_tick_t schedule(seq_tick_t location, seq_func_t func, bool push_front = false);

      virtual void tick(Seq * seq);

      void clear();

      virtual void locate(seq_tick_t location);
      seq_tick_t location() const { return mCurrentLocation; }
    private:
      std::map<seq_tick_t, std::list<SchedPtr> > mSchedule;
      seq_tick_t mCurrentLocation = 0;
  };

  class Group : public Scheduler, public PeriodicSched {
    public:
      Group();
      virtual ~Group() {}

      virtual void exec_start(Seq * seq, Parent * parent);
      virtual bool exec_periodic(Seq * seq, Parent * parent);

  };

  class Seq : public Scheduler {
    public:
      Seq();
      virtual ~Seq() {}

      //schedule to happen tick_offset or seconds from now, happens even if main schedule jumps
      void schedule_absolute(seq_tick_t tick_offset, SchedPtr sched);
      void schedule_absolute(seq_tick_t tick_offset, seq_func_t func);

      void schedule_absolute(double seconds_from_now, SchedPtr sched);
      void schedule_absolute(double seconds_from_now, seq_func_t func);

      void tick();
    private:
      virtual void tick(Seq *s);

      std::list<std::pair<seq_tick_t, SchedPtr> > mSeqAbsolute;
      seq_tick_t mTicksAbsolute = 0;
  };
}

#endif
