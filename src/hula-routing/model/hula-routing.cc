/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hula-routing.h"
#include "hula-tag.h"

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
                  MakeTimeChecker())
    .AddAttribute("HulapathFailureTimeOut",
                  "set hula path failure detect threshold",
                  TimeValue(MicroSeconds(1000)),
                  MakeTimeAccessor(&Ipv4HulaRouting::m_pathFailureTimeOut),
                  MakeTimeChecker());

  return tid;
}

void Ipv4HulaRouting::Ipv4HulaRouting():
  m_ipv4(0),
  m_isTor(false),
  m_torId(0),
  m_flowletTimeOut(MicroSeconds(500)),
  m_pathFailureTimeOut(MicroSeconds(1000)),
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

void Ipv4HulaRouting::SetTorId(uint32_t torId)
{
  m_isTor = true;
  m_torId = torId;
}

Ptr<Ipv4Route> Ipv4HulaRouting::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &socketError)
{
  NS_LOG_ERROR(this << "l3 routing protocol, can't route to upper layer");
  return 0;
}

bool Ipv4HulaRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<NetDevice> idev,
                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb, LocalDeliverCallback lcb,
                                 ErrorCallback ecb)
{
  Ptr<Packet> packet = ConstCast<Packet>(p);
  Ipv4Address destAddress = header.GetDestination();

  //1. tor switch:
  //  a). src tor
  //      i. send probe packet periodically 
  //     ii. 
  //  b). dst tor
  //      i. receive probe packet, update util table, and drop probe packet  
  //     ii. 
  //2. non-tor switch: 
  //  a). probe packet
  //  b). normal packet
  if(m_isTor)
  {
    //tor switch
    Ipv4HulaTag ipv4HulaTag;
    bool isFoundHulaTag = packet->PeekPacketTag(ipv4HulaTag);
    if(!isFoundHulaTag)
    {
      //src tor switch, start probe
    }
    else
    {
      //dst tor switch, probe terminate
    }

  } //end tor
  else
  {
    //non-tor
    Ipv4HulaTag ipv4HulaTag;
    bool isFoundHulaTag = packet->PeekPacketTag(ipv4HulaTag);

    if(isFoundHulaTag)
    {
      //probe packet  
    }
    else
    {
      //normal packet
    }

  } // end non-tor switch 
  
}
