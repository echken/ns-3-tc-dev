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
  void SetProbeDestAddress(Ipv4Address destAddress);
  void SetOutputInterface(uint32_t outputInterface);

  //inherient from base class
  virtual void Serialize(TagBuffer i) const;
  virtual void Deserialize(TagBuffer i);
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Print(std::ostream &os) const;

private:
  uint32_t m_maxPathUtil;  // quantilized path util to specific bit string.
  Ipv4Address m_probeDstAddress;  // probe dest 
  uint32_t m_outputInterface;  // send probe from this interface 

};

}
#endif
