#include "msg/SipGeolocationRoutingHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipGeolocationRoutingHeader::SipGeolocationRoutingHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipGeolocationRoutingHeader::SipGeolocationRoutingHeader() :
        SipHeaderBase(SipHeaderBase::GEOLOCATION_ROUTING),
        m_pGeoLocationRoutingList(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipGeolocationRoutingHeader::SipGeolocationRoutingHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipGeolocationRoutingHeader::SipGeolocationRoutingHeader(
        const SipGeolocationRoutingHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pGeoLocationRoutingList(SIP_NULL)
{
    if (objHeader.m_pGeoLocationRoutingList != SIP_NULL)
    {
        m_pGeoLocationRoutingList = new SipNameValue(*(objHeader.m_pGeoLocationRoutingList));
    }
}

/******************************************************************************
 * Function name      : SipGeolocationRoutingHeader::~SipGeolocationRoutingHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipGeolocationRoutingHeader::~SipGeolocationRoutingHeader()
{
    if (m_pGeoLocationRoutingList != SIP_NULL)
    {
        m_pGeoLocationRoutingList->SipDelete();
    }
}

/******************************************************************************
 * Function name      : SipGeolocationRoutingHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipGeolocationRoutingHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_pGeoLocationRoutingList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "GeoLocation route missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return m_pGeoLocationRoutingList->EncodeFromList(ppCurrPos);
}

SIP_BOOL SipGeolocationRoutingHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pGeoLocationRoutingList = new SipNameValue(GetHdrType());
    if (m_pGeoLocationRoutingList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    return m_pGeoLocationRoutingList->DecHdrNameVal(pStartPt, pEndPt);
}

SipHeaderBase* SipGeolocationRoutingHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipGeolocationRoutingHeader(
                *reinterpret_cast<SipGeolocationRoutingHeader*>(pHeader));
    }
    return new SipGeolocationRoutingHeader();
}
