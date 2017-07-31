use std::cmp::Ordering;
use std::collections::BinaryHeap;
use std::sync::Arc;

macro_rules! w {
  ($v:expr) => { Arc::new($v) }
}

type TimePoint = u64;
type Ticks = u64;
type ID = u64;
type SchedFun = Fn(&mut Scheduler);
type SchedFunPtr = Arc<SchedFun>;

trait Scheduler {
  fn schedule(&mut self, t: TimePoint, f: SchedFunPtr) -> ();
  fn now(&self) -> TimePoint;
}

#[derive(Clone)]
struct ScheduleNode {
  time: TimePoint,
  func: SchedFunPtr,
  id: ID
}

impl Ord for ScheduleNode {
  fn cmp(&self, other: &ScheduleNode) -> Ordering {
    other.time.cmp(&self.time)
  }
}

impl PartialOrd for ScheduleNode {
  fn partial_cmp(&self, other: &ScheduleNode) -> Option<Ordering> {
    Some(self.cmp(other))
  }
}

impl PartialEq for ScheduleNode {
  fn eq(&self, other: &ScheduleNode) -> bool {
    self.id == other.id
  }
}
impl Eq for ScheduleNode {}

struct Schedule {
  items: BinaryHeap<ScheduleNode>,
  id: ID
}

struct Seq {
  schedule: Arc<Schedule>,
  now: TimePoint
}

impl Schedule {
  fn new() -> Schedule {
    Schedule{items: BinaryHeap::new(), id:0}
  }

  fn schedule(&mut self, t: TimePoint, f: SchedFunPtr) -> () {
    self.items.push(ScheduleNode{id:self.id, func:f, time:t});
    self.id += 1
  }

  fn item_before(&mut self, t: TimePoint) -> Option<ScheduleNode> {
    match self.items.peek() {
      Some(n) => { 
        if n.time > t {
          return None;
        }
      },
      None => { return None; }
    }
    self.items.pop()
  }

  fn append(&mut self, other: &mut Schedule) {
    while let Some(n) = other.items.pop() {
      self.schedule(n.time, n.func.clone());
    }
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
    let mut context = Seq::new();
    if let Some(s) = Arc::get_mut(&mut self.schedule) {
      //context shares the schedule, should allow new additions to schedule
      for _ in 0..ticks {
        context.now = self.now;
        while let Some(n) = s.item_before(context.now) {
          (n.func)(&mut context);
        }
        self.now += 1;
        if let Some(o) = Arc::get_mut(&mut context.schedule) {
          s.append(o);
        }
      }
    } else {
      println!("ERROR"); 
    }
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
    println!("solo fn {}", context.now());
  }));

  c.schedule(1, w!(move |context: &mut Scheduler| {
    println!("outer fn {}", context.now());
    context.schedule(2, w!(move |context: &mut Scheduler| {
      println!("inner fn {}", context.now());
    }));
  }));

  // tests that the inner function will happen 1 tick later if it is scheduled before 'now'
  c.schedule(30, w!(move |context: &mut Scheduler| {
    let n = context.now();
    println!("outer fn with earlier inner {}", n);
    context.schedule(n - 3, w!(move |context: &mut Scheduler| {
      println!("earier inner {}", context.now());
    }));
  }));

  c.schedule(23, w!(doit));
  c.schedule(34, w!(doit2));
  c.exec(20);
  c.exec(20);
  c.exec(20);
  c.exec(20);
  c.exec(20);
  c.exec(20);
}

