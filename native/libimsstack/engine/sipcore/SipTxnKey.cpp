/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140321  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SipTxnKey.h"

PUBLIC
SIPTxnKey::SIPTxnKey() :
        objMethod(SipMethod::INVALID),
        nStatusCode(0),
        strViaBranch(AString::ConstNull()),
        nCSeq(0),
        nExtraInt(0),
        strExtraString(AString::ConstNull())
{
}

PUBLIC
SIPTxnKey::SIPTxnKey(IN CONST SipMethod& objMethod_, IN IMS_SINT32 nStatusCode_,
        IN CONST AString& strViaBranch_, IN IMS_UINT32 nCSeq_) :
        objMethod(objMethod_),
        nStatusCode(nStatusCode_),
        strViaBranch(strViaBranch_),
        nCSeq(nCSeq_),
        nExtraInt(0),
        strExtraString(AString::ConstNull())
{
}

PUBLIC
SIPTxnKey::SIPTxnKey(IN CONST SIPTxnKey& objRHS) :
        objMethod(objRHS.objMethod),
        nStatusCode(objRHS.nStatusCode),
        strViaBranch(objRHS.strViaBranch),
        nCSeq(objRHS.nCSeq),
        nExtraInt(objRHS.nExtraInt),
        strExtraString(objRHS.strExtraString)
{
}

PUBLIC
SIPTxnKey::~SIPTxnKey() {}

PUBLIC
SIPTxnKey& SIPTxnKey::operator=(IN CONST SIPTxnKey& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        objMethod = objRHS.objMethod;
        nStatusCode = objRHS.nStatusCode;
        strViaBranch = objRHS.strViaBranch;
        nCSeq = objRHS.nCSeq;

        nExtraInt = objRHS.nExtraInt;
        strExtraString = objRHS.strExtraString;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPTxnKey::Equals(IN CONST SIPTxnKey* pKey) const
{
    //---------------------------------------------------------------------------------------------

    if (pKey == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objMethod.Equals(pKey->objMethod) || (nCSeq != pKey->nCSeq) ||
            !strViaBranch.Equals(pKey->strViaBranch))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPTxnKey::GetExtraInt() const
{
    //---------------------------------------------------------------------------------------------

    return nExtraInt;
}

/*

Remarks

*/
PUBLIC
const AString& SIPTxnKey::GetExtraString() const
{
    //---------------------------------------------------------------------------------------------

    return strExtraString;
}

/*

Remarks

*/
PUBLIC
const SipMethod& SIPTxnKey::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    return objMethod;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPTxnKey::GetCSeq() const
{
    //---------------------------------------------------------------------------------------------

    return nCSeq;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPTxnKey::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    return nStatusCode;
}

/*

Remarks

*/
PUBLIC
const AString& SIPTxnKey::GetViaBranch() const
{
    //---------------------------------------------------------------------------------------------

    return strViaBranch;
}

/*

Remarks

*/
PUBLIC
void SIPTxnKey::SetExtraInt(IN IMS_SINT32 nExtraInt)
{
    //---------------------------------------------------------------------------------------------

    this->nExtraInt = nExtraInt;
}

/*

Remarks

*/
PUBLIC
void SIPTxnKey::SetExtraString(IN CONST AString& strExtraString)
{
    //---------------------------------------------------------------------------------------------

    this->strExtraString = strExtraString;
}
