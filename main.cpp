#include "sequence.h"
#include <iostream>

using std::cout;
using std::endl;

class Blah : public xnorseq::Event {
  public:
    void exec(xnorseq::timepoint t, xnorseq::ExecContext context) {
      cout << "BLAH " << t << endl;
      context.schedule(context.now() + 10, context.self());
    }
};

class Baz : public xnorseq::Event {
  public:
    void exec(xnorseq::timepoint t, xnorseq::ExecContext context) {
      cout << "Baz " << t << endl;
    }
};

class Foo : public xnorseq::Event {
  public:
    void exec(xnorseq::timepoint t, xnorseq::ExecContext context) {
      cout << "FOO " << t << endl;
      auto e = std::make_shared<Baz>();
      context.schedule(t, e);
    }
};

int main(int /*argc*/, char** /*argv*/) {
  xnorseq::Seq seq;

  auto b = std::make_shared<Blah>();
  seq.schedule(32, b);


  auto s = std::make_shared<xnorseq::EventSchedule>();

  auto f = std::make_shared<Foo>();
  s->schedule(std::make_shared<xnorseq::ScheduleItem<xnorseq::EventPtr>>(f, 0));
  s->schedule(std::make_shared<xnorseq::ScheduleItem<xnorseq::EventPtr>>(f, 10));

  auto ss = std::make_shared<xnorseq::StartScheduleEvent>(s);
  seq.schedule(1, ss);

  ss = std::make_shared<xnorseq::StartScheduleEvent>(s);
  seq.schedule(256, ss);

  auto sub = std::make_shared<xnorseq::EventSchedule>();
  sub->schedule(std::make_shared<xnorseq::ScheduleItem<xnorseq::EventPtr>>(f, 1));

  auto sss = std::make_shared<xnorseq::StartScheduleEvent>(sub);
  s->schedule(std::make_shared<xnorseq::ScheduleItem<xnorseq::EventPtr>>(sss, 100));

  for (unsigned int i = 0; i < 1024; i+= 64) {
    seq.exec(i, 64);
    cout << "up to " << i << endl;
  }

  return 0;
}

