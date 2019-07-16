
#include "hula-tag.h"

namespace ns3{
TypeId Ipv4HulaTag::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Ipv4HulaTag")
    .SetParent<Tag>()
    .SetGroupName("hula-routing")
    .AddConstructor<Ipv4HulaTag>();

  return tid;
}

TypeId Ipv4HulaTag::GetInstanceTypeId(void) const
{
  return Ipv4HulaTag::GetTypeId();
}

Ipv4HulaTag::Ipv4HulaTag()
{

}

Ipv4HulaTag::~Ipv4HulaTag()
{

}

void Ipv4HulaTag::SetMaxPathUtil(uint32_t newMaxPathUtil)
{
  m_maxPathUtil = newMaxPathUtil;
}

void Ipv4HulaTag::SetProbeDestAddress(Ipv4Address destAddress)
{
  m_probeDstAddress = destAddress;
}

void Ipv4HulaTag::SetOutputInterface(uint32_t outputInterface)
{
  m_outputInterface = outputInterface;
}

void Ipv4HulaTag::Serialize(TagBuffer i) const
{
  i.WriteU32(m_maxPathUtil);
  i.WriteU32(m_outputInterface);
  //TODO write ipv4Address to serialized stream
}

void Ipv4HulaTag::Deserialize(TagBuffer i)
{
  m_maxPathUtil = i.ReadU32();
  m_outputInterface = i.ReadU32();
  //TODO read ipv4address
}

uint32_t Ipv4HulaTag::GetSerializedSize(void) const
{
  return sizeof(uint32_t)
    + sizeof(uint32_t)
    + sizeof(Ipv4Address);
}

void Ipv4HulaTag::Print(std::ostream &os) const
{
  os << "path:"    << m_outputInterface
     << "util:"    << m_maxPathUtil
     << "address:" << m_probeDstAddress;
}

}
