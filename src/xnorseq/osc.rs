extern crate rosc;

use std::net::UdpSocket;
use std::net::ToSocketAddrs;
use std::net::SocketAddrV4;
use std::sync::Arc;
use xnorseq::Player;
use xnorseq::ValOrVFn;
use xnorseq::alloc;
use xnorseq::resolve;

struct OscRouting {
  src: UdpSocket,
  dest: Vec<SocketAddrV4>
}

struct OscMessage {
  addr: ValOrVFn<String>,
  args: Vec<ValOrVFn<rosc::OscType>>,
  routing: Option<OscRouting>
}

impl OscMessage {
  fn new(addr: ValOrVFn<String>, args: Vec<ValOrVFn<rosc::OscType>>) -> OscMessage {
    OscMessage {addr: addr, args: args, routing: None}
  }

  fn new_empty(addr: ValOrVFn<String>) -> OscMessage {
    OscMessage {addr: addr, args: vec![], routing: None}
  }

  fn encode(&self, context: &mut Player) -> rosc::Result<Vec<u8>> {
    let mut args = Vec::new();
    for a in &self.args {
      args.push(resolve(a, context));
    }
    rosc::encoder::encode(&rosc::OscPacket::Message(rosc::OscMessage {
      addr: resolve(&self.addr, context).to_string(),
      args: if args.len() == 0 { None } else { Some(args) }
    }))
  }

  fn send(&self, context: &mut Player) {
    if let Ok(sock) = UdpSocket::bind("127.0.0.1:10022") {
      if let Ok(buf) = self.encode(context) {
        sock.send_to(&buf, "127.0.0.1:10023").unwrap();
      }
    }
  }
}

pub fn oscfunc(context: &mut Player) {
  let f = ValOrVFn::ValFn(alloc(move |context: &mut Player| {
    rosc::OscType::Long(context.now() as i64)
  }));

  let msg = OscMessage::new(ValOrVFn::Val(String::from("/test")), vec![f, ValOrVFn::Val(rosc::OscType::Long(context.now() as i64))]);

  let n = context.now();
  context.schedule(n + 2, alloc(move |context: &mut Player| {
    msg.send(context);
  }));
}
