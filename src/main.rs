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
        self.now = t;
        f(self);
      }
    }
    self.now = n;
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
        self.runtime.now = t;
        f(&mut self.runtime);
      }
    }
    self.runtime.exec(dur);
    self.now = n;
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

fn midi_note(note: u8, velocity: u8, channel: u8, duration: TimePoint) -> SeqFun {
  w!(move |context: &mut Schedule| {
    let off = w!(|context: &mut Schedule| {
      println!("midi off {}", context.now());
    });
    let o = context.now() + duration;
    println!("midi on {}", context.now());
    context.schedule(o, off);
  })
}

fn main() {
  let mut c = Seq::new();
  c.schedule(23, w!(doit));
  c.schedule(34, w!(doit2));
  c.schedule(10, w!(|context: &mut Schedule| {
    let n = context.now();
    println!("schedule delay: {}", n);
    context.schedule(13 + n, w!(|context: &mut Schedule| {
      println!("delayed {}", context.now());
    }));
  }));

  let dfunc = |_context: &mut Schedule| {
    20
  };

  c.schedule(20, w!(move |context: &mut Schedule| {
    let n = context.now() + dfunc(context);
    println!("schedule delay: {}", n);
    context.schedule(13 + n, w!(|context: &mut Schedule| {
      println!("delayed {}", context.now());
    }));
  }));

  c.schedule(10, midi_note(12, 127, 1, 23));

  loop {
    if c.exec(10) > 1000 {
      break
    }
  }
}
