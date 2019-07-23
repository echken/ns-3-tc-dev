/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ipv4-conga-routing.h"
#include "ipv4-conga-tag.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/channel.h"

#include "ns3/flow-id-tag.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4CongaRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4CongaRouting);

Ipv4CongaRouting::Ipv4CongaRouting():
  m_ipv4(0),
  m_Tdre(MicroSeconds(160)),
  m_alpha(0.2),
  m_Qbit(3),
  m_LinkCapacity(DataRate("10Gbps")),
  m_updateResidualTrafficEvent(),
  m_isLeaf(false),
  m_leafId(0),
  m_flowletTimeOut(100),
  m_congaTableAgingEvent(),
  m_congaTableAgingTime(10)
{
  NS_LOG_FUNCTION(this);
}

Ipv4CongaRouting::~Ipv4CongaRouting()
{
  NS_LOG_FUNCTION(this);
}

TypeId Ipv4CongaRouting::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Ipv4CongaRouting")
    .SetParent<Object>()
    .SetGroupName("Internet")
    .AddConstructor<Ipv4CongaRouting>();
  return tid;
}

void Ipv4CongaRouting::SetTdre(Time tdre)
{
  m_Tdre = tdre;
}

void Ipv4CongaRouting::SetAlpha(double alpha)
{
  m_alpha = alpha;
}

void Ipv4CongaRouting::SetQbit(uint32_t q)
{
  m_Qbit = q;
}

void Ipv4CongaRouting::SetLinkCapacity(DataRate datarate)
{
  m_LinkCapacity = datarate;
}

void Ipv4CongaRouting::SetLinkCapacity(uint32_t interface, DataRate datarate)
{
  m_InterfaceLinkCapacity[interface] = datarate;
}

uint32_t Ipv4CongaRouting::UpdateInputTraffic(const Ipv4Header &header, Ptr<Packet> packet, uint32_t port)
{
  uint32_t inputTraffic = 0;
  std::map<uint32_t, uint32_t>::iterator inputTrafficIterator = m_InputTrafficMap.find(port);
  if(inputTrafficIterator != m_InputTrafficMap.end())
  {
    inputTraffic = inputTrafficIterator->second;
  }

  uint32_t newinputTraffic = inputTraffic + packet->GetSize() + header.GetSerializedSize();
  /* newinputTraffic = newinputTraffic * (1 - m_alpha); */
  m_InputTrafficMap[port] = newinputTraffic;
  NS_LOG_LOGIC(this << "update local interface dre parameter, m_InputTrafficMap");

  return newinputTraffic;
}

/* void Ipv4CongaRouting::UpdateResidualTrafficEvent() */
/* { */
/*   std::map<uint32_t, uint32_t>::iterator itr = m_InputTrafficMap.begin(); */
/*   for(; itr != m_InputTrafficMap.end(), ++itr) */
/*   { */
/*     uint32_t newResidualTraffic = itr->second * (1 - alpha); */
/*     itr.second = newResidualTraffic; */
/*   } */
/*   NS_LOG_LOGIC(this << "update dre periodicaly") */

/*   m_updateResidualTrafficEvent = Simulator::Schedule(m_Tdre, Ipv4CongaRouting::UpdateResidualTrafficEvent, this); */
/* } */

uint32_t Ipv4CongaRouting::QuantilizeDreMetric(uint32_t interface, uint32_t residualTraffic)
{
  DataRate c = 0;
  std::map<uint32_t, DataRate>::iterator itr = m_InterfaceLinkCapacity.find(interface);
  if(itr != m_InterfaceLinkCapacity.end())
  {
    c = itr->second;
  }
  else 
  {
    c = m_LinkCapacity;
  }

  double utilization = static_cast<double>(residualTraffic * 8) / (c.GetBitRate() * m_Tdre.GetSeconds() / m_alpha);
  uint32_t ce = static_cast<uint32_t>(utilization * std::pow(2, m_Qbit));
  NS_LOG_LOGIC(this << "quantilize dre metric to Q bit ce singnal");

  return ce;
}

void Ipv4CongaRouting::SetIpv4(Ptr<Ipv4> ipv4)
{
  NS_LOG_LOGIC(this << "set up ipv4:" << ipv4);
  m_ipv4 = ipv4;
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

void Ipv4CongaRouting::SetLeafId(uint32_t leafId)
{
  m_isLeaf = true;
  m_leafId = leafId;
}

void Ipv4CongaRouting::SetFlowletTimeOut(Time flowletTimeOut)
{
  m_flowletTimeOut = flowletTimeOut;
}

void Ipv4CongaRouting::AddAddreToLeafMap(Ipv4Address ipv4addr, uint32_t leafId)
{
  m_addressToLeafMap[ipv4addr] = leafId;
}

void Ipv4CongaRouting::AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port)
{
  NS_LOG_LOGIC(this << "add conga routing entry");
  Ipv4CongaRouteEntry ipv4CongaRouteEntry;
  ipv4CongaRouteEntry.network = network;
  ipv4CongaRouteEntry.networkMask = networkMask;
  ipv4CongaRouteEntry.Port = port;

  m_congaRouteEntryList.push_back(ipv4CongaRouteEntry);
}

std::vector<Ipv4CongaRouteEntry> Ipv4CongaRouting::LookupCongaRouteEntries(Ipv4Address dest)
{
  std::vector<Ipv4CongaRouteEntry> resultCongaRouteEntries;
  std::vector<Ipv4CongaRouteEntry>::iterator itr = m_congaRouteEntryList.begin();

  for(; itr != m_congaRouteEntryList.end(); itr++)
  {
    if((*itr).networkMask.IsMatch(dest, (*itr).network))
    {
      resultCongaRouteEntries.push_back(*itr);
    }
  }
  return resultCongaRouteEntries;
}

Ptr<Ipv4Route> Ipv4CongaRouting::ConstructIpv4CongaRoute(uint32_t port, Ipv4Address destAddress)
{
  NS_LOG_LOGIC(this << "construct an route entry ");
  Ptr<NetDevice> netdev = m_ipv4->GetNetDevice(port);
  Ptr<Channel> channel = netdev->GetChannel();
  uint32_t peerEnd = (channel->GetDevice(0) = netdev) ? 1 : 0;
  Ptr<Node> nextHop = channel->GetDevice(peerEnd)->GetNode();
  uint32_t nextInterface = channel->GetDevice(peerEnd)->GetIfIndex();
  Ipv4Address nextHopAddress = nextHop->GetObject<Ipv4>()->GetAddress(nextInterface, 0).GetLocal();

  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetOutputDevice(netdev);
  route->SetGateway(nextHopAddress);
  route->SetSource(m_ipv4->GetAddress(port, 0).GetLocal());
  route->SetDestination(destAddress);

  return route;
}

//TODO route input and route output
Ptr<Ipv4Route> Ipv4CongaRouting::RouteOutput(Ptr<Packet> packet, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &socketError)
{
  NS_LOG_ERROR(this << "conga routing only for l3 routing forward");
  return 0;
}

bool Ipv4CongaRouting::RouteInput(Ptr<const Packet> packet, const Ipv4Header &header, Ptr<const NetDevice> idev, 
                                  UnicastForwardCallback ucb, MulticastForwardCallback mcb, LocalDeliverCallback lcb, 
                                  ErrorCallback ecb)
{
  Ptr<Packet> p = ConstCast<Packet>(packet);
  Ipv4Address destAddress = header.GetDestination();

  //TODO implement addflowid in tcp-socket-base partially. 
  uint32_t flowId = 0;
  FlowIdTag flowIdTag;
  bool isFoundFlowIdTag = p->PeekPacketTag(flowIdTag);
  if(!isFoundFlowIdTag)
  {
    NS_LOG_ERROR(this << "can not find flowidtag");
  }
  flowId = flowIdTag.GetFlowId();

  //Get route entry to destaddress indicated edge switch
  std::vector<Ipv4CongaRouteEntry> destCongaRouteEntries = Ipv4CongaRouting::LookupCongaRouteEntries(destAddress);
  Time now = Simulator::Now();

  //1. NOTE. for leaf switch
  if(m_isLeaf)
  {
    Ipv4CongaTag ipv4CongaTag;
    bool isFoundCongaTag = packet->PeekPacketTag(ipv4CongaTag);

    //1.1 NOTE src edge switch.
    if(!isFoundCongaTag)
    {
      std::map<Ipv4Address, uint32_t>::iterator itr = m_addressToLeafMap.find(destAddress);
      if(itr == m_addressToLeafMap.end())
      {
        NS_LOG_ERROR(this << "can't find dest switch leafid in address2leafmap table");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
      }
      uint32_t destLeafId = itr->second;

      //initialize congaFromLeafTable item that located in src leaf switch.
      //1. first packet from src switch to dest switch that use specific path.
      //2. not the first packet... 
      uint32_t fbLbTag = 0;
      uint32_t fbMetric = 0;
      std::map<uint32_t, std::map<uint32_t, FBLBTagInfo>>::iterator fromLeafItr = m_congFromLeafTable.find(destLeafId);
      //there have a item with src switch leafid equals to destleafid in fromleaftable.
      if(fromLeafItr != m_congFromLeafTable.end())
      {
        std::map<uint32_t, FBLBTagInfo>::iterator fromLeafPathItr = (fromLeafItr->second).begin();

        uint32_t index = std::rand() % (fromLeafItr->second).size();
        std::advance(fromLeafPathItr, index);

        unsigned loop = 0;
        while ((fromLeafPathItr->second).change == false && loop < (fromLeafItr->second).size())
        {
          if(++fromLeafPathItr == (fromLeafItr->second).end())
          {
            fromLeafPathItr = (fromLeafItr->second).begin();
          }
          loop++;
        }

        if((fromLeafPathItr->second).change == false)
        {
          NS_LOG_ERROR("there have not such item that have been changed recently");
        }
        fbLbTag = fromLeafPathItr->first;
        fbMetric = (fromLeafPathItr->second).CE;
        (fromLeafPathItr->second).change = false;
      }
      //first packet, fblbtag == 0, fbmetric == 0

      //NOTE. flowlet lookup and route logic.
      //1.1.1 flowlet found
      //  a) isflowletexist == 1 && flowlet not timeout 
      //  b) isflowletexist == 1 && flowlet timeout
      //  c) isflowletexist == 0, flowlet struct was not initialized!!
      //QUESTION? how to get flow hash collision item???
      //1.1.2 flowlet not found
      
      /* struct Flowlet *flowlet = NULL; */
      /* std::map<uint32_t, struct Flowlet*>::iterator flowletItr = m_flowletTable.find(flowId); */

      struct Flowlet flowlet;
      std::map<uint32_t, struct Flowlet>::iterator flowletItr = m_flowletTable.find(flowId);
      //1.1.1 
      if(flowletItr != m_flowletTable.end())
      {
        flowlet = flowletItr->second;
        uint32_t selectPort = 0;
        //TODO  get seconds() or microseconds 
        Time flowGapTime = now - flowlet.activeTime;

        //other method to decide whether flowlet is exists.TODO
        bool isflowletexist = (flowlet.Port != 0) && (flowlet.activeTime != 0);

        //1.1.1.a) already exists flowlet and flowlet doesn't timeout.
        if(isflowletexist && flowGapTime <= m_flowletTimeOut)
        {
          flowlet.activeTime = now;
          selectPort = flowlet.Port;

          //add conga lbtag
          ipv4CongaTag.SetLBTag(selectPort);
          ipv4CongaTag.SetCE(0);
          //conga fblbtag ??
          //TODO
          ipv4CongaTag.SetFBLBTag(fbLbTag);
          ipv4CongaTag.SetCE(fbMetric);

          packet->AddPacketTag(ipv4CongaTag);

          Ipv4CongaRouting::UpdateInputTraffic(header, p, selectPort);
          Ptr<Ipv4Route> route = Ipv4CongaRouting::ConstructIpv4CongaRoute(selectPort, destAddress);
          ucb(route, packet, header);
          NS_LOG_LOGIC(this << "conga route packet to dest leaf switch" << destLeafId <<"from" << selectPort);
          
          return true;
        }
        //1.1.1.b). flowlet already exists. but flowlet timeout, update flowlet table: select new port for this flow.
        else if(isflowletexist && flowGapTime > m_flowletTimeOut) 
        {
          //caculate candidate port for newly arrival flowlet.
          //
          //traverse all possiable interface, select min congestion signal interface.
          std::map<uint32_t, std::map<uint32_t, LBtagInfo>>::iterator toLeafItr = m_congToLeafTable.find(destLeafId);
          uint32_t minCE = 999999;

          std::vector<Ipv4CongaRouteEntry>::iterator destCongaRouteItr = destCongaRouteEntries.begin();
          std::vector<uint32_t> candidatePortList;

          for(; destCongaRouteItr != destCongaRouteEntries.end(); destCongaRouteItr++)
          {
            uint32_t port = (*destCongaRouteItr).Port;
            uint32_t localCE = 0;
            uint32_t remoteCE = 0;

            std::map<uint32_t, uint32_t>::iterator inputTrafficItr = m_InputTrafficMap.find(port);
            if(inputTrafficItr != m_InputTrafficMap.end())
            {
              localCE = Ipv4CongaRouting::QuantilizeDreMetric(port, inputTrafficItr->second);
            }

            std::map<uint32_t, LBtagInfo>::iterator toLeafPathItr = (toLeafItr->second).find(port);
            if(toLeafPathItr != (toLeafItr->second).end())
            {
              remoteCE = (toLeafPathItr->second).CE; 
            }

            uint32_t pathCE = std::max(localCE, remoteCE);

            if(pathCE < minCE)
            {
              minCE = pathCE;
              candidatePortList.clear();
              candidatePortList.push_back(port);
            }
            else if(pathCE == minCE)
            {
              candidatePortList.push_back(port);
            }
            else
            {
              continue;
            }
          }

          //for new flowlet, switch selectPort to next candidate port.
          selectPort = candidatePortList[(flowlet.Port + 1) % candidatePortList.size()];
          flowlet.Port = selectPort;
          flowlet.activeTime = now; 
          //NOTE. doesn't deal with flowId hash collision. 

          ipv4CongaTag.SetLBTag(selectPort);
          ipv4CongaTag.SetCE(0);

          //TODO set fblbtag
          ipv4CongaTag.SetFBLBTag(fbLbTag);
          ipv4CongaTag.SetCE(fbMetric);

          p->AddPacketTag(ipv4CongaTag);

          Ipv4CongaRouting::UpdateInputTraffic(header, p, selectPort);
          Ptr<Ipv4Route> route = Ipv4CongaRouting::ConstructIpv4CongaRoute(selectPort, destAddress);

          ucb(route, p, header);

          return true;
        }
        //1.1.1.c) flowlet was not initialized
        else 
        {
          //doesn't normal case.
        }

      } // flowlet found end
      //1.1.2 flowlet not found
      else
      {
        NS_LOG_LOGIC(this << "newly arrival flow with flowid:" << flowId << "that doesn't contained in flowlet table");
        uint32_t selectPort = 0;

        //caclulate candidate port.
        std::map<uint32_t, std::map<uint32_t, LBtagInfo>>::iterator toLeafItr = m_congToLeafTable.find(destLeafId);

        std::vector<uint32_t> candidatePortList;
        std::vector<Ipv4CongaRouteEntry>::iterator destCongaRouteItr = destCongaRouteEntries.begin();
        
        uint32_t minCE = 999999;

        for(; destCongaRouteItr != destCongaRouteEntries.end(); destCongaRouteItr++)
        {
          uint32_t localCE  = 0;
          uint32_t remoteCE = 0;
          uint32_t port = destCongaRouteItr->Port;

          std::map<uint32_t, uint32_t>::iterator inputTrafficItr = m_InputTrafficMap.find(port);
          if(inputTrafficItr != m_InputTrafficMap.end())
          {
            localCE = Ipv4CongaRouting::QuantilizeDreMetric(port, inputTrafficItr->second);
          }

          std::map<uint32_t, LBtagInfo>::iterator toLeafPathItr = (toLeafItr->second).find(port);
          if(toLeafPathItr != (toLeafItr->second).end())
          {
            remoteCE = (toLeafPathItr->second).CE;
          }

          uint32_t pathCE = std::max(localCE, remoteCE);

          if(pathCE < minCE)
          {
            minCE = pathCE;
            candidatePortList.clear();
            candidatePortList.push_back(port);
          }
          else if(pathCE == minCE)
          {
            candidatePortList.push_back(port);
          }
          else
          {
            continue;
          }
        }  //endfor traverse destaddress route entries 
        
        //construct newflowlet, select flowlet route interface, and update flowlet info.  
        selectPort = candidatePortList[rand() % candidatePortList.size()];

        /* struct Flowlet *newflowlet = new Flowlet; */
        struct Flowlet newflowlet;
        newflowlet.activeTime     = now;
        newflowlet.Port           = selectPort;
        m_flowletTable[flowId]     = newflowlet;

        //add conga related packet tag.
        ipv4CongaTag.SetLBTag(selectPort);
        ipv4CongaTag.SetCE(0);

        ipv4CongaTag.SetFBLBTag(fbLbTag);
        ipv4CongaTag.SetCE(fbMetric);
        /* ipv4CongaTag.Setfb */
        p->AddPacketTag(ipv4CongaTag);

        Ipv4CongaRouting::UpdateInputTraffic(header, p, selectPort);
        Ptr<Ipv4Route> route = Ipv4CongaRouting::ConstructIpv4CongaRoute(selectPort, destAddress);

        ucb(route, p, header);
      }

    } // isfoundcongatag end 
    //1.2 NOTE dst edge switch
    else 
    {
      std::map<Ipv4Address, uint32_t>::iterator addrToLeafItr = m_addressToLeafMap.find(header.GetSource());
      if(addrToLeafItr == m_addressToLeafMap.end())
      {
        NS_LOG_ERROR(this << "conga routing doesn't find src leaf switch id");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
      }
      uint32_t srcLeafId = addrToLeafItr->second;

      //update from leaf table. if received packet with ipv4congatag, update fromleaftable of that switch:
      //change state,updatetime, and CE(for piggyback)!!
      //1. congafromleaftable[srcleafid] == null
      //2. congafromleaftable[srcleafid] != null,
      //  a). but congafromleaftable[srcleafid][pathid] == null.
      //  b). and congafromleaftable[srcleafid][pathid] != null.
      
      std::map<uint32_t, std::map<uint32_t, FBLBTagInfo>>::iterator fromLeafItr = m_congFromLeafTable.find(srcLeafId);

      if(fromLeafItr == m_congFromLeafTable.end())
      {
        FBLBTagInfo fblbTagInfo;
        fblbTagInfo.change     = true;
        fblbTagInfo.updateTime = Simulator::Now();
        fblbTagInfo.CE         = ipv4CongaTag.GetCE();
        
        std::map<uint32_t, FBLBTagInfo> newPathFbMap;
        newPathFbMap[ipv4CongaTag.GetLBTag()] = fblbTagInfo;

        m_congFromLeafTable[srcLeafId] = newPathFbMap;
      }
      else
      {
        std::map<uint32_t, FBLBTagInfo>::iterator fromLeafPathItr = (fromLeafItr->second).find(ipv4CongaTag.GetLBTag());
        if(fromLeafPathItr == (fromLeafItr->second).end())
        {
          FBLBTagInfo fblbTagInfo;
          fblbTagInfo.change     = true;
          fblbTagInfo.updateTime = Simulator::Now();
          fblbTagInfo.CE         = ipv4CongaTag.GetCE();

          (fromLeafItr->second)[ipv4CongaTag.GetLBTag()] = fblbTagInfo;
        }
        else
        {
          (fromLeafPathItr->second).change     = true;
          (fromLeafPathItr->second).updateTime = Simulator::Now();
          (fromLeafPathItr->second).CE         = ipv4CongaTag.GetCE();
        }
      }

      //update congatoleaftable.if received packet with ipv4congatag, update toleaftable of that switch: 
      //
      //1. congatoleaftable[srcleafid] == null
      //2. congatoleaftable[srcleafid] != null,
      //  a). but congatoleaftable[srcleafid][pathid] == null.
      //  b). and congatoleaftable[srcleafid][pathid] != null.
      //  NOTE. question? how to deal with first packet(fbmetric == 0, and fblbtag ==0)
      std::map<uint32_t, std::map<uint32_t, LBtagInfo>>::iterator toLeafItr = m_congToLeafTable.find(srcLeafId);
      if(toLeafItr == m_congToLeafTable.end())
      {
        LBtagInfo lbtagInfo;
        lbtagInfo.activeTime = Simulator::Now();
        lbtagInfo.CE         = ipv4CongaTag.GetFBMetric();

        std::map<uint32_t, LBtagInfo> newToLeafPathMap;
        newToLeafPathMap[ipv4CongaTag.GetFBLBTag()] = lbtagInfo;
        m_congToLeafTable[srcLeafId]                = newToLeafPathMap;
      }
      else
      {
        std::map<uint32_t, LBtagInfo>::iterator toLeafPathItr = (toLeafItr->second).find(ipv4CongaTag.GetFBLBTag());
        if(toLeafPathItr == (toLeafItr->second).end())
        {
          LBtagInfo lbtagInfo;
          lbtagInfo.activeTime = Simulator::Now();
          lbtagInfo.CE         = ipv4CongaTag.GetFBMetric(); // should be set to 0???
          
          (toLeafItr->second)[ipv4CongaTag.GetFBLBTag()] = lbtagInfo;
        }
        else
        {
          (toLeafItr->second)[ipv4CongaTag.GetFBLBTag()].activeTime = Simulator::Now();
          (toLeafItr->second)[ipv4CongaTag.GetFBLBTag()].CE         = ipv4CongaTag.GetFBMetric();
        }
      }

      //TODO. route ?
      uint32_t selectPort = destCongaRouteEntries[flowId % destCongaRouteEntries.size()].Port;

      Ipv4CongaRouting::UpdateInputTraffic(header, p, selectPort);

      Ptr<Ipv4Route> route = Ipv4CongaRouting::ConstructIpv4CongaRoute(selectPort, destAddress);
      ucb(route, p, header);

      return true;
    }    //dst edge end 
  }      // isleaf end 
  //2. NOTE. for core switch
  else
  {
    Ipv4CongaTag ipv4CongaTag;
    bool isFoundCongaTag = p->PeekPacketTag(ipv4CongaTag);
    if(!isFoundCongaTag)
    {
      NS_LOG_ERROR(this << "spine switch can't extract conga tag");
      ecb(p, header, Socket::ERROR_NOROUTETOHOST);
      return false;
    }

    uint32_t selectPort = destCongaRouteEntries[flowId % destCongaRouteEntries.size()].Port;

    uint32_t inputTrafficRegister = Ipv4CongaRouting::UpdateInputTraffic(header, p, selectPort);
    uint32_t localCE = Ipv4CongaRouting::QuantilizeDreMetric(selectPort, inputTrafficRegister);

    if(localCE > ipv4CongaTag.GetCE())
    {
      ipv4CongaTag.SetCE(localCE);
      p->ReplacePacketTag(ipv4CongaTag);
    }
    NS_LOG_LOGIC(this << "forward conga packet to spine switch with~~~ TODO");

    Ptr<Ipv4Route> route = Ipv4CongaRouting::ConstructIpv4CongaRoute(selectPort, destAddress);
    ucb(route, p, header);

    return true;
  } //end core switch 
  return false;
}

void Ipv4CongaRouting::CongaTableAgingEvent()
{
  bool idle = true;
  //aging fromleaftable
  std::map<uint32_t, std::map<uint32_t, FBLBTagInfo>>::iterator fromLeafItr = m_congFromLeafTable.begin();

  for(; fromLeafItr != m_congFromLeafTable.end(); fromLeafItr++)
  {
    std::map<uint32_t, FBLBTagInfo>::iterator fromLeafPathItr = (fromLeafItr->second).begin();
    for(; fromLeafPathItr != (fromLeafItr->second).end(); fromLeafPathItr++)
    {
      if(Simulator::Now() - (fromLeafPathItr->second).updateTime > m_congaTableAgingTime)
      {
        (fromLeafItr->second).erase(fromLeafPathItr);
        if((fromLeafItr->second).empty())
        {
          m_congFromLeafTable.erase(fromLeafItr);
        }
      }
      else
      {
        idle = false;
      }
    }
  }

  //aging toleaftable
  std::map<uint32_t, std::map<uint32_t, LBtagInfo>>::iterator toLeafItr = m_congToLeafTable.begin();

  for(; toLeafItr != m_congToLeafTable.end(); toLeafItr++)
  {
    std::map<uint32_t, LBtagInfo>::iterator toLeafPathItr = (toLeafItr->second).begin();
    for(; toLeafPathItr != (toLeafItr->second).end(); toLeafPathItr++)
    {
      if(Simulator::Now() - (toLeafPathItr->second).activeTime > m_congaTableAgingTime)
      {
        (toLeafItr->second).erase(toLeafPathItr);
        if((toLeafItr->second).empty())
        {
          m_congToLeafTable.erase(toLeafItr);
        }
      }
      else
      {
        idle = false;
      }

    }
  }

  if(!idle)
  {
    //TODO schedule time m_congatableagingtime/n ?? n ??
    m_congaTableAgingEvent = Simulator::Schedule(m_congaTableAgingTime, &Ipv4CongaRouting::CongaTableAgingEvent, this);
  }
  else
  {
    NS_LOG_LOGIC(this << "conga table aging event was not trigged!");
  }
}

void Ipv4CongaRouting::UpdateResidualTrafficEvent()
{
  bool idle = true;

  std::map<uint32_t, uint32_t>::iterator inputTrafficItr = m_InputTrafficMap.begin();
  for(; inputTrafficItr != m_InputTrafficMap.end(); inputTrafficItr++)
  {
    uint32_t newResidualTraffic = inputTrafficItr->second * (1 - m_alpha);
    inputTrafficItr->second = newResidualTraffic;
    if(newResidualTraffic != 0)
    {
      idle = false;
    }
  }

  NS_LOG_LOGIC(this << "update traffic register");

  if(!idle)
  {
    m_updateResidualTrafficEvent = Simulator::Schedule(m_Tdre, &Ipv4CongaRouting::UpdateResidualTrafficEvent, this);
  }
  else
  {
    NS_LOG_LOGIC(this << "UpdateResidualTrafficEvent was not triggered");
  }
}

} //ns3 
