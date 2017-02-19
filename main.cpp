#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;
using namespace xnorseq;

int main(int argc, char * argv[]) {
  Segment seg;

  Event e;
  Event e2;
  seg.schedule(2, e);
  seg.schedule(32, e2);
  seg.execute(nullptr, 23, 25);

  cout << e.id() << endl;
  cout << e2.id() << endl;
  cout << seg.id() << endl;

  return 0;
}
