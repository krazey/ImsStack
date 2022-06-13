#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"

SipTxnFsmData::SipTxnFsmData(
        SipMessage* pSipMsg, SipTransportParameter* pTranspParam, ISipUserData* pUserData) :
        m_pSipMsgIn(pSipMsg),
        m_pTranspParam(pTranspParam),
        m_pUserData(pUserData),
        m_pSendSipMsg(SIP_NULL),
        eTxnStatus(SipTxn::STATUS_INVALID),
        m_pOutUserData(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        bTxnTerminated(SIP_FALSE),
        bTxnCreated(SIP_FALSE)
{
}

SipTxnFsmData::~SipTxnFsmData() {}
