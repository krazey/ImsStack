#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipPolicyContactHeader.h"
#include "msg/sip_msgutil.h"

/*constructor*/
SipPolicyContactHeader::SipPolicyContactHeader() :
        SipNameAddrHeader(SipHeaderBase::POLICY_CONTACT)
{
}

SipPolicyContactHeader::SipPolicyContactHeader(const SipPolicyContactHeader& objHeader) :
        SipNameAddrHeader(objHeader)
{
}

/*destructor*/
SipPolicyContactHeader::~SipPolicyContactHeader() {}

SIP_BOOL SipPolicyContactHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pNameAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty addr-spec", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pNameAddr->SetParameterComponent(DYNAMIC_CAST(IParameterComponent*, this));

    if (m_pNameAddr->Encode(objBuffer, SIP_TRUE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encoding name-addr failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipPolicyContactHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pNameAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty Addr Spec", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pNameAddr->SetParameterComponent(this);

    if (m_pNameAddr->EncodeNameAddr(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "sipEncodePolicyContactHdr: Addr Spec failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

/******************************************************************************
 * Function name      : SipPolicyContactHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPolicyContactHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if ((nDecLen == SIP_ZERO) || (m_pNameAddr == SIP_NULL))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Empty buffer or nameAddr null", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Add reference for Percentage Encoding*/
    if (IsPercentEncHdr() == SIP_TRUE)
    {
        m_pNameAddr->SetParameterComponent(static_cast<IParameterComponent*>(this));
    }

    // Check whether it is policyContact-info = LAQUOT policyContact-uri RAQUOT, if not Failure case
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_INT32 nLen = SIP_ZERO;
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, RIGHT_ANGLE) == SIP_TRUE)
    {
        if (m_pNameAddr->DecodeNameAddr(pStartPt, pTempPre) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "%d::DecodeHdr:Name Addr decoding failed",
                    GetHdrType(), SIP_ZERO);
            return SIP_FALSE;
        }
        /*Now find the start of Sip Prm*/
        nLen = pTempPre - pStartPt;
        pStartPt = pTempPre + SIP_TWO;
        pStartPt = sipSkipFwLWS(pStartPt, pEndPt);
        pTempPre = SIP_NULL;
        SIP_CHAR* pTempNext = SIP_NULL;

        // added the condition ((*pStartPt)==';')
        if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
        {
            if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "%d::DecodeHdr: Prm decoding failed",
                        GetHdrType(), SIP_ZERO);
                return SIP_FALSE;
            }
        }
        else
        {
            if (nLen != (SIP_INT32(nDecLen - 2)))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "%d::DecodeHdr: Invalid Delimiter separating param", GetHdrType(),
                        SIP_ZERO);
                return SIP_FALSE;
            }
        }
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipNameAddr::DecodeNameAddr: Right Angle Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPolicyContactHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPolicyContactHeader(*reinterpret_cast<SipPolicyContactHeader*>(pHeader));
    }
    return new SipPolicyContactHeader();
}
