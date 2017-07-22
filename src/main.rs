trait Schedule {
  fn schedule(&mut self, t: u64, f: Box<Fn(&mut Schedule)>) -> ();
  fn now(&self) -> u64;
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
  fn schedule(&mut self, t: u64, f: SeqFun) -> () {
    self.items.push(f);
  }
  fn now(&self) -> u64 {
    0
  }
}

impl Schedule for RuntimeSeq {
  fn schedule(&mut self, t: u64, f: SeqFun) -> () {
    self.items.push(f);
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
    let mut c = RuntimeSeq::new();
    for f in &self.items {
      f(&mut c);
    }
    self.items.append(&mut c.items);
  }
}

impl Seq {
  fn new() -> Seq {
    Seq{items:Vec::new(), runtime: RuntimeSeq::new()}
  }

  fn exec(&self) {
    let mut c = RuntimeSeq::new();
    for f in &self.items {
      f(&mut c);
    }
    c.exec();
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
