#include "msg/SipIntegerHeader.h"
#include "msg/sip_msgutil.h"
#include "platform/sip_pf_string.h"
#include "sip_debug.h"
#include "sip_error.h"
#include "SipTrace.h"

#define MAX_LEN 20

SipIntegerHeader::SipIntegerHeader(SIP_INT32 eHeaderType) :
        SipHeaderBase(eHeaderType)
{
}

SipIntegerHeader::SipIntegerHeader(const SipIntegerHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipIntegerHeader::~SipIntegerHeader() {}

SIP_BOOL SipIntegerHeader::SetValueInt(const SIP_UINT32 nValue)
{
    if (nValue > MAX_MAXFD)
    {
        if (GetHdrType() == SipHeaderBase::MAX_FORWARDS)
        {
            return SIP_FALSE;
        }
    }

    if (nValue > MAX_ERROR_CODE)
    {
        if (GetHdrType() == SipHeaderBase::GEOLOCATION_ERROR)
        {
            return SIP_FALSE;
        }
    }

    SIP_CHAR szValue[MAX_LEN];
    SipPf_Sprintf(szValue, (SIP_CHAR*)"%u", nValue);
    return SetValue(szValue);
}

SIP_UINT32 SipIntegerHeader::GetValueInt() const
{
    return SipPf_Atoi(GetValue());
}

SIP_BOOL SipIntegerHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams)
{
    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue == SIP_NULL) && (GetHdrType() == SipHeaderBase::CONTENT_LENGTH))
    {
        SetValue("0");
    }
    return SipHeaderBase::EncodeHdr(ppCurrPos, bParams);
}

SIP_BOOL SipIntegerHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue != SIP_NULL) && (SipPf_Atoi_IsZero(pszValue) == SIP_FALSE))
    {
        SIP_UINT32 nValue = SipPf_Atoi(pszValue);
        SIP_INT32 eHeaderType = GetHdrType();
        if ((eHeaderType == SipHeaderBase::MAX_FORWARDS) && (nValue > MAX_MAXFD))
        {
            return SIP_FALSE;
        }
        else if ((eHeaderType == SipHeaderBase::EXPIRES_SEC) && (nValue > MAX_EXPIRES))
        {
            return SIP_FALSE;
        }
        else if ((eHeaderType == SipHeaderBase::GEOLOCATION_ERROR) && (nValue > MAX_ERROR_CODE))
        {
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipIntegerHeader::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipIntegerHeader(*reinterpret_cast<SipIntegerHeader*>(pHeader));
    }
    return new SipIntegerHeader(eHdr);
}
