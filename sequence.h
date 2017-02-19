#pragma once

#include <cinttypes>

namespace xnorseq {
  typedef uint64_t object_id;

  class Object;
  class Event;
  class Segment;

  class Object {
    public:
      Object();
      object_id id() const { return mID; }
    private:
      object_id mID;
  };

  class Event : public Object {
    void execute(Segment * context, T time);
  };

  class Segment : public Object {
    public:
      //execute from time_start up to but not including time_end
      template <typename T>
      void execute(Segment * context, T time_start, T time_end) {
      }

      template <typename T>
      void schedule(T time, Event e) {
      }
    };
}
