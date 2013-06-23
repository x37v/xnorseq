#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Rando {
  public:
    void exec(xnor::Seq * s, xnor::Parent * parent) {
      cout << "rando exec" << endl;
    }
};

int main(int argc, char * argv[]) {
  xnor::Seq seq;

  {
    auto s = [](xnor::Seq * s, xnor::Parent * p) {
      cout << "start func" << endl;
    };

    auto e = [](xnor::Seq * s, xnor::Parent * p) {
      cout << "end func" << endl;
    };

    xnor::SchedPtr p(new xnor::StartEndSchedFunc(1, s, e));
    seq.schedule(2, p);
    seq.schedule(3, p);
    seq.schedule(4, p);
  }

  seq.schedule(5, [](xnor::Seq *s, xnor::Parent *parent) { s->locate(1); });

  Rando r;
  seq.schedule(3, std::bind(&Rando::exec, r, std::placeholders::_1, std::placeholders::_2));

  auto start = [](xnor::Seq * s, xnor::Parent *p) {
    cout << "start periodic" << endl;
  };

  int count = 0;
  auto pfunc = [&count](xnor::Seq * s, xnor::Parent * p) {
    cout << "periodic: " << count << endl;
    if (count++ > 3)
      return false;
    return true;
  };

  xnor::SchedPtr periodic(new xnor::PeriodicSchedFunc(pfunc, start));
  seq.schedule(1, periodic);

  for (int i = 0; i < 20; i++) {
    cout << i << endl;
    seq.tick();
  }
}
