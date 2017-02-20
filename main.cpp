#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Blah : public xnorseq::Executor<Blah, int> {
  public:
    void exec(xnorseq::CallData /*cd*/, int arg) const {
      cout << "int " << arg++ << endl;
      set(arg);
    }
};

class FBlah : public xnorseq::Executor<FBlah, float> {
  public:
    void exec(xnorseq::CallData /*cd*/, float arg) const {
      cout << arg << endl;
    }
};

int main(int argc, char* argv[]) {
  Blah b;
  b.set(234);
  FBlah f;
  f.set(23.4);

  xnorseq::Callable* e;
  e = &b;

  xnorseq::Scheduler s;
  xnorseq::CallData cd(&s);

  b.call(cd);
  b.call(cd);
  b.call(cd);
  b.call(cd);
  f.call(cd);
  e->call(cd);

  return 0;
};
