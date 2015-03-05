#ifndef XNOR_SEQUENCE_H
#define XNOR_SEQUENCE_H

#include <functional>
#include <map>
#include <list>
#include <memory>

namespace xnor {
  class Seq;
  class Schedule;
  class Sched;
  class SchedulePlayer;
  class Group;

  typedef SchedulePlayer Parent;
  typedef std::function<void(Seq * seq, Parent * parent)> seq_func_t;
  typedef std::function<bool(Seq * seq, Parent * parent)> seq_periodic_func_t;

  enum start_end_t {START, END};
  typedef std::function<void(start_end_t state, Seq * seq, Parent * parent)> start_end_func_t;

  typedef std::shared_ptr<Sched> SchedPtr;
  typedef std::shared_ptr<Schedule> SchedulePtr;
  typedef std::shared_ptr<SchedulePlayer> SchedulePlayerPtr;
  typedef std::shared_ptr<Group> GroupPtr;
  typedef int seq_tick_t;
  typedef unsigned int sched_id_t;

  class Sched {
    public:
      Sched();
      virtual void exec(Seq * seq, Parent * parent) = 0;
      virtual ~Sched() {}
      sched_id_t id() const { return mID; }
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

  class StartEndSched : public Sched {
    public:
      StartEndSched(seq_tick_t end_offset);
      virtual ~StartEndSched() {}
      virtual void exec(Seq * seq, Parent * parent);
      //return a new Sched object who's 'exec' will be called after end_offset
      virtual SchedPtr exec_start(Seq * seq, Parent * parent) = 0;
    private:
      seq_tick_t mEndOffset;
  };

  class StartEndSchedFunc : public StartEndSched, public std::enable_shared_from_this<StartEndSchedFunc> {
    public:
      StartEndSchedFunc(seq_tick_t end_offset, start_end_func_t func);
      virtual ~StartEndSchedFunc() {}
      virtual SchedPtr exec_start(Seq * seq, Parent * parent);
    private:
      start_end_func_t mFunc;
  };

  class PeriodicSched : public Sched {
    public:
      seq_tick_t tick_period() const { return mTickPeriod; }

      virtual void exec(Seq * seq, Parent * parent);

      virtual void exec_start(Seq * seq, Parent * parent); //default, do nothing
      virtual void exec_end(Seq * seq, Parent * parent); //default, do nothing

      //true to keep in schedule
      virtual bool exec_periodic(Seq * seq, Parent * parent) = 0;

      //the actual periodic evaluation happens on a copy
      //so you must create a clone method
      //the copy can store and modify its state
      //but make sure its exec_periodic eventually returns false
      virtual PeriodicSched * clone() = 0;
    private:
      seq_tick_t mTickPeriod = 1;
  };

  //dangerous, make sure that you use with care..
  class PeriodicSchedFunc : public PeriodicSched {
    public:
      PeriodicSchedFunc(seq_periodic_func_t periodic_func, seq_func_t start_func = seq_func_t(), seq_func_t end_func = seq_func_t());

      virtual void exec_start(Seq * seq, Parent * parent);
      virtual void exec_end(Seq * seq, Parent * parent);
      virtual bool exec_periodic(Seq * seq, Parent * parent);

      virtual PeriodicSched * clone();
    private:
      seq_func_t mStartFunc = nullptr;
      seq_func_t mEndFunc = nullptr;
      seq_periodic_func_t mPeriodicFunc = nullptr;
  };

  class Schedule {
    public:
      typedef std::map<seq_tick_t, std::list<SchedPtr> > schedule_list_t;

      Schedule();
      virtual ~Schedule() {}

      //schedule at time location given
      seq_tick_t schedule(seq_tick_t location, SchedPtr sched, bool push_front = false);
      seq_tick_t schedule(seq_tick_t location, seq_func_t func, bool push_front = false);

      seq_tick_t length() const;

      void clear();

      schedule_list_t& list() { return mSchedule; }
    private:
      schedule_list_t mSchedule;
  };

  class SchedulePlayer : public std::enable_shared_from_this<SchedulePlayer> {
    public:
      SchedulePlayer(SchedulePtr schedule);

      virtual void locate(seq_tick_t location);
      seq_tick_t location() const { return mCurrentLocation; }

      SchedulePtr schedule() { return mSchedule; }

      void tick(Seq * seq);
      void ms_per_tick(unsigned int ms) { mMSperTick = ms; }

    protected:
      SchedulePlayer();
      void schedule(SchedulePtr schedule);

      SchedulePtr mSchedule;

    private:
      seq_tick_t mCurrentLocation = 0;

      unsigned int ms_per_tick() const { return mMSperTick; }
      unsigned int mMSperTick = 10;
  };

  class Group : public Sched {
    public:
      Group();

      virtual void exec(Seq * seq, Parent * parent);

      //schedule at time location given
      seq_tick_t schedule(seq_tick_t location, SchedPtr sched, bool push_front = false);
      seq_tick_t schedule(seq_tick_t location, seq_func_t func, bool push_front = false);
    private:
      SchedulePtr mSchedule;
      //bool mUseParentTickRate = true;
  };

  class Seq : public SchedulePlayer {
    public:
      Seq();
      virtual ~Seq() {}

      //schedule at time location given
      seq_tick_t schedule(seq_tick_t location, SchedPtr sched, bool push_front = false);
      seq_tick_t schedule(seq_tick_t location, seq_func_t func, bool push_front = false);

      void clear();

      //schedule to happen tick_offset or seconds from now, happens even if main schedule jumps
      void schedule_absolute(seq_tick_t tick_offset, SchedPtr sched, SchedulePlayerPtr parent);
      void schedule_absolute(seq_tick_t tick_offset, seq_func_t func, SchedulePlayerPtr parent);

      void schedule_absolute(double seconds_from_now, SchedPtr sched, SchedulePlayerPtr parent);
      void schedule_absolute(double seconds_from_now, seq_func_t func, SchedulePlayerPtr parent);

      //sets up a dependency so that when parent is removed, child will be too
      void add_dependency(sched_id_t parent, sched_id_t child);
      void remove_dependency(sched_id_t parent, sched_id_t child);

      void remove_dependents(sched_id_t parent);

      void tick();
    private:
      struct abs_sched_t {
        abs_sched_t(seq_tick_t i, SchedPtr s, SchedulePlayerPtr p) {
          index = i;
          sched = s;
          parent = p;
        }
        seq_tick_t index;
        SchedPtr sched;
        SchedulePlayerPtr parent;
      };
      SchedulePtr mSchedule;
      std::list<abs_sched_t> mSeqAbsolute;
      seq_tick_t mTicksAbsolute = 0;
      std::map<sched_id_t, std::list<sched_id_t> > mDependencies;
  };
}

#endif
