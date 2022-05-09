#include "msg/SipHeaderBase.h"
#include "msg/SipAcceptContactHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/*constructor*/
SipAcceptContactHeader::SipAcceptContactHeader() :
        SipHeaderBase(SipHeaderBase::ACCEPT_CONTACT)
{
}

/*constructor*/
SipAcceptContactHeader::SipAcceptContactHeader(const SipAcceptContactHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

/*destructor*/
SipAcceptContactHeader::~SipAcceptContactHeader() {}

/*virtual methods*/
SIP_BOOL SipAcceptContactHeader::SetValue(const SIP_CHAR* pszContact)
{
    if (pszContact && (SipPf_Stricmp(pszContact, "*") == 0))
    {
        return SipHeaderBase::SetValue(pszContact);
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipAcceptContactHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAcceptContactHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
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

SipHeaderBase* SipAcceptContactHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipAcceptContactHeader(*reinterpret_cast<SipAcceptContactHeader*>(pHeader));
    }
    return new SipAcceptContactHeader();
}
