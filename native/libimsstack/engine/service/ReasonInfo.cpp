/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090702  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ReasonInfo.h"

PUBLIC
ReasonInfo::ReasonInfo() :
        nType(REASON_TYPE_NONE)
{
}

PUBLIC
ReasonInfo::ReasonInfo(IN IMS_SINT32 nType_) :
        nType(nType_)
{
}

PUBLIC
ReasonInfo::ReasonInfo(IN IMS_SINT32 nType_, IN CONST SipStatusCode& objStatusCode_) :
        nType(nType_),
        objStatusCode(objStatusCode_)
{
}

PUBLIC
ReasonInfo::~ReasonInfo() {}

PUBLIC VIRTUAL const AString& ReasonInfo::GetReasonPhrase() const
{
    //---------------------------------------------------------------------------------------------

    if (nType != REASON_TYPE_RESPONSE)
        return AString::ConstNull();  // Throw exception

    return objStatusCode.GetReasonPhrase();
}

PUBLIC VIRTUAL IMS_SINT32 ReasonInfo::GetReasonType() const
{
    //---------------------------------------------------------------------------------------------

    return nType;
}

PUBLIC VIRTUAL IMS_SINT32 ReasonInfo::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    if (nType != REASON_TYPE_RESPONSE)
        return (-1);  // Throw exception

    return objStatusCode.ToInt();
}

PUBLIC
void ReasonInfo::SetReasonType(IN IMS_SINT32 nType)
{
    //---------------------------------------------------------------------------------------------

    this->nType = nType;
}

PUBLIC
void ReasonInfo::SetStatusCode(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    this->objStatusCode = nStatusCode;
    // ?????
    this->objStatusCode = SipStatusCode::GetReasonPhrase(nStatusCode);
}
