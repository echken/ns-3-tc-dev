/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CONGA_ROUTING_HELPER_H
#define CONGA_ROUTING_HELPER_H

#include "ns3/ipv4-conga-routing.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class Ipv4CongaRoutingHelper : public Ipv4RoutingHelper
{
public:
  Ipv4CongaRoutingHelper();
  ~Ipv4CongaRoutingHelper();

  Ipv4CongaRoutingHelper(const Ipv4CongaRoutingHelper&);

  Ipv4CongaRoutingHelper* Copy(void) const;

  virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;

  Ptr<Ipv4CongaRouting> GetIpv4CongaRouting(Ptr<Ipv4> ipv4) const;
};

}

#endif /* CONGA_ROUTING_HELPER_H */

