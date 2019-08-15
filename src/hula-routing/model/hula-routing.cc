/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hula-routing.h"
#include "hula-probing.h"
#include "hula-tag.h"

#include "ns3/ipv4.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/flow-id-tag.h"
#include "ns3/log.h"

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

uint32_t GetDirection(Ptr<Packet> packet)
{
  Ipv4HulaTag ipv4HulaTag;
  packet->PeekPacketTag(ipv4HulaTag);
  uint32_t previousSwitchRole = ipv4HulaTag.GetSwitchRole();
  uint32_t currentSwitchRole = Ipv4HulaRouting::GetSwitchRole();
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
    pathUtil = (outputLinkUtilItr->second).second;
    outputTraffic = outputTrafficItr->second;

    Time updateTimeUtilNow = Simulator::Now().GetMicroSeconds() - m_latestPathUtilUpdateTime.GetMicroSeconds();
    double utilRatio = updateTimeUtilNow / m_pathUpdateIterval;
    //XXX. only need current packet size
    newPathUtil = outputTraffic + pathUtil * (1 - utilRatio);

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
  uint32_t pathUtil = outputLinkUtilItr.second;

  Time updateTimeUtilNow = Simulator::Now().GetMicroSeconds() - outputLinkUtilItr.first.GetMicroSeconds();
  double utilRatio = updateTimeUtilNow/m_pathUpdateIterval;
  newPathUtil = currentPacketSize + static_cast<uint32_t>(pathUtil * (1 - utilRatio));

  m_outputLinkUtilMap[port] = std::make_pair<Simulator::Now(), newPathUtil>();
}

Ptr<Ipv4Route> Ipv4HulaRouting::ConstructIpv4HulaRoute(uint32_t port, Ipv4Address destAddress)
{
  NS_LOG_LOGIC(this << "construct an route entry");
  Ptr<NetDevice> netdev = m_ipv4->GetNetDevice(port);
  Ptr<Channel> channel = netdev->GetChannel();
  uint32_t peerEnd = (channel->GetDevice(0) == netdev) ? 1 : 0;
  Ptr<Node> nextHop = channel->GetDevice(peerEnd)->GetNode();
  uint32_t nextIf = channel->GetDevice(peerEnd)->GetIfIndex();
  Ipv4Address nextHopAddr = nextHop->GetObject<Ipv4>()->GetAddress(nextIf, 0).GetLocal();

  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetOutputDevice(m_ipv4->GetNetDevice(port));
  route->SetGateway(nextHopAddr);
  route->SetSource(m_ipv4->GetAddress(port, 0)->GetLocal());
  route->SetDestination(destAddress);

  return route;
}

void Ipv4HulaRouting::AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port)
{
  NS_LOG_LOGIC(this << "add ipv4hula route entry");
  Ipv4HulaRouteEntry = ipv4HulaRouteEntry;
  ipv4HulaRouteEntry.network = network;
  ipv4HulaRouteEntry.networkMask = networkMask;
  ipv4HulaRouteEntry.port = port;

  m_hulaRouteEntryList.push_back(ipv4HulaRouteEntry);
}

Ptr<Ipv4Route> Ipv4HulaRouting::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &socketError)
{
  //TODO, tor switch send probe packet periodicaly.
  // datapath: routeoutput -----> routeinput
  if(m_isTor)
  {
    
  }
  return 0;
}

std::vector<uint32_t> Ipv4HulaRouting::LookUpMulticastGroup(Ipv4Address probeAddress)
{
  std::map<Ipv4Address>::iterator probeMulticastItr = m_probeMulticastTable.find(probeAddress);
  return probeMulticastItr->second;
}

bool Ipv4HulaRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<NetDevice> idev,
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
  }
  uint32_t flowId = flowIdTag.GetflowId();

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
    // TODO. how to identify src && dest tor(first or last hop)
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
          }

          Time flowletGap = Simulator::Now().GetMicroSeconds() - flowletInfo->updateTime.GetMicroSeconds();
          if(flowletInfo != NULL
             && flowletGap < m_flowletTimeOut.GetMicroSeconds())
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

          if(flowletInfo != NULL &&
             flowletGap > m_flowletTimeOut.GetMicroSeconds())
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
        uint32_t torId = ipv4HulaTag.GetTorId();
        uint32_t maxPathUtil = ipv4HulaTag.GetMaxPathUtil();
        Ipv4Address probeMulticatAddress = ipv4HulaTag.GetProbeDestAddress();
        // 1> select list of output interface; 
        std::vector<uint32_t> probeInterface = Ipv4HulaRouting::GetProbeMulticastGroup(probeMulticatAddress);
        //TODO. UNICAST route one by one, or multicast route function ??
        
      } // end probe packet 

    } //end src tor 
    else 
    {
      // 1.b). dest tor
      if(!isFoundHulaTag)
      {
        // 1.b).i) dest tor && normal packet
        // TODO normal routing 
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
          (hulaPathUtilItr->second).udpateTime = Simulator::Now();
        }

      }
      else
      {
        // 
      }

      //TODO.  lookup multicast route table for route probe packet out
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
        }

        Time flowletGap = Simulator::Now().GetMicroSeconds() - flowletInfo->updateTime.GetMicroSeconds();
        if(flowletInfo != NULL
           && flowletGap < m_flowletTimeOut.GetMicroSeconds())
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

        if(flowletInfo != NULL &&
           flowletGap > m_flowletTimeOut.GetMicroSeconds())
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

void Ipv4HulaRouting::SetProbeMulticastGroup(Ipv4Address multicastAddress, std::vector<uint32_t> outputInterfaceSet)
{
  std::vector<uint32_t>::iterator outputInterfaceItr = outputInterfaceSet.begin();
  for(; outputInterfaceItr != outputInterfaceSet.end(); outputInterfaceItr++)
  {
    Ipv4HulaRouting::SetProbeMulticastInterface(multicastAddress, *outputInterfaceItr);
  }
}

std::vector<uint32_t> Ipv4HulaRouting::GetProbeMulticastGroup(Ipv4Address multicastAddress)
{
  std::map<Ipv4Address, std::vector<uint32_t>>::iterator probeMulticastItr = m_probeMulticastTable.find(multicastAddress);
  return probeMulticastItr->second;
}

void Ipv4HulaRouting::SetProbeMulticastInterface(Ipv4Address multicastAddress, uint32_t interface)
{
  std::map<Ipv4Address, std::vector<uint32_t>>::iterator probeMulticastItr = m_probeMulticastTable.find(multicastAddress);
  
  probeMulticastItr->push_back(interface);
}

} // end ns3 
