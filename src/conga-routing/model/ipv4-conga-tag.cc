#include "ipv4-conga-tag.h"

namespace ns3
{
Ipv4CongaTag::Ipv4CongaTag() 
{
}

Ipv4CongaTag::~Ipv4CongaTag()
{
}

TypeId Ipv4CongaTag::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Ipv4CongaTag")
    .SetParent<Tag>()
    .SetGroupName("Internet")
    .AddConstructor<Ipv4CongaTag> ();
  return tid;
}

void Ipv4CongaTag::SetLBTag(uint32_t lbTag)
{
  m_LBTag = lbTag;
}

uint32_t Ipv4CongaTag::GetLBTag() const
{
  return m_LBTag;
}

void Ipv4CongaTag::SetCE(uint32_t ce)
{
  m_CE = ce;
}

uint32_t Ipv4CongaTag::GetCE() const
{
  return m_CE;
}

void Ipv4CongaTag::SetFBLBTag(uint32_t fblbTag)
{
  m_FBLBTag = fblbTag;
}

uint32_t Ipv4CongaTag::GetFBLBTag() const
{
  return m_FBLBTag;
}

void Ipv4CongaTag::SetFBMetric(uint32_t fbMetric)
{
  m_FBMetric = fbMetric;
}

uint32_t Ipv4CongaTag::GetFBMetric() const
{
  return m_FBMetric;
}

TypeId Ipv4CongaTag::GetInstanceTypeId() const
{
  return GetTypeId();
}

void Ipv4CongaTag::Serialize(TagBuffer i) const
{
  i.WriteU32(m_LBTag);
  i.WriteU32(m_CE);
  i.WriteU32(m_FBLBTag);
  i.WriteU32(m_FBMetric);
}

void Ipv4CongaTag::Deserialize(TagBuffer i)
{
  m_LBTag    = i.ReadU32();
  m_CE       = i.ReadU32();
  m_FBLBTag  = i.ReadU32();
  m_FBMetric = i.ReadU32();
}

uint32_t Ipv4CongaTag::GetSerializedSize() const 
{
  return sizeof (uint32_t) +
         sizeof (uint32_t) +
         sizeof (uint32_t) +
         sizeof (uint32_t);
}

void
Ipv4CongaTag::Print (std::ostream &os) const
{
  os << "Lb Tag = " << m_LBTag;
  os << "CE  = " << m_CE;
  os << "Feedback Lb Tag = " << m_FBLBTag;
  os << "Feedback Metric = " << m_FBMetric;
}
}
