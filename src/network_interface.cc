#include <iostream>

#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  uint32_t ip_num = next_hop.ipv4_numeric();
  if(ip_eth_map_.find(ip_num) == ip_eth_map_.end()){
    living_dgram lf_;
    lf_.dgrams_ = {dgram};
    if(living_frames_.find(ip_num) == living_frames_.end()){
      living_frames_.insert({ip_num,lf_});
      EthernetFrame frame = make_frame( ethernet_address_,
                                        ETHERNET_BROADCAST,
                                        EthernetHeader::TYPE_ARP,
                                        serialize(make_arp(ARPMessage::OPCODE_REQUEST, ethernet_address_, ip_address_.ip(), {}, next_hop.ip())));
      transmit(frame);      
    }
    else{
      living_frames_.find(ip_num)->second.dgrams_.push_back(dgram);
    }     
  }
  else{
    EthernetAddress next_hop_eth = ip_eth_map_[ip_num].address_;
    EthernetFrame frame = make_frame(ethernet_address_,next_hop_eth,EthernetHeader::TYPE_IPv4,serialize(dgram));
    transmit(frame);
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  EthernetAddress addr_eth_ = frame.header.dst;
  if(addr_eth_ != ETHERNET_BROADCAST && addr_eth_ != ethernet_address_){ return; }
  auto type = frame.header.type;
  if(type == EthernetHeader::TYPE_IPv4){
    InternetDatagram dgram;
    if(parse(dgram,frame.payload)){
      datagrams_received_.push(dgram);
    }    
  }
  else if(type == EthernetHeader::TYPE_ARP){
      ARPMessage arp;
      EthernetAddress src_eth = frame.header.src;
      if(parse(arp,frame.payload)){
        Address addr_ip_ = Address::from_ipv4_numeric(arp.sender_ip_address);
        if(arp.opcode == ARPMessage::OPCODE_REQUEST && ip_address_.ipv4_numeric() == arp.target_ip_address){
          EthernetFrame rs_frame = make_frame(ethernet_address_,
                                              src_eth,
                                              EthernetHeader::TYPE_ARP,
                                              serialize( make_arp(ARPMessage::OPCODE_REPLY, ethernet_address_, ip_address_.ip(), src_eth, addr_ip_.ip())));
          transmit(rs_frame);
        }
        living_ethernetaddr leth_;
        leth_.address_ = src_eth;
        uint32_t ip_num = addr_ip_.ipv4_numeric();
        ip_eth_map_.insert({ip_num,leth_});
        if(living_frames_.find(ip_num) != living_frames_.end()){
          for(auto dgram: living_frames_.find(ip_num)->second.dgrams_){
            EthernetFrame pend_frame = make_frame(ethernet_address_,
                                                  src_eth,
                                                  EthernetHeader::TYPE_IPv4,
                                                  serialize(dgram));
            transmit(pend_frame);
          }
          living_frames_.erase(ip_num);
        }
      }
  }  
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  for(auto &pair: living_frames_){
    pair.second.time_ += ms_since_last_tick;
    if(pair.second.time_ >= 5000){
      pair.second.time_ = 0;
      EthernetFrame frame = make_frame( ethernet_address_,
                                        ETHERNET_BROADCAST,
                                        EthernetHeader::TYPE_ARP,
                                        serialize(make_arp(ARPMessage::OPCODE_REQUEST,ethernet_address_, ip_address_.ip(), {}, Address::from_ipv4_numeric(pair.first).ip())));
      transmit(frame);
    }
  }
  for(auto it = ip_eth_map_.begin(); it != ip_eth_map_.end();){
    it->second.time_ += ms_since_last_tick;
    if(it->second.time_ >= 30000){
      it = ip_eth_map_.erase(it);
    }else{
      it++;
    }
  }
}
