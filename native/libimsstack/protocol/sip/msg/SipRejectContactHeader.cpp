#include "msg/SipHeaderBase.h"
#include "msg/SipRejectContactHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/*constructor*/
SipRejectContactHeader::SipRejectContactHeader() :
        SipHeaderBase(SipHeaderBase::REJECT_CONTACT)
{
}

/*constructor*/
SipRejectContactHeader::SipRejectContactHeader(const SipRejectContactHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

/*destructor*/
SipRejectContactHeader::~SipRejectContactHeader() {}

SIP_BOOL SipRejectContactHeader::SetValue(const SIP_CHAR* pszContact)
{
    if (pszContact && (SipPf_Strcmp(pszContact, "*") == 0))
    {
        return SipHeaderBase::SetValue(pszContact);
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipRejectContactHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipRejectContactHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue != SIP_NULL) && (SipPf_Strlen(pszValue) == SIP_ONE) && IS_ASTERISK(*pszValue))
    {
        return SIP_TRUE;
    }

    return SIP_FALSE;
}
SipHeaderBase* SipRejectContactHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipRejectContactHeader(*reinterpret_cast<SipRejectContactHeader*>(pHeader));
    }
    return new SipRejectContactHeader();
}
