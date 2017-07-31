use std::sync::Arc;

macro_rules! w {
  ($v:expr) => { Arc::new($v) }
}

type TimePoint = u64;
type Ticks = u64;
type SchedFun = Fn(&mut Scheduler);
type SchedFunPtr = Arc<SchedFun>;

trait Scheduler {
  fn schedule(&mut self, t: TimePoint, f: SchedFunPtr) -> ();
  fn now(&self) -> TimePoint;
}

struct Schedule {
  items: Vec<(TimePoint, SchedFunPtr)>
}

struct Seq {
  schedule: Arc<Schedule>,
  now: TimePoint
}

impl Schedule {
  fn new() -> Schedule {
    Schedule{items: Vec::new()}
  }

  fn schedule(&mut self, t: TimePoint, f: SchedFunPtr) -> () {
    self.items.push((t, f));
  }

  fn items_at(&self, t: TimePoint) -> Vec<SchedFunPtr> {
    let mut v = Vec::new();
    for &(ref ft, ref f) in &self.items {
      if t == *ft {
        v.push(f.clone())
      }
    }
    v
  }
}


impl Scheduler for Seq {
  fn schedule(&mut self, t: TimePoint, f: SchedFunPtr) -> () {
    match Arc::get_mut(&mut self.schedule) {
      Some(s) => { s.schedule(t, f); }
      None => { println!("ERROR"); }
    };
  }
  
  fn now(&self) -> TimePoint {
    self.now
  }
}

impl Seq {
  fn new() -> Seq {
    Seq{schedule:Arc::new(Schedule::new()), now: 0}
  }

  fn exec(&mut self, ticks: Ticks) -> () {
    //context shares the schedule, should allow new additions to schedule
    let mut context = Seq{now:self.now, schedule: self.schedule.clone()};
    for _ in 0..ticks {
      let mut v = self.schedule.items_at(self.now);
      context.now = self.now;
      for f in v.iter_mut() {
        f(&mut context);
      }
      self.now += 1
    };
  }
}

fn doit(context: &mut Scheduler) {
  println!("doit: {}", context.now());
}

fn doit2(context: &mut Scheduler) {
  let n = context.now();
  println!("doit2: {}", n);
  context.schedule(n + 4, w!(doit));
}


fn main() {
  let mut c = Seq::new();
  c.schedule(10, w!(move |context: &mut Scheduler| {
    println!("moved fn {}", context.now());
  }));

  c.schedule(23, w!(doit));
  c.schedule(34, w!(doit2));
  c.exec(20);
  c.exec(20);
}

