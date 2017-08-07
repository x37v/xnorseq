mod xnorseq;

use xnorseq::Player;
use xnorseq::Seq;
use xnorseq::alloc;

fn doit(context: &mut Player) {
  println!("doit: {}", context.now());
}

fn doit2(context: &mut Player) {
  let n = context.now();
  println!("doit2: {}", n);
  context.schedule(n + 4, alloc(doit));
}

fn main() {
  let mut c = Seq::new();
  c.schedule(10, alloc(move |context: &mut Player| {
    println!("solo fn {}", context.now());
  }));

  c.schedule(1, alloc(move |context: &mut Player| {
    println!("outer fn {}", context.now());
    context.schedule(2, alloc(move |context: &mut Player| {
      println!("inner fn {}", context.now());
    }));
  }));

  // tests that the inner function will happen 1 tick later if it is scheduled before 'now'
  c.schedule(30, alloc(move |context: &mut Player| {
    let n = context.now();
    println!("outer fn with earlier inner {}", n);
    context.schedule(n - 3, alloc(move |context: &mut Player| {
      let n = context.now();
      println!("earlier inner {}", n);
      context.schedule(n - 3, alloc(move |context: &mut Player| {
        println!("earlier inner 2 {}", context.now());
      }));
    }));
  }));

  c.schedule(23, alloc(doit));
  c.schedule(34, alloc(doit2));

  c.schedule(54, alloc(xnorseq::osc::oscfunc));

  c.exec(20);
  c.exec(20);
  c.exec(20);
  c.exec(20);
  c.exec(20);
  c.exec(20);
}

