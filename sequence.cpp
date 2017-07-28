#include "sequence.h"
#include <map>

namespace xnorseq {
  typedef std::map<TimePoint, SchedFunc> smap_t;

  class ScheduleContext : public Scheduler {
    public:
      virtual ID schedule(TimePoint t, SchedFunc f) override {
        mEntries[t] = f;
        return 0;
      }
      virtual TimePoint now() override { return mNow; }
      void now(TimePoint n) { mNow = n; }
      smap_t& entries() { return mEntries; }
    private:
      TimePoint mNow = 0 ;
      smap_t mEntries;
  };
  typedef std::shared_ptr<ScheduleContext> ScheduleContextPtr;

  class SequencerImpl : public Sequencer {
    public:
      SequencerImpl() {
        mContext = std::make_shared<ScheduleContext>();
      }
      virtual ID schedule(TimePoint t, SchedFunc f) override {
        ID id = 0;
        mEntries[t] = f;
        return id;
      }
      virtual void exec() override {
        for (auto& kv: mEntries) {
          TimePoint n = kv.first;
          mContext->now(n);
          kv.second(mContext);
          for (auto& kv: mContext->entries()) {
            if (kv.first < n)
              continue;
            mContext->now(kv.first);
            kv.second(mContext);
          }
        }
      }
      virtual TimePoint now() override { return mNow; }
    private:
      smap_t mEntries;
      ScheduleContextPtr mContext;
      TimePoint mNow = 0 ;
  };

  SequencerPtr sequencer() {
    return std::make_shared<SequencerImpl>();
  }
}
