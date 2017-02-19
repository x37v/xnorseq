#pragma once

#include <boost/any.hpp>
#include <map>
#include <iostream>

namespace xnorseq {
  typedef unsigned int timepoint;
  class Scheduler;

  std::map<unsigned int, boost::any> data;
  unsigned int cID;

  class Callable {
    public:
      virtual ~Callable() {}
      virtual bool call(Scheduler* s, timepoint t) const = 0;
  };

  //crtp https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
  template <typename T, typename D>
    class Executor : public Callable {
      public:
        Executor() { mID = cID++; }
        unsigned int id() { return mID; }

        //override this
        bool exec(Scheduler * /*s*/, timepoint /*t*/, D /*arg*/) const {
          //XXX compile time assert that this is overridden?
          return false;
        }

        virtual bool call(Scheduler* s, timepoint t) const {
          return static_cast<const T*>(this)->exec(s, t, get());
        }

        //XXX need an atomic set/get or 
        void set(D d) { data[mID] = d; }
        D get() const { return boost::any_cast<D>(data[mID]); }

        unsigned int id() const { return mID; }
      private:
        unsigned int mID = 0;
    };

  class Scheduler {
  };

}
