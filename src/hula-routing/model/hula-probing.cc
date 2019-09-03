#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/packet.h"
#include "ns3/ipv4-raw-socket-factory.h"
#include "ns3/ipv4-header.h"
#include "ns3/boolean.h"

#include "hula-probing.h"
#include "hula-tag.h"

namespace  ns3{
NS_LOG_COMPONENT_DEFINE("Ipv4HulaProbing");
NS_OBJECT_ENSURE_REGISTERED(Ipv4HulaProbing);

TypeId Ipv4HulaProbing::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Ipv4HulaProbing")
    .SetParent<Object>()
    .SetGroupName("hula-routing")
    .AddConstructor<Ipv4HulaProbing>()
    .AddAttribute("ProbeInterval", 
                  "set probe frequency",
                  TimeValue(MicroSeconds(100)),
                  MakeTimeAccessor(&Ipv4HulaProbing::m_probeInterval),
                  MakeTimeChecker());

  return tid;
}

Ipv4HulaProbing::Ipv4HulaProbing()
  :m_probeSrcAddress(Ipv4Address("127.0.0.1")),
   m_probeDstAddress(Ipv4Address("127.0.0.1")),
   m_probeInterval(MicroSeconds(1000)),
   m_torId(0),
   m_node()
{
  NS_LOG_FUNCTION(this);
}

Ipv4HulaProbing::~Ipv4HulaProbing()
{
  NS_LOG_FUNCTION(this);
}

TypeId Ipv4HulaProbing::GetInstanceTypeId() const
{
  return Ipv4HulaTag::GetTypeId();
}

void Ipv4HulaProbing::SetSrcAddress(Ipv4Address address)
{
  m_probeSrcAddress = address;
}

void Ipv4HulaProbing::SetDstAddress(Ipv4Address address)
{
  m_probeDstAddress = address;
}

void Ipv4HulaProbing::StopProbe(Time stopTimer)
{
  Simulator::Schedule(stopTimer, &Ipv4HulaProbing::DoStopProbe, this);
}

void Ipv4HulaProbing::StartProbe()
{
  m_socket = m_node->GetObject<Ipv4RawSocketFactory>()->CreateSocket();
  m_socket->SetRecvCallback(MakeCallback(&Ipv4HulaProbing::ReceivePacket, this));
  m_socket->Bind(InetSocketAddress(Ipv4Address("0.0.0.0"), 0));
  m_socket->SetAttribute("IpHeaderInclude", BooleanValue(true));

  m_probeEvent = Simulator::ScheduleNow(&Ipv4HulaProbing::DoStartProbe, this); 
}

void Ipv4HulaProbing::DoStopProbe()
{
  m_probeEvent.Cancel();
}

void Ipv4HulaProbing::DoStartProbe()
{
  //for all avaliable output interface of tor switch,  send probing packet periodically.

  //TODO 
  //1. server/hypervisor probe, or
  //2. tor switch probe
  /* uint32_t interfaceCount = 32; */
  /* for(uint32_t i = 0; i < interfaceCount; i++) */
  /* { */
  /*   Ipv4HulaProbing::SendProbe(i); */
  /* } */
  Ipv4HulaProbing::SendProbe();

  m_probeEvent = Simulator::Schedule(m_probeInterval, &Ipv4HulaProbing::DoStartProbe, this);
}

void Ipv4HulaProbing::SendProbe()
{
  //construct probe packet
  Address toAddress = InetSocketAddress(m_probeDstAddress, 0);

  Ptr<Packet> packet = Create<Packet>(0);
  Ipv4Header newheader;
  newheader.SetSource(m_probeSrcAddress);
  newheader.SetDestination(m_probeDstAddress);
  newheader.SetProtocol(0);   // TODO. should we set protocol fileds to zero>>>?
  newheader.SetPayloadSize(packet->GetSize());
  newheader.SetTtl(255);

  packet->AddHeader(newheader);
  
  //add packet tag 
  Ipv4HulaTag ipv4HulaTag;
  //FIXME. should set outputinterface while send out of specific interface.
  //initialize with 0 ???
  /* ipv4HulaTag.SetOutputInterface(interface); */
  ipv4HulaTag.SetMaxPathUtil(0);
  ipv4HulaTag.SetDirection(0);
  ipv4HulaTag.SetProbeDestAddress(m_probeDstAddress);

  packet->AddPacketTag(ipv4HulaTag);

  m_socket->SendTo(packet, 0, toAddress);
}

void Ipv4HulaProbing::ReceivePacket(Ptr<Socket> socket)
{
  //dst tor receive probe packet, whether further processing is need??
  //TODO if a switch doesn't receive a probe packet from neighboring switches for more than a certain threshold of time.
  //update maxPathUtilTable for that hop to max_util, making sure that hop is not chosen as the best hop for any destination tor.
  //
  //timeout
}

} // end ns3
