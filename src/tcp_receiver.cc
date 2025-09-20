#include "tcp_receiver.hh"
#include <iostream>
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if(message.RST) reassembler_.reader().set_error();
  if(message.SYN && !ISN_set_){
    zero_point_ = message.seqno;
    ISN_set_ = true;
    ackno_ = message.seqno + 1;
  }
  if(ISN_set_){
    if(message.FIN) FIN_set_ = true;
    uint64_t stream_idx_ = message.seqno.unwrap(zero_point_,reassembler_.writer().bytes_pushed());
    if(stream_idx_==0 && !message.SYN) return;
    stream_idx_ = (stream_idx_ == 0)? 0:(stream_idx_-1);
    reassembler_.insert(stream_idx_,message.payload,message.FIN|message.RST);   
    ackno_ = zero_point_ + 1 + reassembler_.writer().bytes_pushed() + (FIN_set_ && reassembler_.bytes_pending()==0);
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage msg = {};
  msg.RST = reassembler_.writer().has_error();
  msg.window_size = min(reassembler_.writer().available_capacity(),(uint64_t)UINT16_MAX);
  if(ISN_set_) msg.ackno = ackno_;
  return msg;
}
