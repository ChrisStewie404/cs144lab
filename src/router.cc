#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  router_trie.insert(route_prefix,prefix_length,next_hop,interface_num);
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for(auto it: _interfaces){
    std::queue<InternetDatagram>& dgrams = it->datagrams_received();
    while(!dgrams.empty()){
      InternetDatagram dgram = dgrams.front();
      uint32_t dst_ip_numeric = dgram.header.dst;
      std::optional<forward_info> info = router_trie.find(dst_ip_numeric);
      if(info.has_value()){
      if(dgram.header.ttl >= 2){
        dgram.header.ttl--;
        dgram.header.compute_checksum();
          std::optional<Address> op_hop = info->next_hop_;
        Address next_hop = (op_hop.has_value())? op_hop.value():Address::from_ipv4_numeric(dst_ip_numeric);
          interface(info->interface_num_)->send_datagram(dgram,next_hop);
        }
      }
      dgrams.pop();
    }
  }
  
}
void Trie::insert(  const uint32_t route_prefix,
                  const uint8_t prefix_length,
                  const optional<Address> next_hop,
                  const size_t interface_num )
{
  if(prefix_length == 0){
    Node_arr_[0].add_info(forward_info(next_hop,interface_num));
  }
  else{
    size_t sent_idx = 0;
    uint32_t mask = 0x80000000;
    for(int i=0;i<prefix_length;i++){
      bool is_one = (mask & route_prefix);
      size_t child = Node_arr_[sent_idx].child(is_one);
      if(child){ sent_idx = child; }
      else{
        Node_arr_.push_back(TrieNode());
        size_t len = Node_arr_.size();
        Node_arr_[sent_idx].append(len-1,is_one);
        sent_idx = len - 1;
      }
      if(i == prefix_length-1 && !Node_arr_[sent_idx].info().has_value()){
        Node_arr_[sent_idx].add_info(forward_info(next_hop,interface_num));
      }
      mask /= 2;
    }    
  }
}
std::optional<forward_info> Trie::find( const uint32_t dst_ip_numeric )
{
  std::optional<forward_info> found_info = Node_arr_[0].info();
  size_t sent_idx = 0;
  uint32_t mask = 0x80000000;
  while(true){
    bool is_one = (mask & dst_ip_numeric);
    size_t child = Node_arr_[sent_idx].child(is_one);
    if(!child) break;
    if(Node_arr_[child].info().has_value()){ found_info = Node_arr_[child].info(); }
    sent_idx = child;
    mask /= 2;
  }
  return found_info;
}