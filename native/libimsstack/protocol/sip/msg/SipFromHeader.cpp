#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_error.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipFromHeader.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipFromHeader::SipFromHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipFromHeader::SipFromHeader() :
        SipNameAddrHeader(SipHeaderBase::FROM)
{
}

/******************************************************************************
 * Function name      : SipFromHeader::SipFromHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipFromHeader::SipFromHeader(const SipFromHeader& objHeader) :
        SipNameAddrHeader(objHeader)
{
}
/******************************************************************************
 * Function name      : SipFromHeader::~SipFromHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipFromHeader::~SipFromHeader()
{
    /*    if (m_pszDispName)
          delete m_pszDispName;
     */
}

/******************************************************************************
 * Function name      : SipFromHeader::GetTag
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_CHAR* SipFromHeader::GetTag()
{
    SipParameters* pParameters = GetParameters();

    if (pParameters == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipParameterList* pParameterList = pParameters->GetParameterList();

    if (pParameterList != SIP_NULL)
    {
        return pParameterList->GetParamValue("tag");
    }
    return SIP_NULL;
}

/******************************************************************************
 * Function name      : SipFromHeader::SetTag
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipFromHeader::SetTag(SIP_CHAR* pszFromTag)
{
    SipParameters* pParameters = GetParameters();

    if (pParameters == SIP_NULL)
    {
        InitParameters(SIP_NULL);
        pParameters = GetParameters();
    }

    if (pParameters == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pParameters->AddParam("tag", pszFromTag) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Set Tag failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SipHeaderBase* SipFromHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipFromHeader(*reinterpret_cast<SipFromHeader*>(pHeader));
    }
    return new SipFromHeader();
}
