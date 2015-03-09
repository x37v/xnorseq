#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Rando {
  public:
    void exec(xnor::Seq * s, xnor::Sched * owner, xnor::Parent * parent) {
      cout << "rando exec" << endl;
    }
};

int main(int argc, char * argv[]) {
  std::shared_ptr<xnor::Seq> seq(new xnor::Seq);

  /*
  xnor::GroupPtr group = std::make_shared<xnor::Group>();
  {
    auto f = [](xnor::start_end_t state, xnor::Seq * s, xnor::Parent * p) {
      if (state == xnor::START)
        cout << "start" << endl;
      else
        cout << "end" << endl;
    };

    xnor::SchedPtr p = std::make_shared<xnor::StartEndSchedFunc>(1, f);
    group->schedule(0, p);

    //XXX why don't these happen?
    p = std::make_shared<xnor::StartEndSchedFunc>(3, f);
    group->schedule(2, p);
    group->schedule(5, p);
  }

  xnor::SchedPtr s = group;
  seq->schedule(0, s);
  */

  seq->schedule(15, [](xnor::Seq *s, xnor::Sched * owner, xnor::Parent *parent) { s->locate(1); });

  Rando r;
  seq->schedule(3, std::bind(&Rando::exec, r, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  /*
  auto start = [](xnor::Seq * s, xnor::Parent *p) {
    cout << "start periodic" << endl;
  };

  auto end = [](xnor::Seq * s, xnor::Parent *p) {
    cout << "end periodic" << endl;
  };

  int count = 0;
  auto pfunc = [&count](xnor::Seq * s, xnor::Parent * p) -> bool {
    cout << "periodic: " << count << endl;
    if (count++ > 3)
      return false;
    return true;
  };

  */

  {
    auto pfunc = [](xnor::p_state_t state, std::shared_ptr<int> count, xnor::Seq * s, xnor::Sched * o, xnor::Parent * p) -> bool {
      switch (state) {
        case xnor::P_END:
          cout << "periodic " << count << ": end" << endl;
          return false; //ignored
        case xnor::P_START:
          *count = 0;
          cout << "periodic " << count << ": start" << endl;
        default:
          cout << "periodic " << count << ": " << *count << endl;
          if ((*count)++ > 3)
            return false;
          break;
      }
      return true;
    };

    xnor::SchedPtr periodic = std::make_shared<xnor::PeriodicSchedFunc<int>>(pfunc);
    seq->schedule(1, periodic);
  }

  for (int i = 0; i < 30; i++) {
    cout << i << " tick: " << seq->location() << endl;
    seq->tick();
  }
}
