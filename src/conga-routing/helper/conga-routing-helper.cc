/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "conga-routing-helper.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4CongaRoutingHelper");

Ipv4CongaRoutingHelper::Ipv4CongaRoutingHelper() {}

Ipv4CongaRoutingHelper::~Ipv4CongaRoutingHelper() {}

Ipv4CongaRoutingHelper::Ipv4CongaRoutingHelper(const Ipv4CongaRoutingHelper&)
{}

Ipv4CongaRoutingHelper* Ipv4CongaRoutingHelper::Copy(void) const
{
  return new Ipv4CongaRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol> Ipv4CongaRoutingHelper::Create(Ptr<Node> node) const
{
  Ptr<Ipv4CongaRouting> ipv4CongaRouting = CreateObject<Ipv4CongaRouting> ();
  return ipv4CongaRouting;
}

Ptr<Ipv4CongaRouting> Ipv4CongaRoutingHelper::GetIpv4CongaRouting(Ptr<Ipv4> ipv4) const
{
  Ptr<Ipv4RoutingProtocol> ipv4RoutingProtocol = ipv4->GetRoutingProtocol();

  if(DynamicCast<Ipv4CongaRouting> (ipv4RoutingProtocol))
  {
    NS_LOG_LOGIC(this << "found ipv4congarouting ");
    return DynamicCast<Ipv4CongaRouting>(ipv4RoutingProtocol);
  }
  return 0;
}

}

