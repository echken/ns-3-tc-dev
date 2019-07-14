/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_CONGA_ROUTING_H
#define IPV4_CONGA_ROUTING_H

#include "vector"
#include "map"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/data-rate.h"

namespace ns3 {
/* ... */
  struct Ipv4CongaRouteEntry {
    Ipv4Address network;
    Ipv4Mask networkMask;
    uint32_t Port;
  };
class Ipv4CongaRouting: public Ipv4RoutingProtocol
{
public:
  Ipv4CongaRouting();
  ~Ipv4CongaRouting();

  static TypeId GetTypeId(void);

  virtual void SetIpv4(Ptr<Ipv4> ipv4);
  //1.XXX. dre alg function.  alpha, dre, register X for input/residual traffic, Q bit ce flag for quatilizing congestion metric of each interface, echken.
  void SetTdre(Time time);
  void SetAlpha(double alpha);
  void SetQbit(uint32_t qbit);
  void SetLinkCapacity(DataRate linkcap);                      //set universal link capacity
  void SetLinkCapacity(uint32_t interface, DataRate lincap);   //set link cap for each interface.
  
  uint32_t UpdateInputTraffic(const Ipv4Header &header, Ptr<Packet> packet, uint32_t port);    // update local port inputTraffic register, X = X*(1-alpha)
  uint32_t QuantilizeDreMetric(uint32_t interface, uint32_t residualTraffic);  //quantilizing residual traffic register to Qbit CE metric. ce = 2^Q * (X/(linkCap*Tdre/alpha)))
  //dre alg parameter
  //DRE event loop
  void UpdateResidualTrafficEvent();
  
  //2.XXX. leaf identity related. leafid, address2leafidmap, isleaf
  void SetLeafId(uint32_t leafId);
  void AddAddreToLeafMap(Ipv4Address ipv4addr, uint32_t leafId);

  //3.XXX. flowlet related. 
  void SetFlowletTimeOut(Time flowletTimeOut);   // whether need to aging??
  
  //4.XXX. congestion statistics and update.
  //TODO. congestion to leaf table and congestion table aging??
  void CongaTableAgingEvent();

  //5.XXX. conga routing related data struct, echken.
  std::vector<Ipv4CongaRouteEntry> LookupCongaRouteEntries(Ipv4Address dest);
  Ptr<Ipv4Route> ConstructIpv4CongaRoute(uint32_t port, Ipv4Address destAddress);
  void AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port);

  virtual Ptr<Ipv4Route> RouteOutput(Ptr<Packet> packet, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &socketError);
  virtual bool RouteInput(Ptr<const Packet> packet, const Ipv4Header &header, Ptr<const NetDevice> idev, 
                          UnicastForwardCallback ucb, MulticastForwardCallback mcb, 
                          LocalDeliverCallback lcb, ErrorCallback ecb);

  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit uint) const;
private:
  Ptr<Ipv4> m_ipv4;
  
  //1.XXX  dre
  Time m_Tdre;
  double m_alpha;
  uint32_t m_Qbit;

  DataRate m_LinkCapacity;
  std::map<uint32_t, DataRate> m_InterfaceLinkCapacity;
  std::map<uint32_t, uint32_t> m_InputTrafficMap;   //<interface, inputTrafficRegister>
  
  EventId m_updateResidualTrafficEvent;
  //2.XXX leafid 
  bool m_isLeaf;
  uint32_t m_leafId;
  std::map<Ipv4Address, uint32_t> m_addressToLeafMap;
  
  //3.XXX flowlet
  Time m_flowletTimeOut;
  struct Flowlet{
    Time activeTime;
    uint32_t Port;
    /* uint32_t flowletId; */
  };
  std::map<uint32_t, Flowlet> m_flowletTable;
  
  //4.XXX congestion statistics   
  struct LBtagInfo{
    Time activeTime;  
    uint32_t CE;
  };

  struct FBLBTagInfo{
    /* uint32_t FBLBTag; */
    bool change;
    uint32_t CE;
    Time updateTime;   //modified while CE was updated, 
  };

  EventId m_congaTableAgingEvent;
  Time m_congaTableAgingTime;

  std::map<uint32_t, std::map<uint32_t, LBtagInfo>> m_congToLeafTable;   //<destLeafId, <pathId, CE>>
  std::map<uint32_t, std::map<uint32_t, FBLBTagInfo>> m_congFromLeafTable; //<sourceLeafid, <pathId, feedback>>

  //5.XXX routing
  //NOTE. another implementation: <dest, std::vector<Ipv4CongaRouteEntry>>


  std::vector<Ipv4CongaRouteEntry> m_congaRouteEntryList;

};

}

#endif /* IPV4_CONGA_ROUTING_H */

