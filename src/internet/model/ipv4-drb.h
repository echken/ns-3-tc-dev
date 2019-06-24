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

}
}
