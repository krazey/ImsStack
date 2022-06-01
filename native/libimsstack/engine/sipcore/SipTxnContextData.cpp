/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090725  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SipPrivate.h"
#include "SipTxnContextData.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPTxnContextData::SIPTxnContextData() :
        pTxnState(IMS_NULL)
{
}

PUBLIC
SIPTxnContextData::~SIPTxnContextData()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: SIPTxnContextData (%" PFLS_x ")",
            pTxnState.IsNull() ? 0 : pTxnState.Get(), 0, 0);

    pTxnState = IMS_NULL;
}

PUBLIC
const SipMethod& SIPTxnContextData::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    return objMethod;
}

PUBLIC
SIPTransactionState* SIPTxnContextData::GetTxnState() const
{
    //---------------------------------------------------------------------------------------------

    return pTxnState.Get();
}

PUBLIC
void SIPTxnContextData::SetMethod(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    this->objMethod = objMethod;
}

PUBLIC
void SIPTxnContextData::SetTxnState(IN SIPTransactionState* pTxnState)
{
    //---------------------------------------------------------------------------------------------

    this->pTxnState = pTxnState;
}
