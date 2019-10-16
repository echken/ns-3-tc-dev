/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_PBQ_ROUTING_H
#define IPV4_PBQ_ROUTING_H

#include "ns3/queue.h"
#include "ns3/queue-disc.h"

#include "ns3/object.h"
#include "ns3/ipv4"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"

#include <algorithm>
#include <random>
#include <vector> 
#include <list>

namespace ns3 {

/* ... */
class Ipv4PBQRouting : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId(void);
  Ipv4PBQRouting();
  virtual ~Ipv4PBQRouting();
  virtual TypeId GetInstanceTypeId(void) const;

  virtual void NotifyInterfaceUp(uint32_t interface);
  virtual void NotifyInterfaceDown(uint32_t interface);
  virtual void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
  virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit uint) const;

  //overview:
  // 1. measurement flow level statistics;
  // 2. filter heavy queue in recent    
  //    NOTE. maybe first select port&&queue based on credit, then queue-level summary, next update queue statistic, 
  //    last update real-time credit(periodically triggered and max credit threshold triggered update real-time credit)
  // 3. credi
  
  uint32_t Ipv4PBQRoutingCredit();

}; 

class HHFQueueDisc : public QueueDisc
{
public:
  static TypeId TypeId(void);
  HHFQueueDisc();
  virtual ~HHFQueueDisc();

private:
};

}

#endif /* IPV4_PBQ_ROUTING_H */

