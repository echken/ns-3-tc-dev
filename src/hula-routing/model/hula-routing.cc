/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hula-routing.h"
#include "hula-probing.h"
#include "hula-tag.h"

#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/flow-id-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4HulaRouting");
NS_OBJECT_ENSURE_REGISTERED(Ipv4HulaRouting);

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

Ipv4HulaRouting::Ipv4HulaRouting():
  m_ipv4(0),
  m_isTor(false),
  m_torId(0),
  m_flowletTimeOut(MicroSeconds(500)),
  m_pathFailureTimeOut(MicroSeconds(1000)),
  m_pathUpdateIterval(MicroSeconds(100)),
  m_latestPathUtilUpdateTime(0)
{

}

Ipv4HulaRouting::~Ipv4HulaRouting()
{

}

TypeId Ipv4HulaRouting::GetInstanceTypeId(void) const
{
  return Ipv4HulaRouting::GetTypeId();
}


void
Ipv4HulaRouting::NotifyInterfaceUp (uint32_t interface)
{
}

void
Ipv4HulaRouting::NotifyInterfaceDown (uint32_t interface)
{
}

void
Ipv4HulaRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4HulaRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4HulaRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit uint) const 
{
}

void Ipv4HulaRouting::SetIpv4(Ptr<Ipv4> ipv4)
{
  m_ipv4 = ipv4;
  NS_ASSERT(m_ipv4->GetNInterfaces()==1 && m_ipv4->GetAddress(0,0).GetLocal() == Ipv4Address("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice(0);

  NS_ASSERT(m_lo != 0);

  Ipv4RoutingTableEntry();

}

void Ipv4HulaRouting::SetTorId(uint32_t torId)
{
  m_isTor = true;
  m_torId = torId;
}

void Ipv4HulaRouting::AddAddressToTorIdMap(Ipv4Address ipv4Address, uint32_t torId)
{
  m_addressToTorIdMap[ipv4Address] = torId; 
}

void Ipv4HulaRouting::SetSwitchRole(SwitchRole role)
{
  m_switchRole = role;
}
uint32_t Ipv4HulaRouting::GetSwitchRole() const
{
  uint32_t role = 0;
  switch(m_switchRole)
  {
  case HULA_NONE:
    //error
    break;
  case HULA_TOR:
    role = 1;
    break;
  case HULA_AGGREGATE:
    role = 2;
    break;
  case HULA_SPINE:
    role = 3;
    break;
  default:
    //error 
    break;
  }
  return role;
}

uint32_t Ipv4HulaRouting::GetDirection(Ptr<Packet> packet)
{
  Ipv4HulaTag ipv4HulaTag;
  packet->PeekPacketTag(ipv4HulaTag);
  uint32_t previousSwitchRole = ipv4HulaTag.GetSwitchRole();
  uint32_t currentSwitchRole = GetSwitchRole();
  uint32_t direction = ((currentSwitchRole - previousSwitchRole) > 0) ? 0 : 1;

  return direction;
}

void Ipv4HulaRouting::SetPathUpdateInterval(Time pathUpdateInterval)
{
  m_pathUpdateIterval = pathUpdateInterval;
}

void Ipv4HulaRouting::SetLatestPathUtilUpdateTime(Time latestUpdateTime)
{
  m_latestPathUtilUpdateTime = latestUpdateTime;
}

uint32_t Ipv4HulaRouting::UpdateOutputTraffic(const Ipv4Header &header, Ptr<Packet> p, uint32_t port)
{
  uint32_t outputTraffic = 0;
  std::map<uint32_t, uint32_t>::iterator outputTrafficItr = m_outputTrafficMap.find(port);
  if(outputTrafficItr != m_outputTrafficMap.end())
  {
    outputTraffic = outputTrafficItr->second;
  }

  uint32_t newOutputTraffic = outputTraffic + p->GetSize() + header.GetSerializedSize();
  m_outputTrafficMap[port] = newOutputTraffic;

  return newOutputTraffic;
}

void Ipv4HulaRouting::UpdateOutputTrafficEvent()
{
  bool idle = true;
  std::map<uint32_t, uint32_t>::iterator outputTrafficItr = m_outputTrafficMap.begin();
  std::map<uint32_t, std::pair<Time, uint32_t>>::iterator outputLinkUtilItr = m_outputLinkUtilMap.begin();
  for(; (outputLinkUtilItr != m_outputLinkUtilMap.end()) && (outputLinkUtilItr != m_outputLinkUtilMap.end())
      ; outputTrafficItr++, outputLinkUtilItr++)
  {
    double pathUtil = (outputLinkUtilItr->second).second;
    double outputTraffic = outputTrafficItr->second;

    int64_t updateTimeUtilNow = Simulator::Now().GetMicroSeconds() - m_latestPathUtilUpdateTime.GetMicroSeconds();
    double utilRatio = updateTimeUtilNow / m_pathUpdateIterval.GetMicroSeconds();
    //XXX. only need current packet size
    double newPathUtil = outputTraffic + pathUtil * (1 - utilRatio);

    if(newPathUtil != 0)
    {
      idle = false;
    }
  }

  if(!idle)
  {
    Simulator::Schedule(m_pathUpdateIterval/4, &Ipv4HulaRouting::UpdateOutputTrafficEvent, this);
  }
  else 
  {
    NS_LOG_LOGIC(this << "UpdateOutputTrafficEvent was not triggered!");
  }
}

void Ipv4HulaRouting::OutputLinkUtilUpdate(const Ipv4Header &header, Ptr<Packet> packet, uint32_t port)
{
  std::map<uint32_t, std::pair<Time, uint32_t>>::iterator outputLinkUtilItr = m_outputLinkUtilMap.find(port);

  uint32_t currentPacketSize = packet->GetSize() + header.GetSerializedSize();
  uint32_t pathUtil = outputLinkUtilItr->second.second;

  int64_t updateTimeUtilNow = Simulator::Now().GetMicroSeconds() - outputLinkUtilItr->second.first.GetMicroSeconds();
  double utilRatio = updateTimeUtilNow/m_pathUpdateIterval.GetMicroSeconds();
  uint32_t newPathUtil = currentPacketSize + static_cast<uint32_t>(pathUtil * (1 - utilRatio));

  m_outputLinkUtilMap[port] = std::pair<Time, uint32_t>(Simulator::Now(), newPathUtil);
  /* m_outputLinkUtilMap[port] = std::make_pair<Simulator::Now(), newPathUtil>(); */
}

Ptr<Ipv4Route> Ipv4HulaRouting::ConstructIpv4HulaRoute(uint32_t port, Ipv4Address destAddress)
{
  NS_LOG_LOGIC("construct an route entry");
  Ptr<NetDevice> netdev = m_ipv4->GetNetDevice(port);
  Ptr<Channel> channel = netdev->GetChannel();
  uint32_t peerEnd = (channel->GetDevice(0) == netdev) ? 1 : 0;
  Ptr<Node> nextHop = channel->GetDevice(peerEnd)->GetNode();
  uint32_t nextIf = channel->GetDevice(peerEnd)->GetIfIndex();
  Ipv4Address nextHopAddr = nextHop->GetObject<Ipv4>()->GetAddress(nextIf, 0).GetLocal();

  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetOutputDevice(m_ipv4->GetNetDevice(port));
  route->SetGateway(nextHopAddr);
  route->SetSource(m_ipv4->GetAddress(port, 0).GetLocal());
  route->SetDestination(destAddress);

  return route;
}

void Ipv4HulaRouting::AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port)
{
  NS_LOG_LOGIC(this << "add ipv4hula route entry");
  Ipv4HulaRouteEntry ipv4HulaRouteEntry;
  ipv4HulaRouteEntry.network = network;
  ipv4HulaRouteEntry.networkMask = networkMask;
  ipv4HulaRouteEntry.port = port;

  m_hulaRouteEntryList.push_back(ipv4HulaRouteEntry);
}

Ptr<Ipv4Route> Ipv4HulaRouting::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &socketError)
{
  //TODO, tor switch send probe packet periodicaly.
  // datapath: routeoutput -----> loopback device ----->  routeinput ----> multicast forwarding
  Ipv4HulaTag ipv4HulaTag;
  bool isFoundHulaTag = p->PeekPacketTag(ipv4HulaTag);

  if(isFoundHulaTag && m_isTor)
  {
    // route to loopback device 
    //or lookup probe multicast route table
    return LoopbackRoute(header, oif);
  }
  return 0;
}

Ptr<Ipv4Route> Ipv4HulaRouting::LoopbackRoute(const Ipv4Header &hdr, Ptr<NetDevice> oif) const
{
  NS_LOG_FUNCTION(this << hdr);
  NS_ASSERT(m_lo != 0);
  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination(hdr.GetDestination());
  //XXX. need be set correct source address 
  rt->SetSource(hdr.GetSource());

  rt->SetGateway(Ipv4Address("127.0.0.1"));
  rt->SetOutputDevice(m_lo);
  return  rt;
}

void Ipv4HulaRouting::AddMulticastRoute(Ipv4Address origin, Ipv4Address group, uint32_t inputInterface, std::vector<uint32_t> outputInterfaces)
{
  NS_LOG_FUNCTION(this << origin << " " << group <<" " << inputInterface << " " << &outputInterfaces);
  Ipv4MulticastRoutingTableEntry *route = new Ipv4MulticastRoutingTableEntry();
  *route = Ipv4MulticastRoutingTableEntry::CreateMulticastRoute(origin, group, inputInterface, outputInterfaces);

  m_multicastRoutes.push_back(route);
}

Ipv4MulticastRoutingTableEntry Ipv4HulaRouting::GetMulticastRoute(uint32_t index) const
{
  NS_LOG_FUNCTION(this << index);
  NS_ASSERT_MSG(index < m_multicastRoutes.size(), "Ipv4HulaRouting::GetMulticastRoute(): index out of range");

  if (index < m_multicastRoutes.size())
  {
    uint32_t tmp = 0;
    for (std::list<Ipv4MulticastRoutingTableEntry *>::const_iterator multicastRoutesItr = m_multicastRoutes.begin(); multicastRoutesItr != 
         m_multicastRoutes.end(); multicastRoutesItr++)
    {
      if(tmp == index)
      {
        return *multicastRoutesItr;
      }
      tmp++;
    }
  }
  return 0;
}

Ptr<Ipv4MulticastRoute> Ipv4HulaRouting::LookupMulticastRoute(Ipv4Address origin, Ipv4Address group, uint32_t interface)
{
  NS_LOG_FUNCTION (this << origin << " " << group << " " << interface);
  Ptr<Ipv4MulticastRoute> mrtentry = 0;

  for (std::list<Ipv4MulticastRoutingTableEntry *>::const_iterator multicastRoutesItr = m_multicastRoutes.begin();
       multicastRoutesItr != m_multicastRoutes.end();
       multicastRoutesItr++) 
    {
      Ipv4MulticastRoutingTableEntry *route = *multicastRoutesItr;
//
// We've been passed an origin address, a multicast group address and an 
// interface index.  We have to decide if the current route in the list is
// a match.
//
// The first case is the restrictive case where the origin, group and index
// matches.
//
      if (origin == route->GetOrigin () && group == route->GetGroup ())
        {
          // Skipping this case (SSM) for now
          NS_LOG_LOGIC ("Found multicast source specific route" << *multicastRoutesItr);
        }
      if (group == route->GetGroup ())
        {
          //TODO when send probe packet at src tor switch, set interface to ipv4::IF_ANY
          if (interface == Ipv4::IF_ANY || 
              interface == route->GetInputInterface ())
            {
              NS_LOG_LOGIC ("Found multicast route" << *multicastRoutesItr);
              mrtentry = Create<Ipv4MulticastRoute> ();
              mrtentry->SetGroup (route->GetGroup ());
              mrtentry->SetOrigin (route->GetOrigin ());
              mrtentry->SetParent (route->GetInputInterface ());
              for (uint32_t j = 0; j < route->GetNOutputInterfaces (); j++)
                {
                  if (route->GetOutputInterface (j))
                    {
                      NS_LOG_LOGIC ("Setting output interface index " << route->GetOutputInterface (j));
                      mrtentry->SetOutputTtl (route->GetOutputInterface (j), Ipv4MulticastRoute::MAX_TTL - 1);
                    }
                }
              return mrtentry;
            }
        }
    }
  return mrtentry;
}

bool Ipv4HulaRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb, LocalDeliverCallback lcb,
                                 ErrorCallback ecb)
{
  Ptr<Packet> packet = ConstCast<Packet>(p);
  Ipv4Address destAddress = header.GetDestination();

  std::map<Ipv4Address, uint32_t>::iterator destTorItr = m_addressToTorIdMap.find(destAddress);
  uint32_t destTorId = destTorItr->second;

  FlowIdTag flowIdTag;
  bool isFoundFlowId;
  isFoundFlowId = packet->PeekPacketTag(flowIdTag);
  if(!isFoundFlowId)
  {
    NS_LOG_ERROR(this << "can't find flowidtag");
    ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
    return false;
  }
  uint32_t flowId = flowIdTag.GetFlowId();

  uint32_t selectedPort = 0;

  //1. tor switch:
  //  a). src tor
  //      i. normal packet 
  //     ii. probe packet
  //  b). dst tor
  //      i. normal packet 
  //     ii. receive probe packet, update util table, and drop probe packet
  //2. non-tor switch: 
  //  a). probe packet
  //  b). normal packet
  
  if(m_isTor)
  {
    Ipv4HulaTag ipv4HulaTag;
    bool isFoundHulaTag = packet->PeekPacketTag(ipv4HulaTag);

    bool isSrcTor = false;
    // TODO. how to identify src && dest tor(first or last hop), TTL
    if(isSrcTor)
    {
      if(!isFoundHulaTag)
      {
        // 1.a).i) src tor && normal packet

        //1. flowlet was found
        //2. flowlet was not found
        struct FlowletInfo *flowletInfo = NULL;
        std::map<uint32_t, struct FlowletInfo*>::iterator bestHopItr = m_hulaBestPathTable.find(flowId);
        if(bestHopItr != m_hulaBestPathTable.end())
        {
          //1. flowlet was found
          
          //  1> flowlet == NULL
          //  2> flowlet != NULL && flowlet was not timeout
          //  3> flowlet != NULL && flowlet was timeout
          flowletInfo = bestHopItr->second;

          if(flowletInfo == NULL)
          {
            //1.1> not the normal case 
            NS_LOG_ERROR(this << "error");
            ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
            return false;
          }
          else
          {
            int64_t flowletGap = Simulator::Now().GetMicroSeconds() - flowletInfo->updateTime.GetMicroSeconds();
            if(flowletGap <= m_flowletTimeOut.GetMicroSeconds())
            {
              //1.2> 
              selectedPort = flowletInfo->interface;
              flowletInfo->updateTime = Simulator::Now();

              //update tx linkUtil metric estimator of selectedPort.
              Ipv4HulaRouting::OutputLinkUtilUpdate(header, packet, selectedPort);
              Ptr<Ipv4Route> route = Ipv4HulaRouting::ConstructIpv4HulaRoute(selectedPort, destAddress);
              ucb(route, packet, header);
              NS_LOG_LOGIC(this << " route packet out");

              return true;
            }
            else
            {
              //1.3> re-select output interface ??
              std::map<uint32_t, HulaPathUtilInfo>::iterator pathUtilItr = m_hulaPathUtilTable.find(destTorId);
              selectedPort = (pathUtilItr->second).interface;

              //switch to current best nexthop
              flowletInfo->interface = selectedPort;
              flowletInfo->updateTime = Simulator::Now();

              Ipv4HulaRouting::OutputLinkUtilUpdate(header, packet, selectedPort);
              Ptr<Ipv4Route> route = Ipv4HulaRouting::ConstructIpv4HulaRoute(selectedPort, destAddress);
              ucb(route, packet, header);
              NS_LOG_LOGIC(this << "route packet out");

              return true;
            }
          }

        } // end flowlet was found 
        else
        {
          // 2. flowlet was not found. 
          std::map<uint32_t, HulaPathUtilInfo>::iterator pathUtilItr = m_hulaPathUtilTable.find(destTorId);
          if(pathUtilItr != m_hulaPathUtilTable.end())
          {
            selectedPort = (pathUtilItr->second).interface; 
          }

          //insert newly arrival flowlet to besthop table
          struct FlowletInfo *flowletInfo = new FlowletInfo;
          flowletInfo->interface = selectedPort;
          flowletInfo->updateTime = Simulator::Now();
          m_hulaBestPathTable[flowId] = flowletInfo;

          Ipv4HulaRouting::OutputLinkUtilUpdate(header, packet, selectedPort);
          Ptr<Ipv4Route> route = Ipv4HulaRouting::ConstructIpv4HulaRoute(selectedPort, destAddress);
          ucb(route, packet, header);
          NS_LOG_LOGIC(this << "route packet out");

          return true;
        }

      } // end 1.a).i) normal packet 
      else
      {
        // 1.a).ii) src tor && probe packet
        if(idev == m_lo && header.GetDestination().IsMulticast())
        {
          //src tor, probe packet. multicast forwarding.
          /* uint32_t torId = ipv4HulaTag.GetTorId(); */
          /* uint32_t maxPathUtil = ipv4HulaTag.GetMaxPathUtil(); */
          
          Ipv4Address probeMulticatAddress = ipv4HulaTag.GetProbeDestAddress();
          //TODO. UNICAST route one by one, or multicast route function ??
          //whether idev == m_lo
          Ptr<Ipv4MulticastRoute> mrtentry = LookupMulticastRoute(header.GetSource(), probeMulticatAddress,  m_ipv4->GetInterfaceForDevice(idev));

          if(mrtentry)
          {
            NS_LOG_LOGIC("multicast forwarding probe packet");
            mcb(mrtentry, packet, header);
            return true;
          }
          else 
          {
            NS_LOG_LOGIC("multicasr route not found ");
            ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
            return false;
          }
        }
        else
        {
          //TODO. non-probe non-normal data packet 
          return false;
        }
      } // end probe packet 

    } //end src tor 
    else 
    {
      // 1.b). dest tor
      if(!isFoundHulaTag)
      {
        // 1.b).i) dest tor && normal packet
        // TODO normal routing 
        return true;
      }
      else
      {
        // 1.b).ii) dest tor && probe packet
        uint32_t currentMaxPathUtil = ipv4HulaTag.GetMaxPathUtil();
        uint32_t torId = ipv4HulaTag.GetTorId();

        std::map<uint32_t, HulaPathUtilInfo>::iterator hulaPathUtilItr = m_hulaPathUtilTable.find(torId);
        uint32_t previousMaxPathUtil = (hulaPathUtilItr->second).pathUtil;
        uint32_t previousBestNextHop = (hulaPathUtilItr->second).interface;

        if(currentMaxPathUtil < previousMaxPathUtil)
        {
          //update hulaPathUtilTable
          
          //get input interface 
          Ptr<Channel> channel = idev->GetChannel();
          uint32_t end = (channel->GetDevice(0) == idev) ? 0 : 1;
          uint32_t inputInterface = channel->GetDevice(end)->GetIfIndex();

          if(previousBestNextHop == inputInterface)
          {
            (hulaPathUtilItr->second).pathUtil = currentMaxPathUtil;
            (hulaPathUtilItr->second).updateTime = Simulator::Now();
          }
          else
          {
            (hulaPathUtilItr->second).interface = inputInterface;
            (hulaPathUtilItr->second).pathUtil = currentMaxPathUtil;
            (hulaPathUtilItr->second).updateTime = Simulator::Now();
          }
        }
        else
        {
          // does not need update best hop
        }
        //TODO drop probe packet in dest tor
        return true;
      }
      
    } //end dst tor

  } //end tor
  else
  {
    // 2. non-tor
    Ipv4HulaTag ipv4HulaTag;
    bool isFoundHulaTag = packet->PeekPacketTag(ipv4HulaTag);

    if(isFoundHulaTag)
    {
      // 2.a). non-tor && probe packet  
      uint32_t currentMaxPathUtil = ipv4HulaTag.GetMaxPathUtil();
      uint32_t torId = ipv4HulaTag.GetTorId();
      std::map<uint32_t, HulaPathUtilInfo>::iterator hulaPathUtilItr = m_hulaPathUtilTable.find(torId);

      uint32_t previousMaxPathUtil = (hulaPathUtilItr->second).pathUtil;
      uint32_t previousBestNextHop = (hulaPathUtilItr->second).interface;

      if(currentMaxPathUtil < previousMaxPathUtil)
      {
        Ptr<Channel> channel = idev->GetChannel();
        uint32_t end = (channel->GetDevice(0) == idev) ? 0 : 1;
        uint32_t inputInterface = channel->GetDevice(end)->GetIfIndex();

        if(inputInterface == previousBestNextHop)
        {
          (hulaPathUtilItr->second).pathUtil = currentMaxPathUtil;
          (hulaPathUtilItr->second).updateTime = Simulator::Now();
        }
        else
        {
          (hulaPathUtilItr->second).interface = inputInterface;
          (hulaPathUtilItr->second).pathUtil = currentMaxPathUtil;
          (hulaPathUtilItr->second).updateTime = Simulator::Now();
        }

      }
      else
      {
        //doesn't need update best path util 
      }

      //TODO.  lookup multicast route table for route probe packet out
      Ptr<Ipv4MulticastRoute> mrtentry = LookupMulticastRoute(header.GetSource(), header.GetDestination(), m_ipv4->GetInterfaceForDevice(idev));
      if(mrtentry)
      {
        NS_LOG_LOGIC(this << "multicast forwarding probe packet on intermediate node");
        mcb(mrtentry, packet, header);
        return true;
      }
      else
      {
        NS_LOG_LOGIC("multicast route not found");
        return false;
      }
    } //end 2.a) non-tor && probe packet
    else
    {
      // 2.b). non-tor && normal packet
      //1. flowlet already found 
      //2. flowlet is not found

      struct FlowletInfo *flowletInfo = NULL;
      std::map<uint32_t, struct FlowletInfo*>::iterator bestHopItr = m_hulaBestPathTable.find(flowId);
      if(bestHopItr != m_hulaBestPathTable.end())
      {
        flowletInfo = bestHopItr->second;

        //1. flowlet was found
        //  1> flowlet == NULL
        //  2> flowlet != NULL && flowlet was not timeout
        //  3> flowlet != NULL && flowlet was timeout
        if(flowletInfo == NULL)
        {
          //1.1> not the normal case 
          NS_LOG_ERROR(this << "error");
          ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
          return false;
        }
        else
        {
          int64_t flowletGap = Simulator::Now().GetMicroSeconds() - flowletInfo->updateTime.GetMicroSeconds();
          if(flowletGap <= m_flowletTimeOut.GetMicroSeconds())
          {
            //1.2> 
            selectedPort = flowletInfo->interface;
            flowletInfo->updateTime = Simulator::Now();

            //update tx linkUtil metric estimator of selectedPort.
            Ipv4HulaRouting::OutputLinkUtilUpdate(header, packet, selectedPort);
            Ptr<Ipv4Route> route = Ipv4HulaRouting::ConstructIpv4HulaRoute(selectedPort, destAddress);
            ucb(route, packet, header);
            NS_LOG_LOGIC(this << " route packet out");

            return true;
          }
          else
          {
            //1.3> re-select output interface ??
            std::map<uint32_t, HulaPathUtilInfo>::iterator pathUtilItr = m_hulaPathUtilTable.find(destTorId);
            selectedPort = (pathUtilItr->second).interface;

            //switch to current best nexthop
            flowletInfo->interface = selectedPort;
            flowletInfo->updateTime = Simulator::Now();

            Ipv4HulaRouting::OutputLinkUtilUpdate(header, packet, selectedPort);
            Ptr<Ipv4Route> route = Ipv4HulaRouting::ConstructIpv4HulaRoute(selectedPort, destAddress);
            ucb(route, packet, header);
            NS_LOG_LOGIC(this << "route packet out");

            return true;
          }
        }
      } // end bestHopItr != m_bestHopTable.end();
      else
      {
        // 2. flowlet was not found. 
        std::map<uint32_t, HulaPathUtilInfo>::iterator pathUtilItr = m_hulaPathUtilTable.find(destTorId);
        if(pathUtilItr != m_hulaPathUtilTable.end())
        {
          selectedPort = (pathUtilItr->second).interface; 
        }

        //insert newly arrival flowlet to besthop table
        struct FlowletInfo *flowletInfo = new FlowletInfo;
        flowletInfo->interface = selectedPort;
        flowletInfo->updateTime = Simulator::Now();
        m_hulaBestPathTable[flowId] = flowletInfo;

        Ipv4HulaRouting::OutputLinkUtilUpdate(header, packet, selectedPort);
        Ptr<Ipv4Route> route = Ipv4HulaRouting::ConstructIpv4HulaRoute(selectedPort, destAddress);
        ucb(route, packet, header);
        NS_LOG_LOGIC(this << "route packet out");

        return true;
      }
    } //end 2.b) non-tor && normal packet 
  } // end non-tor switch 

}

} // end ns3 

/* void Ipv4HulaRouting::SetProbeMulticastGroup(Ipv4Address multicastAddress, std::vector<uint32_t> outputInterfaceSet) */
/* { */
/*   std::vector<uint32_t>::iterator outputInterfaceItr = outputInterfaceSet.begin(); */
/*   for(; outputInterfaceItr != outputInterfaceSet.end(); outputInterfaceItr++) */
/*   { */
/*     Ipv4HulaRouting::SetProbeMulticastInterface(multicastAddress, *outputInterfaceItr); */
/*   } */
/* } */

/* std::vector<uint32_t> Ipv4HulaRouting::GetProbeMulticastGroup(Ipv4Address multicastAddress) */
/* { */
/*   std::map<Ipv4Address, std::vector<uint32_t>>::iterator probeMulticastItr = m_probeMulticastTable.find(multicastAddress); */
/*   return probeMulticastItr->second; */
/* } */

/* void Ipv4HulaRouting::SetProbeMulticastInterface(Ipv4Address multicastAddress, uint32_t interface) */
/* { */
/*   std::map<Ipv4Address, std::vector<uint32_t>>::iterator probeMulticastItr = m_probeMulticastTable.find(multicastAddress); */
  
/*   probeMulticastItr->push_back(interface); */
/* } */

/* std::vector<uint32_t> Ipv4HulaRouting::LookUpMulticastGroup(Ipv4Address probeAddress) */
/* { */
/*   std::map<Ipv4Address, std::vector<uint32_t>>::iterator probeMulticastItr = m_probeMulticastTable.find(probeAddress); */
/*   return probeMulticastItr->second; */
/* } */

