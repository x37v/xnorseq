use std::cmp::Ordering;
use std::collections::BinaryHeap;
use std::sync::Arc;

pub mod osc;

pub type TimePoint = u64;
pub type Ticks = u64;
pub type SFn = Fn(&mut Player);
pub type SFnPtr = Arc<SFn>;

type ID = u64;

pub trait Player {
  fn schedule(&mut self, t: TimePoint, f: SFnPtr) -> ();
  fn now(&self) -> TimePoint;
}

pub fn alloc<T>(v: T) -> Arc<T> {
  Arc::new(v)
}

#[allow(dead_code)]
pub enum ValueOrValueFn<T: Clone> {
  Value(T),
  ValueFn(Arc<Fn(&mut Player) -> T>)
}

#[allow(dead_code)]
pub fn resolve<T: Clone>(v: &ValueOrValueFn<T>, player: &mut Player) -> T {
  match v {
    &ValueOrValueFn::Value(ref v) => { v.clone() },
    &ValueOrValueFn::ValueFn(ref f) => { f(player) }
  }
}

#[derive(Clone)]
struct SNode {
  time: TimePoint,
  func: SFnPtr,
  id: ID
}

impl Ord for SNode {
  fn cmp(&self, other: &SNode) -> Ordering {
    other.time.cmp(&self.time)
  }
}

impl PartialOrd for SNode {
  fn partial_cmp(&self, other: &SNode) -> Option<Ordering> {
    Some(self.cmp(other))
  }
}

impl PartialEq for SNode {
  fn eq(&self, other: &SNode) -> bool {
    self.id == other.id
  }
}

impl Eq for SNode {}

struct Schedule {
  items: BinaryHeap<SNode>,
  id: ID
}

pub struct Seq {
  schedule: Arc<Schedule>,
  now: TimePoint
}

impl Schedule {
  fn new() -> Schedule {
    Schedule{items: BinaryHeap::new(), id:0}
  }

  fn schedule(&mut self, t: TimePoint, f: SFnPtr) -> () {
    self.items.push(SNode{id:self.id, func:f, time:t});
    self.id += 1
  }

  fn item_before(&mut self, t: TimePoint) -> Option<SNode> {
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
    //could use self.items.append(&mut other.items); but the ids would conflict
  }
}

impl Player for Seq {
  fn schedule(&mut self, t: TimePoint, f: SFnPtr) -> () {
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
  pub fn new() -> Seq {
    Seq{schedule:Arc::new(Schedule::new()), now: 0}
  }

  pub fn exec(&mut self, ticks: Ticks) -> () {
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
