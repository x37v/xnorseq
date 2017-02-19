#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Blah : public xnorseq::Executor<Blah, int> {
  public:
    bool exec(xnorseq::Scheduler *s, xnorseq::timepoint t, int arg) const {
      cout << "int " << arg++ << endl;
      return true;
    }
};

class FBlah : public xnorseq::Executor<FBlah, float> {
  public:
    bool exec(xnorseq::Scheduler *s, xnorseq::timepoint t, float arg) const {
      cout << arg << endl;
      return true;
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
  xnorseq::timepoint t = 0;

  b.call(&s, t);
  b.call(&s, t);
  b.call(&s, t);
  b.call(&s, t);
  f.call(&s, t);
  e->call(&s, t);

  return 0;
};
