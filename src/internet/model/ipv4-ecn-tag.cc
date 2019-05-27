#include "ipv4-ecn-tag.h"

namespace ns3{
Ipv4EcnTag::Ipv4EcnTag() {}

void Ipv4EcnTag::SetEcn(Ipv4Header::EcnType ecn)
{
    m_ipv4_ecn = ecn;
}
Ipv4Header::EcnType Ipv4EcnTag::GetEcn(void) const
{
    return Ipv4Header::EcnType(m_ipv4_ecn);
}

TypeId Ipv4EcnTag::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::Ipv4EcnTag")
        .SetParent<Tag>()
        .SetGroupName("Internet")
        .AddConstructor<Ipv4EcnTag>();

    return tid;
}

}
