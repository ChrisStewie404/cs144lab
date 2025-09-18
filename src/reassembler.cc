#include "reassembler.hh"
#include <iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  string new_rs_buffer_ = rs_buffer_;
  string new_rs_marked_ = rs_marked_;
  const uint64_t rs_cap_ = output_.writer().available_capacity();
  uint64_t rs_len_ = rs_buffer_.length();
  const uint64_t d_len_ = data.length();
  for(uint64_t i=0;i<rs_cap_-rs_len_;i++){
    new_rs_buffer_+=' ';
    new_rs_marked_+='0';
  }
  if(first_index<next_idx_+rs_cap_ && first_index+d_len_>next_idx_){
    uint64_t lb=max(first_index,next_idx_),ub=min(first_index+d_len_,next_idx_+rs_cap_);
    for(uint64_t i=lb;i<ub;i++){
      new_rs_buffer_[i-next_idx_]=data[i-first_index];
      new_rs_marked_[i-next_idx_]='1';
    }
  }
  uint64_t cnt=0;
  for(;cnt<rs_cap_ && new_rs_marked_[cnt]=='1';cnt++){}
  output_.writer().push(new_rs_buffer_.substr(0,cnt));
  rs_buffer_ = new_rs_buffer_.substr(cnt);
  rs_marked_ = new_rs_marked_.substr(cnt);
  next_idx_ += cnt;
  rs_len_ = rs_buffer_.length();
  uint64_t new_bytes_pending_ = 0;
  for(uint64_t i=0;i<rs_len_;i++){ if(rs_marked_[i]=='1') new_bytes_pending_++;}
  bytes_pending_ = new_bytes_pending_;
  if(is_last_substring){
    bytes_total_ = first_index + data.length();
    eof_set_ = true;
  }
  if(eof_set_ && next_idx_ == bytes_total_){
    output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return bytes_pending_;
}
