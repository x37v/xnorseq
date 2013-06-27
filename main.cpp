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
  std::shared_ptr<xnor::Seq> seq(new xnor::Seq);

  xnor::GroupPtr group = std::make_shared<xnor::Group>();
  {
    auto s = [](xnor::Seq * s, xnor::Parent * p) {
      cout << "start func" << endl;
    };

    auto e = [](xnor::Seq * s, xnor::Parent * p) {
      cout << "end func" << endl;
    };

    xnor::SchedPtr p = std::make_shared<xnor::StartEndSchedFunc>(1, s, e);
    group->schedule(0, p);

    p = std::make_shared<xnor::StartEndSchedFunc>(3, s, e);
    group->schedule(2, p);
  }

  xnor::SchedPtr s = group;
  seq->schedule(0, s);

  seq->schedule(15, [](xnor::Seq *s, xnor::Parent *parent) { s->locate(1); });

  Rando r;
  seq->schedule(3, std::bind(&Rando::exec, r, std::placeholders::_1, std::placeholders::_2));

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

  xnor::SchedPtr periodic = std::make_shared<xnor::PeriodicSchedFunc>(pfunc, start, end);
  seq->schedule(1, periodic);
  */

  for (int i = 0; i < 20; i++) {
    cout << i << endl;
    seq->tick();
  }
}
