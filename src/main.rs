type TimePoint = u64;

trait Schedule {
  fn schedule(&mut self, t: TimePoint, f: Box<Fn(&mut Schedule)>) -> ();
  fn now(&self) -> TimePoint;
}

type SeqFun = Box<Fn(&mut Schedule)>;

struct RuntimeSeq {
  items: Vec<SeqFun>
}

struct Seq {
  items: Vec<SeqFun>,
  runtime: RuntimeSeq
}

impl Schedule for Seq {
  fn schedule(&mut self, t: TimePoint, f: SeqFun) -> () {
    self.items.push(f);
  }
  fn now(&self) -> TimePoint {
    0
  }
}

impl Schedule for RuntimeSeq {
  fn schedule(&mut self, t: TimePoint, f: SeqFun) -> () {
    self.items.push(f);
  }
  fn now(&self) -> TimePoint {
    0
  }
}

impl RuntimeSeq {
  fn new() -> RuntimeSeq {
    RuntimeSeq{items:Vec::new()}
  }


  fn exec(&mut self) {
    let mut iter = self.items.iter_mut();
    while let Some(f) = iter.next() {
//      f(self);
    }
  }
}

impl Seq {
  fn new() -> Seq {
    Seq{items:Vec::new(), runtime: RuntimeSeq::new()}
  }

  fn exec(&mut self) {
    for f in &self.items {
      f(&mut self.runtime);
    }
    self.runtime.exec();
  }
}

fn doit(context: &mut Schedule) {
  println!("doit: {}", context.now());
}

fn doit2(context: &mut Schedule) {
  println!("doit2: {}", context.now());
  context.schedule(2000, Box::new(doit));
}

fn main() {
  let mut c = Seq::new();
  c.schedule(23, Box::new(doit));
  c.schedule(34, Box::new(doit2));
  c.schedule(10, Box::new(|context:  &mut Schedule| {
    println!("outer dude: {}", context.now());
    context.schedule(43, Box::new(|context: &mut Schedule| {
      println!("inner dude");
    }));
  }));
  c.exec();
}
