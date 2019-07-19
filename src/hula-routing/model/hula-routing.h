/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef HULA_ROUTING_H
#define HULA_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/object.h"
#include "ns3/ipv4.h"
#include "ns3/nstime.h"

#include <algorithm>
#include <vector>
#include <map>

namespace ns3 {

/* ... */
class Ipv4HulaRouting : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId(void);
  Ipv4HulaRouting();
  ~Ipv4HulaRouting();
  virtual TypeId GetInstanceTypeId(void);

  //inherit from base 
  virtual void NotifyInterfaceUp(uint32_t interface);
  virtual void NotifyInterfaceDown(uint32_t interface);
  virtual void NotifyAddAddress(uint32_t interface, Ipv4Address address);
  virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
  virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit uint) const;


  //>>>??
  virtual void SetIpv4(Ptr<Ipv4> ipv4);
  void SetPathUpdateInterval(Time pathUpdateInterval);
  void SetLatestPathUtilUpdateTime(Time latestUpdateTime);

  //processing probe packet && linkutil measurement:
  uint32_t UpdateOutputTraffic(const Ipv4Header header&, Ptr<Packet> p, uint32_t port);
  void UpdateOutputTrafficEvent(); 

  //route 
  //1. if probe packet
  //  a). udpate hulaPathUtilTable and hulaBestNextHopTable
  //2. if normal packet:
  //  a). flowlet detect and route
  Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &SocketErrno);
  bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb, LocalDeliverCallback lcb,
                            ErrorCallback ecb);



private:
  Ptr<Ipv4> m_ipv4;
  Time m_flowletTimeOut;

  //processing probe packet && linkutil measurement:
  Time m_pathUpdateIterval;   // link util update cycle
  Time m_latestPathUtilUpdateTime;  // time between lastest pathUtil update. 
  std::map<uint32_t, std::pair<Time, uint32_t>> m_outputLinkUtilMap; //<interface, <udpateTime, outputlinkUtil>>
  std::map<uint32_t, uint32_t> m_outputTrafficMap;  //<interface, outputTrafficRegister>
  
  //pathutil statistics 
  std::map<uint32_t, std::pair<uint32_t, uint32_t>> m_hulaPathUtilTable;  // <destTorId, <nextHop, pathUtil>>

  //bestnexthop table
  std::map<uint32_t, uint32_t> m_hulaBestPathTable;   //<flowletId, nextHop>
};

}
#endif /* HULA_ROUTING_H */

