#pragma once
#include "ns3/tag.h"
#include "ipv4-header.h"

namespace ns3 {

class Ipv4EcnTag : public Tag
{
public:
    ipv4-ecn-tag() {}
    ~ipv4-ecn-tag() {}

    // layer3 set and get function for simulate swich mark ip packet according to queue parameter.
    void SetEcn(Ipv4Header::EcnType ecn);
    Ipv4Header::EcnType GetEcn(void) const;

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;

    //TODO. other tag virtual function ??
private:
    uint8_t m_ipv4_ecn;
}

}

