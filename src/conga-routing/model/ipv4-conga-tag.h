#ifndef NS3_IPV4_CONGA_TAG
#define NS3_IPV4_CONGA_TAG

#include "ns3/tag.h"

namespace ns3{

class Ipv4CongaTag: public Tag
{
public:
  Ipv4CongaTag();
  ~Ipv4CongaTag();

  static TypeId GetTypeId(void);

  virtual TypeId GetInstanceTypeId(void) const;

  virtual void Serialize(TagBuffer i) const;
  virtual void Deserialize(TagBuffer i);
  virtual uint32_t GetSerializedSize(void) const;

  void SetLBTag(uint32_t lbTag);
  uint32_t GetLBTag(void) const;

  void SetCE(uint32_t ce);
  uint32_t GetCE(void) const;

  void SetFBLBTag(uint32_t fblbTag);
  uint32_t GetFBLBTag(void) const;

  void SetFBMetric(uint32_t fbMetric);
  uint32_t GetFBMetric(void) const;

  virtual void Print (std::ostream &os) const;
private:
  uint32_t m_LBTag;
  uint32_t m_CE;
  uint32_t m_FBLBTag;
  uint32_t m_FBMetric;
};

} 
#endif
