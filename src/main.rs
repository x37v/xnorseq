use std::sync::Arc;

macro_rules! w {
  ($v:expr) => { Arc::new($v) }
}

type TimePoint = u64;

trait Schedule {
  fn schedule(&mut self, t: TimePoint, f: Arc<Fn(&mut Schedule)>) -> ();
  fn now(&self) -> TimePoint;
  fn last(&self) -> TimePoint;
}

type SeqFun = Arc<Fn(&mut Schedule)>;

struct RuntimeSeq {
  items: Vec<(TimePoint, SeqFun)>,
  now: TimePoint,
  last: TimePoint
}

struct Seq {
  items: Vec<(TimePoint, SeqFun)>,
  runtime: RuntimeSeq,
  now: TimePoint,
  last: TimePoint
}

impl Schedule for Seq {
  fn schedule(&mut self, t: TimePoint, f: SeqFun) -> () {
    self.items.push((t, f));
  }
  fn now(&self) -> TimePoint {
    self.now
  }
  fn last(&self) -> TimePoint {
    self.last
  }
}

impl Schedule for RuntimeSeq {
  fn schedule(&mut self, t: TimePoint, f: SeqFun) -> () {
    self.items.push((t, f));
  }
  fn now(&self) -> TimePoint {
    self.now
  }
  fn last(&self) -> TimePoint {
    self.last
  }
}

impl RuntimeSeq {
  fn new() -> RuntimeSeq {
    RuntimeSeq{items:Vec::new(), now: 0, last: 0}
  }

  fn exec(&mut self, dur: TimePoint) {
    let n = self.now() + dur;
    let l = self.last();
    self.now = n;
    let mut v = self.items.clone();
    let mut iter = v.iter_mut();
    while let Some(&mut (ref t, ref f)) = iter.next() {
      let t = *t;
      if t < n && t > l {
        f(self);
      }
    }
    self.last = n;
  }
}

impl Seq {
  fn new() -> Seq {
    Seq{items:Vec::new(), runtime: RuntimeSeq::new(), now: 0, last: 0}
  }

  fn exec(&mut self, dur: TimePoint) -> TimePoint {
    let n = self.last() + dur;
    let l = self.last();
    self.now = n;
    for &(ref t, ref f) in &self.items {
      let t = *t;
      if t < n && t >= l {
        f(&mut self.runtime);
      }
    }
    self.runtime.exec(dur);
    self.last = n;
    n
  }
}

fn doit(context: &mut Schedule) {
  println!("doit: {}", context.now());
}

fn doit2(context: &mut Schedule) {
  println!("doit2: {}", context.now());
  context.schedule(2000, w!(doit));
}

fn main() {
  let mut c = Seq::new();
  c.schedule(23, w!(doit));
  c.schedule(34, w!(doit2));
  c.schedule(10, w!(|context:  &mut Schedule| {
    println!("outer dude: {}", context.now());
    context.schedule(43, w!(|context: &mut Schedule| {
      println!("inner dude {}", context.now());
    }));
  }));
  loop {
    if c.exec(10) > 1000 {
      break
    }
  }
}
