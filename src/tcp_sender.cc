#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return sequence_numbers_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  while(true){
    TCPSenderMessage msg;
    bool closed = input_.writer().is_closed();
    msg.SYN = (ackno_+sequence_numbers_in_flight_==isn_);
    uint16_t l = min(TCPConfig::MAX_PAYLOAD_SIZE,input_.reader().bytes_buffered());
    uint16_t l_1 = (max(window_size_,(uint16_t)1)-sequence_numbers_in_flight_-msg.SYN);
    l = min(l,l_1);
    msg.FIN = (closed && !FIN_set_ && l==input_.reader().bytes_buffered() && l < l_1);
    if(msg.FIN) FIN_set_ = true;
    msg.payload = input_.reader().peek().substr(0,l);
    msg.seqno = ackno_ + sequence_numbers_in_flight_;
    msg.RST = input_.reader().has_error();
    if(!msg.sequence_length()) break;
    transmit(msg);
    messages_.push(msg);
    input_.reader().pop(l);
    sequence_numbers_in_flight_ += msg.sequence_length();  
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.SYN = msg.FIN = false;
  msg.RST = input_.reader().has_error();
  msg.payload = "";
  msg.seqno = ackno_+sequence_numbers_in_flight_;
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if(msg.RST) input_.writer().set_error();
  if(!msg.ackno.has_value()){
    window_size_ = msg.window_size;
    return;
  }
  if(ackno_ <= msg.ackno.value() && msg.ackno.value() <= ackno_ + sequence_numbers_in_flight_){
    timer_ = initial_RTO_ms_;
    consecutive_retransmissions_ = 0;
    window_size_ = msg.window_size;
    while (!messages_.empty())
    {
      TCPSenderMessage f = messages_.front();
      if(f.seqno+f.sequence_length() <= msg.ackno.value()){
        messages_.pop();
      }
      else{ break; }
    }
    if (ackno_ < msg.ackno.value()) clock_ = 0;
    sequence_numbers_in_flight_ = (ackno_+sequence_numbers_in_flight_).unwrap(msg.ackno.value(),0);
    ackno_ = msg.ackno.value();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  if(!messages_.empty()) clock_ += ms_since_last_tick;
  if(clock_ >= timer_){
    if(window_size_>0) timer_ = timer_ * 2;
    consecutive_retransmissions_ ++;
    clock_ = 0;
    if(!messages_.empty()){
      TCPSenderMessage msg = messages_.front();
      transmit(msg);      
    }
  }
}
