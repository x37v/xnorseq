#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Blah : public xnorseq::Event {
  public:
    void exec(xnorseq::timepoint t, xnorseq::ExecContext context) {
      cout << "BLAH " << t << endl;
      context.schedule(context.now() + 1, context.self());
    }
};

class Foo : public xnorseq::Event {
  public:
    void exec(xnorseq::timepoint t, xnorseq::ExecContext context) {
      cout << "FOO " << t << endl;
      //context.schedule(context.now() + 1, context.self());
    }
};

int main(int /*argc*/, char** /*argv*/) {
  xnorseq::Seq seq;

  auto b = std::make_shared<Blah>();
  seq.schedule(32, b);


  auto s = std::make_shared<xnorseq::EventSchedule>();

  auto f = std::make_shared<Foo>();
  s->schedule(std::make_shared<xnorseq::ScheduleItem<xnorseq::EventPtr>>(f, 0));
  s->schedule(std::make_shared<xnorseq::ScheduleItem<xnorseq::EventPtr>>(f, 100));

  auto ss = std::make_shared<xnorseq::StartScheduleEvent>(s);
  seq.schedule(64, ss);

  for (unsigned int i = 0; i < 1024; i+= 64) {
    seq.exec(i);
    cout << "up to " << i << endl;
  }

  return 0;
}

