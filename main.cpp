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
  auto s = [](xnor::Seq * s, xnor::Parent * p) {
    cout << "start func" << endl;
  };

  auto e = [](xnor::Seq * s, xnor::Parent * p) {
    cout << "end func" << endl;
  };

  {
    xnor::SchedPtr p(new xnor::StartEndSchedFunc(2, s, e));
    seq.schedule(2, p);
  }

  seq.schedule(5, [](xnor::Seq *s, xnor::Parent *parent) { s->locate(2); });

  Rando r;
  seq.schedule(3, std::bind(&Rando::exec, r, std::placeholders::_1, std::placeholders::_2));

  for (int i = 0; i < 20; i++) {
    cout << i << endl;
    if (i == 8)
      seq.clear();
    seq.tick();
  }
}
