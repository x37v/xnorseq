#ifndef XNOR_SEQUENCE_H
#define XNOR_SEQUENCE_H

#include <functional>
#include <map>
#include <unordered_map>
#include <list>
#include <memory>

namespace xnor {
  class Seq;
  class Schedule;
  class Sched;
  class SchedulePlayer;
  class Group;

  typedef SchedulePlayer Parent;

  enum se_state_t {SE_START, SE_END, SE_JUMPED};
  enum p_state_t {P_START, P_PERIODIC, P_END, P_JUMPED};

  typedef std::function<void(Seq * seq, Sched * owner, Parent * parent)> seq_func_t;
  typedef std::function<void(se_state_t state, Seq * seq, Sched * owner, Parent * parent)> start_end_func_t;

  typedef std::shared_ptr<Sched> SchedPtr;
  typedef std::shared_ptr<Schedule> SchedulePtr;
  typedef std::shared_ptr<SchedulePlayer> SchedulePlayerPtr;
  typedef std::shared_ptr<Group> GroupPtr;
  typedef int seq_tick_t;
  typedef unsigned int sched_id_t;

  class SchedulePlayerObserver {
    public:
      virtual void location_jumped(SchedulePlayer * player, seq_tick_t offset) = 0;
  };

  class Sched : public std::enable_shared_from_this<Sched> {
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

  /*
  class StartEndSchedFunc : public StartEndSched, public std::enable_shared_from_this<StartEndSchedFunc> {
    public:
      StartEndSchedFunc(seq_tick_t end_offset, start_end_func_t func);
      virtual ~StartEndSchedFunc() {}
      virtual SchedPtr exec_start(Seq * seq, Parent * parent);
    private:
      start_end_func_t mFunc;
  };
  */

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
      seq_tick_t location_last() const { return mLastLocation; }

      SchedulePtr schedule() { return mSchedule; }

      void tick(Seq * seq);
      void ms_per_tick(unsigned int ms) { mMSperTick = ms; }

      void add_observer(std::shared_ptr<SchedulePlayerObserver> observer);
      void remove_observer(std::shared_ptr<SchedulePlayerObserver> observer);
      void remove_observer(SchedulePlayerObserver * observer);

    protected:
      SchedulePlayer();
      void schedule(SchedulePtr schedule);

      SchedulePtr mSchedule;

    private:
      seq_tick_t mCurrentLocation = 0;
      seq_tick_t mLastLocation = 0;

      unsigned int ms_per_tick() const { return mMSperTick; }
      unsigned int mMSperTick = 10;
      std::unordered_map<SchedulePlayerObserver *, std::weak_ptr<SchedulePlayerObserver>> mObservers;
  };

  /*
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
  */

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

      /*
      void schedule_absolute(double seconds_from_now, SchedPtr sched, SchedulePlayerPtr parent);
      void schedule_absolute(double seconds_from_now, seq_func_t func, SchedulePlayerPtr parent);
      */

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
  };

  template <typename StateStorage>
    //class PeriodicSched : public Sched, public std::enable_shared_from_this<PeriodicSched<StateStorage>> {
    class PeriodicSched : public Sched {
      private:
        class PeriodicEvaluator : public Sched, public SchedulePlayerObserver {
          public:
            PeriodicEvaluator(std::weak_ptr<PeriodicSched<StateStorage>> periodic, sched_id_t parent_id) {
              mPeriodic = periodic;
              mParentID = parent_id;
              mState = std::make_shared<StateStorage>();
            }

            virtual void exec(Seq * seq, Parent * parent) {
              std::shared_ptr<PeriodicSched> ref = mPeriodic.lock();
              if (mJumped) {
                parent->remove_observer(this);
                if (ref)
                  ref->exec_jumped(mState, seq, parent);
              } else {

                if (ref && ref->exec_periodic(mState, seq, parent)) {
                  SchedPtr self = shared_from_this();
                  seq->schedule_absolute(ref->tick_period(), self, parent->shared_from_this());
                } else {
                  if (ref)
                    ref->exec_end(mState, seq, parent);
                  parent->remove_observer(this);
                }
              }
            }

            void exec_start(Seq * seq, Parent * parent) const {
              std::shared_ptr<PeriodicSched> ref = mPeriodic.lock();
              if (ref)
                ref->exec_start(mState, seq, parent);
            }

            virtual void location_jumped(SchedulePlayer * /* player */, seq_tick_t /* offset */) {
              //XXX eventually we should check the offset and see if we should actually play through
              mJumped = true;
            }

            sched_id_t parent_id() const { return mParentID; }
          private:
            std::weak_ptr<PeriodicSched> mPeriodic;
            sched_id_t mParentID;
            std::shared_ptr<StateStorage> mState;
            bool mJumped = false;
        };
      public:
        seq_tick_t tick_period() const { return mTickPeriod; }

        virtual void exec(Seq * seq, Parent * parent) {
          //create a copy and schedule an evaluator with that copy
          std::shared_ptr<PeriodicSched<StateStorage>> ref = shared_from_this();
          auto e = std::make_shared<PeriodicEvaluator>(ref, id());

          parent->add_observer(e);
          e->exec_start(seq, parent);
          seq->schedule_absolute(ref->tick_period(), e, parent->shared_from_this());
        }

        virtual void exec_start(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) {} //default, do nothing
        virtual void exec_end(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) {} //default, do nothing

        //true to keep in schedule
        virtual bool exec_periodic(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) = 0;

        //we jumped, do any cleanup we might need to do, exec_end will not be called
        virtual void exec_jumped(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) = 0;

        //couldn't get enable_shared_from_this inherited into this class so just created our own..
        std::shared_ptr<PeriodicSched<StateStorage>> shared_from_this() { return std::static_pointer_cast<PeriodicSched<StateStorage>>(Sched::shared_from_this()); }
      private:
        seq_tick_t mTickPeriod = 1;
    };

  //dangerous, make sure that you use with care..
  template <typename StateStorage>
    class PeriodicSchedFunc : public PeriodicSched<StateStorage> {
      public:
        typedef std::function<bool(p_state_t state, std::shared_ptr<StateStorage> storage, Seq * seq, Sched * owner, Parent * parent)> func_t;

        PeriodicSchedFunc(func_t periodic_func) : mPeriodicFunc(periodic_func) {
        }

        virtual void exec_start(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) {
          mPeriodicFunc(P_START, state_storage, seq, this, parent);
        }

        virtual void exec_end(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) {
          mPeriodicFunc(P_END, state_storage, seq, this, parent);
        }

        virtual bool exec_periodic(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) {
          return mPeriodicFunc(P_PERIODIC, state_storage, seq, this, parent);
        }

        virtual void exec_jumped(std::shared_ptr<StateStorage> state_storage, Seq * seq, Parent * parent) {
          mPeriodicFunc(P_JUMPED, state_storage, seq, this, parent);
        }

      private:
        func_t mPeriodicFunc;
    };
}

#endif
