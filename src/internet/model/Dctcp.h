#ifndef TCP_DCTCP_H
#define TCP_DCTCP_H

#include "ns3/tcp-congestion-ops.h"

namespace ns3 {

class Dctcp : public TcpNewReno
{
public:
    static TypeId GetTypeId(void);
    Dctcp() {};
    Dctcp(const Dctcp& sock);
    virtual ~Dctcp(void) {};

    virtual std::string GetName() const;
    virtual Ptr<TcpCongestionOps> Fork();

    /* reduce cwind based RFC8267 && RFC3168 */
    virtual void ReduceCwnd(Ptr<TcpSocketState> tcb);
    /* get segement Acked and rtt estimate value */
    virtual void PktsAcked(Ptr<TcpSocketState> tcb,
                           uint32_t segementsAcked,
                           const Time &rtt);

    /* change congestion state */
    virtual void CwndEvent (Ptr<TcpSocketState> tcb,
                            const TcpSocketState::TcpCAEvent_t event);

private:
    void UpdateAckReserved(Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCAEvent_t);

    void SetDctcpAlpha(double alpha);
    void Reset(Ptr<TcpSocketState> tcb);

    //parameter
    uint32_t m_ackedBytesCongestion;  //Bytes experience congestion
    uint32_t m_ackedBytesTotal;       //total bytes
    double m_alpha;                   // congestion parameter
    bool m_ceState;                   // whether it's ce enable connection

    SequenceNumber32 m_nextWinSeqFlag;
    SequenceNumber32 m_nextWinSeq;    // next windows first bit seq
    
    bool m_congestionState;
    bool m_delayedAckReserved;

};

}

#endif
