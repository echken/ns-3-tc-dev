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
#include "ns3/ipv4-routing-table-entry.h"

namespace ns3 {

class Ipv4MulticastRoutingTableEntry;

struct Ipv4HulaRouteEntry{
  Ipv4Address network;
  Ipv4Mask networkMask;
  uint32_t port;
}

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
  void SetTorId(uint32_t torId);
  void AddAddressToTorIdMap(Ipv4Address ipv4Address, uint32_t torId);
  
  void SetSwitchRole(SwitchRole role);
  uint32_t GetSwitchRole() const;
  uint32_t GetDirection(Ptr<Packet> packet);

  void SetPathUpdateInterval(Time pathUpdateInterval);
  void SetLatestPathUtilUpdateTime(Time latestUpdateTime);

  //processing probe packet && linkutil measurement:
  uint32_t UpdateOutputTraffic(const Ipv4Header header&, Ptr<Packet> p, uint32_t port);
  void UpdateOutputTrafficEvent(); 
  void OutputLinkUtilUpdate(const Ipv4Header &header, Ptr<Packet> packet, uint32_t port);

  //route 
  //1. if probe packet
  //  a). udpate hulaPathUtilTable and hulaBestNextHopTable
  //2. if normal packet:
  //  a). flowlet detect and route
  Ptr<Ipv4Route> ConstructIpv4HulaRoute(uint32_t port, Ipv4Address destAddress);
  void AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port);
  //TODO add multicat route table for setup  probe routing path 
  void AddMulticastRoute(Ipv4Address origin, Ipv4Address group, uint32_t inputInterface, 
                         std::vector<uint32_t> outputInterfaces);

  Ipv4MulticastRoutingTableEntry GetMulticastRoute(uint32_t index) const;
  Ptr<Ipv4MulticastRoute> LookupMulticastRoute(Ipv4Address origin, Ipv4Address group, uint32_t interface);

  virtual Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &SocketErrno);
  virtual bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb, LocalDeliverCallback lcb,
                            ErrorCallback ecb);

  void SetProbeMulticastGroup(Ipv4Address multicastAddress, std::vector<uint32_t> outputInterfaceSet);
  void SetProbeMulticastInterface(Ipv4Address multicastAddress, uint32_t interface);
  std::vector<uint32_t> GetProbeMulticastGroup(Ipv4Address multicastAddress);
  std::vector<uint32_t> LookUpMulticastGroup(Ipv4Address probeAddress);

  Ptr<Ipv4Route> LoopbackRoute (const Ipv4Header &header, Ptr<NetDevice> oif) const;

private:
  Ptr<Ipv4> m_ipv4;
  bool m_isTor;
  uint32_t m_torId;

  Ptr<NetDevice> m_lo;
  std::list<Ipv4MulticastRoutingTableEntry> m_multicastRoutes;

  std::map<Ipv4Address, uint32_t> m_addressToTorIdMap;
  /* std::map<uint32_t, std::vector<uint32_t>> m_torToUpstreamInterfaceMap; // control plane set up */ 

  std::map<Ipv4Address, std::vector<uint32_t>> m_porbeMulticastTable;  //<multicastAddress, vector<outputInterface>>
  
  Time m_flowletTimeOut;
  Time m_pathFailureTimeOut;

  //processing probe packet && linkutil measurement:
  Time m_pathUpdateIterval;   // link util update cycle
  Time m_latestPathUtilUpdateTime;  // time between lastest pathUtil update. 
  std::map<uint32_t, std::pair<Time, uint32_t>> m_outputLinkUtilMap; //<interface, <udpateTime, outputlinkUtil>>
  std::map<uint32_t, uint32_t> m_outputTrafficMap;  //<interface, outputTrafficRegister>
  
  //pathutil statistics, at each hop, save the next hop to reach each dest. 
  /* std::map<uint32_t, std::pair<Time, std::pair<uint32_t, uint32_t>>> m_hulaPathUtilTable;  // <destTorId, <nextHop, pathUtil>> */

  std::map<uint32_t, HulaPathUtilInfo> m_hulaPathUtilTable;
  struct HulaPathUtilInfo{
    uint32_t pathUtil;
    uint32_t interface;
    Time updateTime;
  };

  //bestnexthop table, at each hop, for each flow
  std::map<uint32_t, FlowletInfo*> m_hulaBestPathTable;   //<flowletId, nextHop>
  struct FlowletInfo{
    uint32_t interface;
    Time updateTime;
  }

  enum SwitchRole{
    HULA_NONE = 0;
    HULA_TOR;
    HULA_AGGREGATE;
    HULA_SPINE;
  };

  SwitchRole m_switchRole{SwitchRole::HULA_NONE};
  std::vector<Ipv4HulaRouteEntry> m_hulaRouteEntryList;
};

}
#endif /* HULA_ROUTING_H */

