/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hula-routing.h"

namespace ns3 {

TypeId Ipv4HulaRouting::GetTypeId()
{
  static TypeId tid = TypeId("ns3::Ipv4HulaRouting")
    .SetParent<Object>()
    .SetGroupName("hula-routing")
    .AddConstructor<Ipv4HulaRouting>()
    .AddAttribute("HulaPathUtilUpdateInterval",
                  "set hula probe frenquency",
                  TimeValue(MicroSeconds(200)),
                  MakeTimeAccessor(&Ipv4HulaRouting::m_pathUpdateIterval),
                  MakeTimeChecker());

  return tid;
}

void Ipv4HulaRouting::Ipv4HulaRouting():
  m_ipv4(0),
  m_flowletTimeOut(MicroSeconds(500)),
  m_pathUpdateIterval(MicroSeconds(100)),
  m_latestPathUtilUpdateTime(0)
{

}

void Ipv4HulaRouting::~Ipv4HulaRouting()
{

}

TypeId Ipv4HulaRouting::GetInstanceTypeId() const
{
  return Ipv4HulaRouting::GetTypeId();
}


void
Ipv4CongaRouting::NotifyInterfaceUp (uint32_t interface)
{
}

void
Ipv4CongaRouting::NotifyInterfaceDown (uint32_t interface)
{
}

void
Ipv4CongaRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4CongaRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4CongaRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit uint) const 
{
}

void Ipv4HulaRouting::SetIpv4(Ptr<Ipv4> ipv4)
{
  m_ipv4 = ipv4;
}

Ptr<Ipv4Route> Ipv4HulaRouting::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &socketError)
{
  NS_LOG_ERROR(this << "l3 routing protocol, can't route to upper layer");
  return 0;
}

bool Ipv4HulaRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<NetDevice> idev,
                                 uni)
