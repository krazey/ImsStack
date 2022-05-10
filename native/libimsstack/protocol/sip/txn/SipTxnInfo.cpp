#include "txn/SipTxn.h"
#include "txn/SipTxnInfo.h"

SipTxnInfo::SipTxnInfo() :
        m_pSendSipMsg(SIP_NULL),
        m_pUserData(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        eTxnStatus(SipTxn::STATUS_INVALID),
        bTxnTerminated(SIP_FALSE),
        bTxnCreated(SIP_FALSE)
{
}

SipTxnInfo::~SipTxnInfo()
{
    if (m_pSendSipMsg != SIP_NULL)
    {
        m_pSendSipMsg->SipDelete();
    }
}
