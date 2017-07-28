#pragma once

#include <functional>
#include <memory>

namespace xnorseq {
  class Scheduler;
  class Sequencer;

  typedef uint64_t TimePoint;
  typedef uint64_t ID;
  typedef std::shared_ptr<Sequencer> SequencerPtr;
  typedef std::shared_ptr<Scheduler> SchedulerPtr;
  typedef std::function<void(SchedulerPtr)> SchedFunc;
  typedef std::shared_ptr<SchedFunc> SchedFuncPtr;

  SequencerPtr sequencer();

  class Scheduler {
    public:
      virtual ID schedule(TimePoint t, SchedFunc f) = 0;
      virtual TimePoint now() = 0; 

      // eventually we could call our own allocator
      template<class T, class...Args>
      std::shared_ptr<T> make_sched(Args&&... args) {
        return std::allocate_shared<T>(std::allocator<T>(), std::forward(args)...);
      }
  };

  class Sequencer : public Scheduler {
    public:
      virtual void exec() = 0;
  };

}
