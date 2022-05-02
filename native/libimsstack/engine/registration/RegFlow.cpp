/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090911  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceSystemTime.h"
#include "SipDebug.h"
#include "SipFactory.h"
#include "RegFlow.h"

__IMS_TRACE_TAG_REG__;



PUBLIC
RegFlow::RegFlow(IN const RegKey& objRegKey_)
    : objRegKey(objRegKey_)
    , strCallId(AString::ConstNull())
    , nCSeqValue(0)
    , nSubscriber(NO_SUBSCRIBER)
    , strSessionId(AString::ConstNull()) // HEADER_REQ_SESSION-ID
{
    SIPFactory::GenerateCallId(AString::ConstNull(), strCallId);

    // HEADER_REQ_SESSION-ID
    SIPFactory::GenerateSessionId(objRegKey.GetSlotId(), strCallId, strSessionId);
}

PUBLIC
RegFlow::RegFlow(IN const RegFlow &objRHS)
    : objRegKey(objRHS.objRegKey)
    , strCallId(objRHS.strCallId)
    , nCSeqValue(objRHS.nCSeqValue)
    , nSubscriber(objRHS.nSubscriber)
    , strSessionId(objRHS.strSessionId) // HEADER_REQ_SESSION-ID
{
}

PUBLIC
RegFlow::~RegFlow()
{
    IMS_TRACE_D("Destructor :: %X, %s, %u",
            nSubscriber, SIPDebug::GetCharA1(strCallId.GetStr(), 8, '@'), nCSeqValue);
}

/*

Remarks

*/
PUBLIC
RegFlow& RegFlow::operator=(IN const RegFlow &objRHS)
{
    if (this != &objRHS)
    {
        objRegKey = objRHS.objRegKey;

        strCallId = objRHS.strCallId;
        nCSeqValue = objRHS.nCSeqValue;

        nSubscriber = objRHS.nSubscriber;

        // HEADER_REQ_SESSION-ID
        strSessionId = objRHS.strSessionId;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegFlow::Capture(IN IMS_UINT32 nSubscriber_ /* = DEFAULT_SUSCRIBER */)
{
    if (nSubscriber != NO_SUBSCRIBER)
    {
        return IMS_FALSE;
    }

    nSubscriber = nSubscriber_;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
const AString& RegFlow::GetCallId() const
{
    return strCallId;
}

/*

Remarks

*/
PUBLIC
const RegKey& RegFlow::GetRegKey() const
{
    return objRegKey;
}

/*

Remarks
 HEADER_REQ_SESSION-ID
*/
PUBLIC
const AString& RegFlow::GetSessionId() const
{
    return strSessionId;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegFlow::IncreaseNGetCSeqValue(IN IMS_SINT32 nIncrement /* = 1 */)
{
    if (nIncrement == 0)
    {
        return nCSeqValue;
    }

    nCSeqValue += nIncrement;

    return nCSeqValue;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegFlow::IsReserved(OUT IMS_UINT32 *pnSubscriber_ /* = IMS_NULL */) const
{
    if (pnSubscriber_ != IMS_NULL)
    {
        *pnSubscriber_ = nSubscriber;
    }

    return (nSubscriber != NO_SUBSCRIBER) ? IMS_TRUE : IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
void RegFlow::Release()
{
    nSubscriber = NO_SUBSCRIBER;
}

/*

Remarks

*/
PUBLIC
void RegFlow::Restore()
{
    SIPFactory::GenerateCallId(AString::ConstNull(), strCallId);

    nSubscriber = NO_SUBSCRIBER;
    nCSeqValue = 0;

    // HEADER_REQ_SESSION-ID
    SIPFactory::GenerateSessionId(objRegKey.GetSlotId(), strCallId, strSessionId);
}

/*

Remarks

*/
PUBLIC
void RegFlow::SetCSeqValue(IN IMS_SINT32 nValue)
{
    nCSeqValue = nValue;
}

/*

Remarks

*/
PUBLIC
void RegFlow::UpdateCallId(IN const IPAddress &objIP)
{
    // Check if the Call-ID already contains '@' character
    if (strCallId.Contains('@'))
    {
        return;
    }

    if (objIP.IsIPv4Address() || objIP.IsIPv6Address())
    {
        strCallId.Append('@');
        strCallId.Append(objIP.ToString());

        // HEADER_REQ_SESSION-ID
        SIPFactory::GenerateSessionId(objRegKey.GetSlotId(), strCallId, strSessionId);
    }
}
