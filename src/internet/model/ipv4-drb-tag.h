#ifndef NS3_IPV4_DRB_TAG
#define NS3_IPV4_DRB_TAG

#include "ns3/tag.h"
#include "ns3/ipv4-address.h"

namespace ns3 {
class IpvrDrbTag :: public Tag
{
public:
    Ipv4DrbTag();
    ~Ipv4DrbTag();

    //set end to end destination host address.
    void SetOriginalDestAddr(Ipv4Address addr);
    //get end to end destination host address.
    Ipv4Address GetOriginalDestAddr(void) const;

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;

    virtual Serialize(TagBuffer i) const;
    virtual Deserialize(TagBuffer i);
    virtual GetSerialziedSize(void) const;

    virtual Print(std::ostream &os) const;

private:
    Ipv4Address m_addr;
}
}
#endif
