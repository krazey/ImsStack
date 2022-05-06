/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140304  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_IPSEC_STATE_H_
#define _SIP_IPSEC_STATE_H_

#include "EngineActivity.h"
#include "SIPStackHeaders.h"
#include "SIPTxnKey.h"
#include "SIPTransportAddress.h"
#include "ISipIpSecState.h"

class SIPIPSecState : public EngineActivity, public ISipIpSecState
{
private:
    class SA
    {
    public:
        SA();
        SA(IN CONST IPAddress& objIP_U_, IN IMS_SINT32 nPort_UC_, IN IMS_SINT32 nPort_US_,
                IN CONST IPAddress& objIP_P_, IN IMS_SINT32 nPort_PC_, IN IMS_SINT32 nPort_PS_);
        SA(IN CONST SA& objRHS);
        ~SA();

    private:
        SA& operator=(IN CONST SA& objRHS);

    public:
        IMS_BOOL AddTransaction(IN CONST SIPTxnKey* pTxnKey);
        IMS_BOOL CheckIPAddress(IN CONST SIPTransportAddress& objNearEnd,
                IN CONST SIPTransportAddress& objFarEnd) const;
        IMS_SINT32 GetSA(IN CONST SIPTransportAddress& objNearEnd,
                IN CONST SIPTransportAddress& objFarEnd, IN IMS_SINT32 nDirection) const;
        IMS_SINT32 GetState() const;
        IMS_BOOL HasPendingTransaction() const;
        IMS_BOOL RemoveTransaction(IN CONST SIPTxnKey* pTxnKey);
        void SetState(IN IMS_SINT32 nState);

    private:
        static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

    public:
        // Direction of SA
        enum
        {
            SA_IN = 1,
            SA_OUT = 2
        };

        // SA pairs
        enum
        {
            // src_port, dst_port, transport, direction
            SA_START = 1,

            SA_PUC_PPS_U_OUT = SA_START,
            SA_PUC_PPS_T_OUT,
            SA_PUS_PPC_T_OUT,
            SA_PPC_PUS_U_IN,
            SA_PPC_PUS_T_IN,
            SA_PPS_PUC_T_IN,

            SA_END
        };

        IPAddress objIP_U;
        IMS_SINT32 nPort_UC;
        IMS_SINT32 nPort_US;
        IPAddress objIP_P;
        IMS_SINT32 nPort_PC;
        IMS_SINT32 nPort_PS;

        IMS_SINT32 nState;
        // For tracking SIP transaction
        IMSList<SIPTxnKey> objSAStat;
    };

public:
    SIPIPSecState();
    virtual ~SIPIPSecState();

private:
    SIPIPSecState(IN CONST SIPIPSecState& objRHS);
    SIPIPSecState& operator=(IN CONST SIPIPSecState& objRHS);

public:
    IMS_BOOL IsIPSecEnabled() const;
    void NotifyMessageReceived(IN CONST SIPTransportAddress& objNearEnd,
            IN CONST SIPTransportAddress& objFarEnd, IN SipMessage* pstMessage);
    void NotifyMessageSent(IN CONST SIPTransportAddress& objNearEnd,
            IN CONST SIPTransportAddress& objFarEnd, IN SipMessage* pstMessage);
    void NotifyMessageSentFailed(IN SipMessage* pstMessage);
    void NotifyTransactionAborted(IN SipTxnKey* pstTxnKey);

private:
    // EngineActivity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // ISipIpSecState class
    virtual void ClearIpSecSa(IN IMS_SINT32 nSAType);
    virtual IMS_SINT32 GetState(IN IMS_SINT32 nSAType) const;
    virtual IMS_BOOL HasPendingTransaction(IN IMS_SINT32 nSAType) const;
    virtual void SetIpSecSa(IN IMS_SINT32 nSAType, IN CONST IPAddress& objIP_U,
            IN IMS_SINT32 nPort_UC, IN IMS_SINT32 nPort_US, IN CONST IPAddress& objIP_P,
            IN IMS_SINT32 nPort_PC, IN IMS_SINT32 nPort_PS);
    virtual void SetListener(IN ISipIpSecStateListener* piListener);

    void NotifyMessageReceivedInternal(IN CONST SIPTransportAddress& objNearEnd,
            IN CONST SIPTransportAddress& objFarEnd, IN SIPTxnKey* pTxnKey);
    void NotifyMessageSentInternal(IN CONST SIPTransportAddress& objNearEnd,
            IN CONST SIPTransportAddress& objFarEnd, IN SIPTxnKey* pTxnKey);
    void NotifyTransactionAbortedInternal(IN SIPTxnKey* pTxnKey);

private:
    // Event for message processing
    enum
    {
        AMSG_NOTIFY_STATE_CHANGED = AMSG_USER
    };

    SA* pNewSA;
    SA* pOldSA;

    ISipIpSecStateListener* piListener;
};

#endif  // _SIP_IPSEC_STATE_H_
