#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Blah : public xnorseq::EphemeralEvent {
  public:
    void exec(xnorseq::timepoint t, xnorseq::ExecContext context) {
      cout << "BLAH " << t << endl;
      context.schedule(context.now(), context.self());
    }
};

int main(int /*argc*/, char** /*argv*/) {
  xnorseq::Seq seq;

  auto b = std::make_shared<Blah>();
  seq.schedule(32, b);

  for (unsigned int i = 0; i < 1024; i+= 64) {
    seq.exec(i);
    cout << "up to " << i << endl;
  }

  return 0;
}

