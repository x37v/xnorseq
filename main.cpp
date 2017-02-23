#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Blah : public xnorseq::Executor<Blah, int, int> {
  public:
    Blah(xnorseq::Seq* seq, int d) : xnorseq::Executor<Blah, int, int>(seq, d) {}

    void exec(xnorseq::CallData /*cd*/, int& arg, int id) const {
      cout << "int " << arg++ << endl;
      cout << "instance data " << id << endl;
    }
};

//class FBlah : public xnorseq::Executor<FBlah, float> {
//  public:
//    void exec(xnorseq::CallData /*cd*/, float arg) const {
//      cout << arg << endl;
//    }
//};

int main(int argc, char* argv[]) {
  xnorseq::Seq seq;
  xnorseq::ObjectRef r = seq.make_obj<Blah>(22);
  seq.exec(r);
  seq.exec(r);
  seq.exec(r);

  /*
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
  */

  return 0;
}

