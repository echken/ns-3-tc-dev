#ifndef TCP_DCTCP_H
#define TCP_DCTCP_H

#include "ns3/tcp-congestion-ops.h"
#include "tcp-socket-base.h"

namespace ns3 {
class Dctcp : public TcpNewReno
{
public:
    static TypeId GetTypeId(void);
    Dctcp();
    Dctcp(const Dctcp& sock);
    virtual ~Dctcp(void);

    virtual Ptr<TcpSocketBase> GetSocketBase(void);
    virtual void SetSocketBase(Ptr<TcpSocketBase> tsb);

    virtual std::string GetName() const;
    virtual Ptr<TcpCongestionOps> Fork();

    /* reduce cwind based RFC8267 && RFC3168 */
    virtual void ReduceCwnd(Ptr<TcpSocketState> tcb);
    /* get segment Acked and rtt estimate value */
    virtual void PktsAcked(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked,
                           const Time &rtt);

    /* change congestion state */
    virtual void CwndEvent (Ptr<TcpSocketState> tcb,
                            const TcpSocketState::TcpCAEvent_t event);

private:
    void UpdateAckReserved(Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCAEvent_t event);

    void SetDctcpAlpha(double alpha);
    void Reset(Ptr<TcpSocketState> tcb);

    void CeState0to1(Ptr<TcpSocketState> tcb);
    void CeState1to0(Ptr<TcpSocketState> tcb);

    //parameter
    Ptr<TcpSocketBase> m_tsb;             //!< TCP Socket Base state
    uint32_t m_ackedBytesCongestion;  //Bytes experience congestion
    uint32_t m_ackedBytesTotal;       //total bytes
    double m_alpha;                   // congestion parameter
    double m_g;
    bool m_ceState;                   // whether it's ce enable connection

    bool m_nextWinSeqFlag;
    SequenceNumber32 m_nextWinSeq;    // next windows first bit seq
    
    SequenceNumber32 m_priorRcvNxt;       //!< Sequence number of the first missing byte in data
    bool m_priorRcvNxtFlag;               //!< Variable used in setting the value of m_priorRcvNxt for first time

    bool m_congestionState;
    bool m_delayedAckReserved;

};

}

#endif
