mod xnorseq;

use std::sync::Arc;
use xnorseq::Player;
use xnorseq::Seq;

macro_rules! w {
  ($v:expr) => { Arc::new($v) }
}

fn doit(context: &mut Player) {
  println!("doit: {}", context.now());
}

fn doit2(context: &mut Player) {
  let n = context.now();
  println!("doit2: {}", n);
  context.schedule(n + 4, w!(doit));
}

fn main() {
  let mut c = Seq::new();
  c.schedule(10, w!(move |context: &mut Player| {
    println!("solo fn {}", context.now());
  }));

  c.schedule(1, w!(move |context: &mut Player| {
    println!("outer fn {}", context.now());
    context.schedule(2, w!(move |context: &mut Player| {
      println!("inner fn {}", context.now());
    }));
  }));

  // tests that the inner function will happen 1 tick later if it is scheduled before 'now'
  c.schedule(30, w!(move |context: &mut Player| {
    let n = context.now();
    println!("outer fn with earlier inner {}", n);
    context.schedule(n - 3, w!(move |context: &mut Player| {
      let n = context.now();
      println!("earlier inner {}", n);
      context.schedule(n - 3, w!(move |context: &mut Player| {
        println!("earlier inner 2 {}", context.now());
      }));
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

