#include "sequence.h"
#include <iostream>
#include <atomic>

using std::cout;
using std::endl;

class Blah {
  public:
    void operator()(xnorseq::SchedulerPtr p) {
      cout << "ASDFASDF" << endl;
    }
};

int main(int arc, char * argv[]) {
  auto s = xnorseq::sequencer();
  s->schedule(0, [](xnorseq::SchedulerPtr p) {
    cout << "YEAH: " << p->now() << endl;
  });

  {
    auto b = s->make_sched<Blah>();
    s->schedule(23, *b);
  }
  {
    auto x = std::make_shared<std::atomic<xnorseq::TimePoint>>(23);

    xnorseq::SchedFunc inner = [x](xnorseq::SchedulerPtr p) {
      *x += 1;
      cout << "INNER YEAH: " << p->now() << endl;
      cout << "x: " << *x << endl;
    };
    auto outer = [x, inner](xnorseq::SchedulerPtr p) {
      *x += 1;
      cout << "YEAH: " << p->now() << endl;
      cout << "x: " << *x << endl;
      p->schedule(p->now() + 12, inner);
      p->schedule(p->now() + 15, inner);
      p->schedule(p->now() + 18, inner);
      p->schedule(p->now() + 20, inner);
    };
    s->schedule(2, outer);
  }


  s->exec();

  return 0;
}
