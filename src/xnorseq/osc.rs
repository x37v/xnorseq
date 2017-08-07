extern crate rosc;

use std::net::UdpSocket;
use xnorseq::Player;
use xnorseq::ValueOrValueFn;
use xnorseq::resolve;

struct OscMessage {
  addr: ValueOrValueFn<String>,
  args: Vec<ValueOrValueFn<rosc::OscType>>
}

impl OscMessage {
  fn encode(&self, context: &mut Player) -> rosc::Result<Vec<u8>> {
    rosc::encoder::encode(&rosc::OscPacket::Message(rosc::OscMessage {
      addr: resolve(&self.addr, context).to_string(),
      args: None
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
  let msg = OscMessage{addr: ValueOrValueFn::Value(String::from("/test")), args: vec![]};

  msg.send(context);
  /*
  let sock = UdpSocket::bind("127.0.0.1:10022").expect("BLAH");

  let msg_buf = rosc::encoder::encode(&rosc::OscPacket::Message(rosc::OscMessage {
    addr: "/xnorseq".to_string(),
    args: Some(vec![rosc::OscType::Long(context.now() as i64)]),
  }))
  .unwrap();

  sock.send_to(&msg_buf, "127.0.0.1:10023").unwrap();
  println!("osc fn {}", context.now());
  */
}
