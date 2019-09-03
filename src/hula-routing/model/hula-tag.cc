
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
uint32_t Ipv4HulaTag::GetMaxPathUtil(void) const
{
  return m_maxPathUtil;
}

void Ipv4HulaTag::SetTorId(uint32_t torId)
{
  m_torId = torId;
}

uint32_t Ipv4HulaTag::GetTorId(void) const
{
  return m_torId;
}

void Ipv4HulaTag::SetProbeDestAddress(Ipv4Address destAddress)
{
  m_probeDstAddress = destAddress;
}
Ipv4Address Ipv4HulaTag::GetProbeDestAddress(void) const
{
  return m_probeDstAddress;
}

/* void Ipv4HulaTag::SetOutputInterface(uint32_t outputInterface) */
/* { */
/*   m_outputInterface = outputInterface; */
/* } */
/* uint32_t Ipv4HulaTag::GetOutputInterface(void) const */
/* { */
/*   return m_outputInterface; */
/* } */

void Ipv4HulaTag::SetDirection(uint32_t direction)
{
  m_direction = direction;
}
uint32_t Ipv4HulaTag::GetDirection(void) const
{
  return m_direction;
}

void Ipv4HulaTag::SetSwitchRole(uint32_t role)
{
  m_switchRole = role;
}
uint32_t Ipv4HulaTag::GetSwitchRole(void) const
{
  return m_switchRole;
}

void Ipv4HulaTag::Serialize(TagBuffer i) const
{
  i.WriteU32(m_maxPathUtil);
  /* i.WriteU32(m_outputInterface); */
  i.WriteU32(m_direction);
  //TODO write ipv4Address to serialized stream
}

void Ipv4HulaTag::Deserialize(TagBuffer i)
{
  m_maxPathUtil = i.ReadU32();
  /* m_outputInterface = i.ReadU32(); */
  m_direction = i.ReadU32();
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
  os << "util:"    << m_maxPathUtil
     << "address:" << m_probeDstAddress;
}

}
