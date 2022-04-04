/*
   Author
   <table>
   date      author                description
   --------  --------------        ----------
   20170110  vijay.nair@           Created
   </table>

   Description

 */

#include "txn/SipTxn.h"
#include "txn/SipTimeoutData.h"
#include "sip_error.h"

SipTimeoutData::SipTimeoutData()
{
    m_eTxnType = SipTxn::INVALID_TXN;
    m_eTimerType = SipTxn::TIMER_TYPE_INVALID;
    m_pTxnKey = SIP_NULL;
}

SipTimeoutData::SipTimeoutData(SIP_INT32 eTxnType, SIP_INT32 eTimerType, SipTxnKey* pTxnKey)
{
    m_eTxnType = eTxnType;
    m_eTimerType = eTimerType;

    SIP_UINT16 nError;
    m_pTxnKey = new SipTxnKey(pTxnKey, &nError);
    if (E_ERR_PF_MALLOCFAILED == nError)
    {
        delete m_pTxnKey;
        m_pTxnKey = SIP_NULL;
    }
}

SipTimeoutData::~SipTimeoutData()
{
    delete m_pTxnKey;
    m_pTxnKey = SIP_NULL;
}

SipTxnKey *SipTimeoutData::GetTxnKey() const
{
    return m_pTxnKey;
}

SIP_INT32 SipTimeoutData::GetTimerType() const
{
    return m_eTimerType;
}

SIP_INT32 SipTimeoutData::GetTxnType() const
{
    return m_eTxnType;
}

SIP_BOOL SipTimeoutData::SetTxnKey(SipTxnKey* pTxnKey)
{
    this->m_pTxnKey = pTxnKey;
    return SIP_TRUE;
}

SIP_BOOL SipTimeoutData::SetTimerType(SIP_INT32 eTimerType)
{
    this->m_eTimerType = eTimerType;
    return SIP_TRUE;
}

SIP_BOOL SipTimeoutData::SetTxnType(SIP_INT32 eTxnType)
{
    this->m_eTxnType = eTxnType;
    return SIP_TRUE;
}
