#ifndef NS_HULA_PROBING_H
#define NS_HULA_PROBING_H

#include "ns3/object.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

#include "ns3/socket.h"
#include "ns3/node.h"

namespace ns3{

class Ipv4HulaProbing: public Object
{
public:
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  Ipv4HulaProbing();
  ~Ipv4HulaProbing();

  //construct probe packet header
  void SetSrcAddress(Ipv4Address srcaddress);
  void SetDstAddress(Ipv4Address dstaddress);

  //processing probe
  void StartProbe();
  void StopProbe(Time stopTimer);
  void DoStartProbe();
  void DoStopProbe();
  void SendProbe();

  void ReceivePacket(Ptr<Socket> socket);

  //update probe packet maxPathUtil fileds???
  
private:
  //for construct probe packet!
  //[srcaddress, destaddress, TorId, interface??, maxPathUtil]
  Ipv4Address m_probeSrcAddress;
  Ipv4Address m_probeDstAddress;
  Time m_probeInterval;
  uint32_t m_maxPathUtil;
  uint32_t m_torId;

  /* std::map<Ipv4Address, std::pair<uint32_t, double>> m_hulaPathUtilTable; */
  Ptr<Ipv4> m_ipv4;
  Ptr<Socket> m_socket;
  Ptr<Node> m_node;

  EventId m_probeEvent;
};

}
#endif
