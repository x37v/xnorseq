#ifndef XNOR_SEQUENCE_H
#define XNOR_SEQUENCE_H

#include <functional>
#include <map>
#include <list>
#include <memory>

namespace xnor {
  class Sequence;
  class Sched;

  typedef std::function<void(Sequence * seq)> seq_func_t;
  typedef std::shared_ptr<Sched> SchedPtr;
  typedef int seq_tick_t;

  class Sched {
    public:
      virtual void exec(Sequence * seq) = 0;
      virtual ~Sched() {}
  };

  class SchedFunc : public Sched {
    public:
      SchedFunc(seq_func_t func);
      virtual ~SchedFunc() {}
      virtual void exec(Sequence * seq);
    private:
      seq_func_t mFunc;
  };

  class StartEndSched : public Sched, public std::enable_shared_from_this<StartEndSched> {
    public:
      StartEndSched(seq_tick_t end_offset);
      virtual ~StartEndSched() {}
      virtual void exec(Sequence * seq);
      virtual void exec_start(Sequence * seq) = 0;
      virtual void exec_end(Sequence * seq) = 0;
    private:
      seq_tick_t mEndOffset;
  };

  class StartEndSchedFunc : public StartEndSched {
    public:
      StartEndSchedFunc(seq_tick_t end_offset, seq_func_t start_func, seq_func_t end_func);
      virtual ~StartEndSchedFunc() {}
      virtual void exec_start(Sequence * seq);
      virtual void exec_end(Sequence * seq);
    private:
      seq_func_t mStartFunc;
      seq_func_t mEndFunc;
  };

  class Sequence {
    public:
      Sequence();

      //schedule at time location given
      seq_tick_t schedule(seq_tick_t location, SchedPtr sched, bool push_front = false);
      seq_tick_t schedule(seq_tick_t location, seq_func_t func, bool push_front = false);

      //schedule to happen tick_offset or seconds from now, happens even if main schedule jumps
      void schedule_absolute(seq_tick_t tick_offset, SchedPtr sched);
      void schedule_absolute(seq_tick_t tick_offset, seq_func_t func);

      void schedule_absolute(double seconds_from_now, SchedPtr sched);
      void schedule_absolute(double seconds_from_now, seq_func_t func);

      void tick();

      void clear();

      void locate(seq_tick_t location);
      seq_tick_t location() const { return mCurrentLocation; }
    private:
      std::map<seq_tick_t, std::list<SchedPtr> > mSeq;
      std::list<std::pair<seq_tick_t, SchedPtr> > mSeqAbsolute;

      seq_tick_t mCurrentLocation = 0;
      seq_tick_t mTicksAbsolute = 0;
  };
}

#endif
