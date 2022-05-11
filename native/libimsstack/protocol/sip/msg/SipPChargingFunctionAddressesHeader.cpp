#include "msg/SipPChargingFunctionAddressesHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

SipPChargingFunctionAddressesHeader::SipPChargingFunctionAddressesHeader() :
        SipHeaderBase(SipHeaderBase::P_CHRG_FUN_ADDR),
        m_pChargeAddr(SIP_NULL)
{
}

SipPChargingFunctionAddressesHeader::SipPChargingFunctionAddressesHeader(
        const SipPChargingFunctionAddressesHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pChargeAddr(SIP_NULL)
{
    if (objHeader.m_pChargeAddr != SIP_NULL)
    {
        m_pChargeAddr = new SipNameValue(*(objHeader.m_pChargeAddr));
    }
}

/*destructor*/
SipPChargingFunctionAddressesHeader::~SipPChargingFunctionAddressesHeader()
{
    if (m_pChargeAddr != SIP_NULL)
    {
        m_pChargeAddr->SipDelete();
    }
}

/*virtual methods*/
SIP_BOOL SipPChargingFunctionAddressesHeader::Encode(
        AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pChargeAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargeAddr->EncodeFromList(objBuffer) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

/*Function for encoding of headers*/
SIP_BOOL SipPChargingFunctionAddressesHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pChargeAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipPChargingFunctionAddressesHeader::EncodeHdr:m_pChargeAddr missing", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargeAddr->EncodeFromList(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipPChargingFunctionAddressesHeader::EncodeHdr: Name Value Encoding failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

/******************************************************************************
 * Function name      : SipPChargingFunctionAddressesHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPChargingFunctionAddressesHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipPChargingFunctionAddressesHeader::DecodeHdr",
                SIP_ZERO, SIP_ZERO);
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
                    "SipPChargingFunctionAddressesHeader::DecodeHdr: Hdr Prm Decoding Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }

    /*Decode the Header Value*/
    m_pChargeAddr = new SipNameValue(GetHdrType());
    if (m_pChargeAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargeAddr->DecHdrNameVal(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Name Value Decoding Successful", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPChargingFunctionAddressesHeader::GetNewObj(
        SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPChargingFunctionAddressesHeader(
                *reinterpret_cast<SipPChargingFunctionAddressesHeader*>(pHeader));
    }
    return new SipPChargingFunctionAddressesHeader();
}
