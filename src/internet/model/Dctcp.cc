#include <iostream>
#include "Dctcp.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"
#include "math.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/sequence-number.h"
#include "ns3/double.h"
#include "ns3/nstime.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("Dctcp");
NS_OBJECT_ENSURE_REGISTERED(Dctcp);

TypeId TcpDctcp::GetTypeId(void)
{
    static TypeId tid = TypeId ( "ns3::Dctcp" )
        .SetParent<TcpNewReno> ()
        .AddConstructor<Dctcp>()
        .SetGroupName("Internet")
        .AddAttribute("g",
                      "Parameter G for updating dctcp_alpha",
                      DoubleValue(0.0625),
                      MakeDoubleAccessor(&Dctcp::m_g),
                      MakeDoubleChecker<double>(0))
        .AddAttribute("DctcpAlphaOnInit",
                      "Parameter alpha initial value",
                      DoubleValue(0.0),
                      MakeDoubleAccessor(&Dctcp::SetDctcpAlpha),
                      MakeDoubleChecker<double>(0))
        ;
    return tid;
}
Dctcp::Dctcp():TcpNewReno()
{
    NS_LOG_FUNCTION(this);
    m_delayedAckReserved = false;
    m_ackedBytesCongestion = 0;
    m_ackedBytesTotal = 0;
    m_nextWinSeqFlag = false; 

    m_ceState = false;
    m_prioeRcvNxtFlag = false;
}

Dctcp::Dctcp(const Dctcp& sock):TcpNewReno(sock)
{
    NS_LOG_FUNCTION(this);
    m_delayedAckReserved = (sock.m_delayedAckReserved);
    m_ceState = (sock.m_ceState);
}

Dctcp::~Dctcp(void)
{
    NS_LOG_FUNCTION(this);
}

Ptr<TcpCongestionOps> Dctcp::Fork(void)
{
    NS_LOG_FUNCTION(this);
    return CopyObject<Dctcp>(this);
}
void Dctcp::ReduceCwnd(Ptr<TcpSocketState>)
{
    NS_LOG_FUNCTION(this << tcb);
    uint32_t val = (int)((1 - m_alpha / 2.0) * tcb->m_cWnd);
    //max
    tcb->m_cWnd = std::max(val, 2 * tcb->m_segementSize);
}

void Dctcp::PktsAcked(Ptr<TcpSocketState> tcb,
                      uint32_t segementsAcked,
                      const Time &rtt)
{
    NS_LOG_FUNCTION(this << tcb << segementsAcked << rtt << m_ackedBytesTotal << m_ackedBytesCongestion << tcb->m_ecnState << m_alpha);
    m_ackedBytesTotal +=segementsAcked * tcb->m_segmentSize;
    if(tcb->m_ackedBytesCongestion == TcpSocketState::ECN_CE_RCVD)
    {
        m_ackedBytesCongestion += segementsAcked * tcb->m_segementSize;
    }
    if (tcb->m_lastAckedSeq >= m_nextWinSeq)  // if last received ack num larger than first byte of next window, update parameter
    {
        double EcnRatio;
        if(m_ackedBytesCongestion > 0)
        {
            EcnRatio = (double)m_ackedBytesCongestion / m_ackedBytesTotal;
        }
        else 
        {
            EcnRatio = 0;
        }
        m_alpha = (1.0 - m_g) *m_alpha + m_g * EcnRatio;
        Reset(tcb);
    }
}

void Dctcp::SetDctcpAlpha(double alpha)
{
    NS_LOG_FUNCTION(this << alpha);
    m_alpha = alpha;
}

void Dctcp::Reset(Ptr<TcpSocketState> tcb)
{
    // set next window first seq num, and clear window total byte and congestion byte statutic
    NS_LOG_FUNCTION(this << tcb);
    m_nextWinSeq = tcb->m_nextTxSequence;
    m_ackedBytesCongestion = 0;
    m_ackedBytesTotal = 0;
}

void Dctcp::CeState0to1(Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this << tcb);
    if(!m_ceState && m_delayedAckReserved)
    {
        //next SequenceNumber 
        SequenceNumber32 tmpRcvNxt;
        tmpRcvNxt = tcb->m_rxBuffer->NextRxSequence();

        // send ack for previous packet that with ece flags.
        tcb->m_rxBuffer->SetNextRxSequence(m_priorRcvNxt);
        SendEmptyPacket(TcpHeader::ack);

        // save practically current rcv_nxt.
        tcb->m_rxBuffer->SetNextRxSequence(tmpRcvNxt);
    }

    //this is test file 02:00 am may 27 
    
    m_priorRcvNxt = tcb->m_rxBuffer->NextRxSequence();
    m_ceState = true;
    tcb->m_ecnState = TcpSocketState::ECN_CE_RCVD;
}

void Dctcp::CeState1to0 (Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this << tcb);
    if(m_ceState && m_delayedAckReserved)
    {
        SequenceNumber32 tmpRcvNxt;
        tmpRcvNxt = tcb->m_rxBuffer->NextRxSequence();

        tcb->m_rxBuffer->SetNextRxSequence(m_priorRcvNxt);
        SendEmptyPacket(TcpHeader::ack|TcpHeader::ece);

        tcb->m_rxBuffer->SetNextRxSequence(tmpRcvNxt);
    }

    m_priorRcvNxt = tcb->m_rxBuffer->NextRxSequence();
    m_ceState = false;
    if(tcb->m_ecnState == TcpSocketState::ECN_CE_RCVD || tcb->m_ecnState == TcpSocketState::ECN_SENDING_ECE)
    {
        tcb->m_ecnState = TcpSocketState::ECN_IDLE;
    }
}

void UpdateAckReserved(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCAEvent_t event)
{
    NS_LOG_FUNCTION(this << tcb << event);
    switch (event)
    {
    case TcpSocketState::CA_EVENT_DELAYED_ACK:
        if(!m_delayedAckReserved)
        {
            m_delayedAckReserved = true;
        }
        break;
    case TcpSocketState::CA_EVENT_NON_DELAYED_ACK:
        if(m_delayedAckReserved)
        {
            m_delayedAckReserved = false;
        }
        break;
    }
}

void Dctcp::CwndEvent(Ptr<TcpSocketState> tcb,
                      const TcpSocketState::TcpCAEvent_t event)
{
    NS_LOG_FUNCTION(this << tcb << event << tcb->m_ecnState << tcb->m_nextTxSequence);
    switch(event)
    {
    case TcpSocketState::CA_EVENT_ECN_IS_CE:
        CeState0to1(tcb);
        break;
    case TcpSocketState::CA_EVENT_ECN_NO_CE:
        CeState1to0(tcb);
        break;
    case TcpSocketState::CA_EVENT_DELAYED_ACK:
    case TcpSocketState::CA_EVENT_NON_DELAYED_ACK:
        UpdateAckReserved(tcb, event);
        break;
    default:
        break;
    }
}
}

