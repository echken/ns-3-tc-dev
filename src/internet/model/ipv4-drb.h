#ifndef IPV4_DRB_H
#define IPV4_DRB_H

#include "vector.h"
#include "map.h"
#include "ns3/object.h"
#include "ns3/ipv4-address.h"

namespace ns3 {
class Ipv4Drb : public Object
{
public:
    static TypeId GetTypeId(void);

    Ipv4Drb();
    ~Ipv4Drb();

    Ipv4Address GetCoreSwitchAddress(uint32_t flowId);
    //add bounce core switch at per-packet granularity, 
    void AddCoreSwitchAddress(Ipv4Address addr);
    //add bounce core switch at k-packet granularity, for implement presto etc, flowcell based LB mechanism.
    void AddCoreSwitchAddress(uint32_t k, Ipv4Address addr);

private:
    std::vector<Ipv4Address> m_coreSwitchAddressList;
    std::map<uint32_t, uint32_t> m_indexMap;
};
}
#endif
