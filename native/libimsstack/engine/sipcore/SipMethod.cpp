/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SipMethod.h"



PUBLIC GLOBAL
const IMS_CHAR* SIPMethod::NAME[] =
{
    "ACK",
    "BYE",
    "CANCEL",
    "INVITE",
    "OPTIONS",
    "REGISTER",
    "PRACK",
    "SUBSCRIBE",
    "NOTIFY",
    "UPDATE",
    "MESSAGE",
    "REFER",
    "PUBLISH",
    "INFO",
    "UNKNOWN"
};

PUBLIC GLOBAL
const SIPMethod SIPMethod::INVALID_METHOD;



PUBLIC
SIPMethod::SIPMethod(IN CONST IMS_SINT32 nMethod_ /* = SIPMethod::INVALID */)
    : nMethod(nMethod_)
{
    //---------------------------------------------------------------------------------------------

    strMethod = ConvertMethodToString(nMethod_);
}

PUBLIC
SIPMethod::SIPMethod(IN CONST IMS_CHAR *pszMethod_)
    : strMethod(pszMethod_)
{
    //---------------------------------------------------------------------------------------------

    nMethod = ConvertStringToMethod(strMethod);
}

PUBLIC
SIPMethod::SIPMethod(IN CONST AString &strMethod_)
    : strMethod(strMethod_)
{
    //---------------------------------------------------------------------------------------------

    nMethod = ConvertStringToMethod(strMethod);
}

PUBLIC
SIPMethod::SIPMethod(IN CONST SIPMethod &objRHS)
    : nMethod(objRHS.nMethod)
    , strMethod(objRHS.strMethod)
{
}

PUBLIC
SIPMethod::~SIPMethod()
{
}

PUBLIC
SIPMethod& SIPMethod::operator=(IN CONST SIPMethod &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nMethod = objRHS.nMethod;
        strMethod = objRHS.strMethod;
    }

    return (*this);
}

PUBLIC
SIPMethod& SIPMethod::operator=(IN IMS_SINT32 nMethod_)
{
    //---------------------------------------------------------------------------------------------

    if (nMethod == nMethod_)
        return (*this);

    nMethod = nMethod_;
    strMethod = ConvertMethodToString(nMethod);

    return (*this);
}

PUBLIC
SIPMethod& SIPMethod::operator=(IN CONST IMS_CHAR *pszMethod_)
{
    //---------------------------------------------------------------------------------------------

    if (strMethod.Equals(pszMethod_))
        return (*this);

    strMethod = pszMethod_;
    nMethod = ConvertStringToMethod(strMethod);

    return (*this);
}

PUBLIC
SIPMethod& SIPMethod::operator=(IN CONST AString &strMethod_)
{
    //---------------------------------------------------------------------------------------------

    if (strMethod.Equals(strMethod_))
        return (*this);

    strMethod = strMethod_;
    nMethod = ConvertStringToMethod(strMethod);

    return (*this);
}

PUBLIC GLOBAL
const IMS_CHAR* SIPMethod::ToName(IN IMS_SINT32 nMethod)
{
    //---------------------------------------------------------------------------------------------

    return ((nMethod > INVALID) && (nMethod < MAX)) ? NAME[nMethod] : "";
}

PRIVATE GLOBAL
IMS_SINT32 SIPMethod::ConvertStringToMethod(IN CONST AString &strMethod)
{
    //---------------------------------------------------------------------------------------------

    if (strMethod.IsNULL() || strMethod.IsEmpty())
        return SIPMethod::INVALID;
    else
    {
        for (IMS_SINT32 i = (SIPMethod::INVALID + 1); i < SIPMethod::MAX; i++)
        {
            if (strMethod.Equals(SIPMethod::NAME[i]))
            {
                return i;
            }
        }
    }

    return SIPMethod::INVALID;
}

PRIVATE GLOBAL
AString SIPMethod::ConvertMethodToString(IN IMS_SINT32 nMethod)
{
    //---------------------------------------------------------------------------------------------

    if ((nMethod > SIPMethod::INVALID) && (nMethod < SIPMethod::MAX))
        return AString(NAME[nMethod]);

    return AString::ConstNull();
}
