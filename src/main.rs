//use std::collections::HashMap;

/*
struct Context<'a> {
  now: u64,
  items: Vec<& 'a Fn(&mut Context)>
}

impl<'a> Context<'a> {
  fn schedule(&mut self, time: u64, func: &Fn(&mut Context)) {
    println!("now: {}, scheduled time: {}", self.now(), time);
    func(self);
  }

  fn now(&self) -> u64 { self.now }

  fn incr(&mut self) -> u64 {
    self.now = self.now + 1;
    self.now
  }
}

fn blah(c: &mut Context) {
  println!("blah: {}", c.now());
}

fn reschedule(c: &mut Context) {
  let n = c.now() + 10;
  c.schedule(n, &blah);
}

fn main() {
  let mut c = Context{now: 0,items:Vec::new()};
  let f = |c: &mut Context| {
    println!("now: {}", c.now());
  };

  c.items.push(blah);
  c.schedule(34, &f);
  c.schedule(35, &f);
  c.schedule(37, &reschedule);
  c.schedule(37, &blah);
  loop {
    if c.incr() > 64 { break; }
  }
}
*/


trait Schedule {
  fn schedule(&mut self, t: u64, f: Box<Fn(&mut Schedule)>) -> ();
  fn now(&self) -> u64;
}

type SeqFun = Box<Fn(&mut Schedule)>;
struct Seq {
  items: Vec<SeqFun>
}

impl Schedule for Seq {
  fn schedule(&mut self, t: u64, f: SeqFun) -> () {
    self.items.push(f);
  }
  fn now(&self) -> u64 {
    0
  }
}

impl Seq {
  fn new() -> Seq {
    Seq{items:Vec::new()}
  }

  fn exec(&self) {
    let mut c = Seq::new();
    for f in &self.items {
      f(&mut c);
    }
  }
}


/*
impl Schedule for RuntimeSeq {
  fn schedule(&mut self, t: u64, f: SeqFun) -> () {
    //self.items.push(f);
  }
  fn now(&self) -> u64 {
    0
  }
}

impl RuntimeSeq {
  fn new() -> RuntimeSeq {
    RuntimeSeq{items:Vec::new()}
  }

  fn exec(&mut self) {
    for f in &self.items {
      f(self);
    }
  }
}

*/

fn doit(context: &mut Schedule) {
  println!("now: {}", context.now());
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
    context.schedule(43, Box::new(|context: &mut Schedule| {
      println!("inner dude");
    }));
    println!("now: {}", context.now());
  }));
  c.exec();
}
