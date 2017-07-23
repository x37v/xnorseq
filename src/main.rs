use std::sync::Arc;

type TimePoint = u64;

trait Schedule {
  fn schedule(&mut self, t: TimePoint, f: Arc<Fn(&mut Schedule)>) -> ();
  fn now(&self) -> TimePoint;
}

type SeqFun = Arc<Fn(&mut Schedule)>;

struct RuntimeSeq {
  items: Vec<SeqFun>,
  now: TimePoint
}

struct Seq {
  items: Vec<SeqFun>,
  runtime: RuntimeSeq,
  now: TimePoint
}

impl Schedule for Seq {
  fn schedule(&mut self, t: TimePoint, f: SeqFun) -> () {
    self.items.push(f);
  }
  fn now(&self) -> TimePoint {
    self.now
  }
}

impl Schedule for RuntimeSeq {
  fn schedule(&mut self, t: TimePoint, f: SeqFun) -> () {
    self.items.push(f);
  }
  fn now(&self) -> TimePoint {
    self.now
  }
}

impl RuntimeSeq {
  fn new() -> RuntimeSeq {
    RuntimeSeq{items:Vec::new(), now: 0}
  }

  fn exec(&mut self) {
    let mut v = self.items.clone();
    let mut iter = v.iter_mut();
    while let Some(f) = iter.next() {
      f(self);
    }
  }
}

impl Seq {
  fn new() -> Seq {
    Seq{items:Vec::new(), runtime: RuntimeSeq::new(), now: 0}
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
  context.schedule(2000, Arc::new(doit));
}

fn main() {
  let mut c = Seq::new();
  c.schedule(23, Arc::new(doit));
  c.schedule(34, Arc::new(doit2));
  c.schedule(10, Arc::new(|context:  &mut Schedule| {
    println!("outer dude: {}", context.now());
    context.schedule(43, Arc::new(|context: &mut Schedule| {
      println!("inner dude");
    }));
  }));
  c.exec();
}
