#pragma once

#include <memory>
#include <optional>
#include <array>

#include "exception.hh"
#include "network_interface.hh"

struct forward_info{
  std::optional<Address> next_hop_;
  size_t interface_num_;
  forward_info(std::optional<Address> next_hop,size_t interface_num):next_hop_(next_hop),
                                                                      interface_num_(interface_num){}
};


class TrieNode{
  public:
    void append(size_t idx,bool is_one){
      children[is_one] = idx;
    }
    bool appended(bool is_one){
      return (children[is_one] != 0);
    }
    size_t child(bool is_one){
      return children[is_one];
    }
    std::optional<forward_info> info(){ return info_; }
    void add_info(forward_info info){ info_ = info; }
  private:
    array<size_t, 2> children {};
    std::optional<forward_info> info_ {};
};

class Trie{
  public:
    Trie():Node_arr_({TrieNode()}){ }
    void insert(  const uint32_t route_prefix,
                  const uint8_t prefix_length,
                  const optional<Address> next_hop,
                  const size_t interface_num );
    std::optional<forward_info> find( const uint32_t dst_ip_numeric );
  private:
    std::vector<TrieNode> Node_arr_;
};

// \brief A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
class Router
{
public:
  // Add an interface to the router
  // \param[in] interface an already-constructed network interface
  // \returns The index of the interface after it has been added to the router
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // Add a route (a forwarding rule)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces
  void route();

private:
  // The router's collection of network interfaces
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};
  Trie router_trie {};
};
