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

  SequencerPtr sequencer();

  class Scheduler {
    public:
      virtual ID schedule(TimePoint t, SchedFunc f) = 0;
      virtual TimePoint now() = 0; 
  };

  class Sequencer : public Scheduler {
    public:
      virtual void exec() = 0;
  };

}
