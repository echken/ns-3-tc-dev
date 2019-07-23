#ifndef NS_IPV4_HULA_TAG
#define NS_IPV4_HULA_TAG

#include "ns3/tag.h"
#include "ns3/ipv4-address.h"

namespace ns3{

class Ipv4HulaTag: public Tag
{
public:
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  Ipv4HulaTag();
  ~Ipv4HulaTag();

  void SetMaxPathUtil(uint32_t newMaxPathUtil);
  uint32_t GetMaxPathUtil(void) const;

  void SetTorId(uint32_t torId);
  uint32_t GetTorId(void) const;

  void SetProbeDestAddress(Ipv4Address destAddress);
  Ipv4Address GetProbeDestAddress(void) const;
  
  void SetOutputInterface(uint32_t outputInterface);
  uint32_t GetOutputInterface(void) const;

  void SetDirection(uint32_t direction);
  uint32_t GetDirection(void) const;

  void SetSwitchRole(uint32_t role);
  uint32_t GetSwitchRole(void) const;

  //inherient from base class
  virtual void Serialize(TagBuffer i) const;
  virtual void Deserialize(TagBuffer i);
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Print(std::ostream &os) const;

private:
  uint32_t m_maxPathUtil;  // quantilized path util to specific bit string.
  uint32_t m_torId;
  Ipv4Address m_probeDstAddress;  // probe dest 
  uint32_t m_outputInterface;  // send probe from it  

  uint32_t m_direction;
  uint32_t m_switchRole;

};

}
#endif
