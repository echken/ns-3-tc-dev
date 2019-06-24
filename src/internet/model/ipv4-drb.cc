#include "ns3/log.h"
#include "ipv4-drb.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("Ipv4Drb");
NS_OBJECT_ENSURE_REGISTERED(Ipv4Drb);

TypeId GetTypeId(void) 
{
   static TypeId tid = TypeId("ns3::Ipv4Drb")
       .SetParent<Object>()
       .SetGroupName("Internet")
       .AddConstructor<Ipv4Drb>();
   return tid;
}

Ipv4Drb::Ipv4Drb()
{
    NS_LOG_FUNCTION(this);
}

Ipv4Drb::~Ipv4Drb()
{
    NS_LOG_FUNCTION(this);
}

void AddCoreSwitchAddress(Ipv4Address addr)
{
    NS_LOG_FUNCTION(this << addr);
    m_coreSwitchAddressList.push_back(addr);
}

void AddCoreSwitchAddress(uint32_t k, Ipv4Address addr)
{
    NS_LOG_FUNCTION(this << k << addr);
    for (int i = 0; i < k; i++)
    {
        m_coreSwitchAddressList.push_back(addr);
    }
}

Ipv4Address GetCoreSwitchAddress(uint32_t flowId)
{
    NS_LOG_FUNCTION(this << flowId);
    
    uint32_t listSize = m_coreSwitchAddressList.size();
    if(listSize == 0)
    {
        return Ipv4Address();
    }

    uint32_t index = rand() % listSize; //for the flow that was received just now, select random index.
    std::map<uint32_t, uint32_t>::iterator itr = m_indexMap.find(flowId);

    if(itr != m_indexMap.end())
    {
        index = itr.second();
    }
    Ipv4Address addr m_coreSwitchAddressList[index];
    m_indexMap[flowId] = (index + 1)%listSize; //update index for next packet/flowcell of flow that have same flowId with current flow.

    NS_LOG_DEBUG(this << "index for flow:" << flowId << "is" << index);
    return addr;
}
}
