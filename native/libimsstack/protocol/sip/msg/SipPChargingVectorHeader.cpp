#include "msg/SipPChargingVectorHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

SipPChargingVectorHeader::SipPChargingVectorHeader() :
        SipHeaderBase(SipHeaderBase::P_CHARGING_VECTOR),
        m_pChargingVectorList(SIP_NULL)
{
}

SipPChargingVectorHeader::SipPChargingVectorHeader(const SipPChargingVectorHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pChargingVectorList(SIP_NULL)
{
    if (objHeader.m_pChargingVectorList != SIP_NULL)
    {
        m_pChargingVectorList = new SipNameValue(*(objHeader.m_pChargingVectorList));
    }
}

/*destructor*/
SipPChargingVectorHeader::~SipPChargingVectorHeader()
{
    if (m_pChargingVectorList != SIP_NULL)
    {
        m_pChargingVectorList->SipDelete();
    }
}

/*virtual methods*/
/*Function for encoding of headers*/
SIP_BOOL SipPChargingVectorHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pChargingVectorList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipPChargingVectorHeader::EncodeHdr:m_pChargingVectorList missing", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargingVectorList->EncodeFromList(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipPChargingVectorHeader::EncodeHdr: Name Value Encoding failed", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

/******************************************************************************
 * Function name      : SipPChargingVectorHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPChargingVectorHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "SipPChargingVectorHeader::DecodeHdr", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;
    /*Header value is the first node of SipParameterList
      and the other Node will contain SIP parameter*/
    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipPChargingVectorHeader::DecodeHdr: Hdr Prm Decoding Failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }

    /*Decode the Header Value*/
    m_pChargingVectorList = new SipNameValue(GetHdrType());
    if (m_pChargingVectorList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargingVectorList->DecHdrNameVal(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Name Value Decoding Successful", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    // charge vector should be icid-value
    if (SipPf_Stricmp("icid-value", m_pChargingVectorList->m_pszName) != SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Name Value Decoding Successful", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPChargingVectorHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPChargingVectorHeader(*reinterpret_cast<SipPChargingVectorHeader*>(pHeader));
    }
    return new SipPChargingVectorHeader();
}
