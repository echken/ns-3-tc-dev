/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef HULA_ROUTING_H
#define HULA_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/object.h"
#include "ns3/ipv4.h"

#include <algorithm>
#include <vector>
#include <map>

namespace ns3 {

/* ... */
class Ipv4HulaRouting : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void);
  Ipv4HulaRouting();
  ~Ipv4HulaRouting();

  //processing probe packet: 

private:
  std::map<uint32_t, std::pair<uint32_t, double>> m_hulaPathUtilTable;  // <destTorId, <nextHop, pathUtil>>
  std::map<uint32_t, uigithubnt32_t> m_hulaBestPathTable;   //<flowletId, nextHop>

  struct SwitchLayer{
    uint32_t Tor;
    uint32_t Agg;
    uint32_t Core;
  };

};

}

#endif /* HULA_ROUTING_H */

