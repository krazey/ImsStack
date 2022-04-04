/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipRSeqHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : giridhar.a@
 * Creation date       : July. 27,2010
 *
 * Edit HisAlertry         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Giridhar               0.0a            Initial creation
 *****************************************************************************/

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipRSeqHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"
#include "msg/sip_comdef.h"


/****************************************************************************
  Macro Definitions
 *****************************************************************************/
#define MAX_SEQ_LEN 12


/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/
/*constructor*/
SipRSeqHeader::SipRSeqHeader()
    : SipHeaderBase(SipHeaderBase::RSEQ)
{
}


SipRSeqHeader::SipRSeqHeader(const SipRSeqHeader& objHeader)
    : SipHeaderBase(objHeader)
{
}

/*destructor*/
SipRSeqHeader::~SipRSeqHeader()
{
}

/*Sets */
SIP_BOOL SipRSeqHeader::SetRSeqValue(SIP_UINT32 nRSeqValue)
{
    SIP_CHAR szLen[MAX_SEQ_LEN];
    SipPf_Sprintf(szLen, (SIP_CHAR*)"%u", nRSeqValue);
    return SetValue(szLen);
}

/*Gets */
SIP_UINT32 SipRSeqHeader::GetRSeqValue()
{
    return SipPf_Atoi(GetValue());
}

SIP_BOOL SipRSeqHeader::IsValidHeader() const
{
    SIP_INT32 nRSeq   = SipPf_Atoi(GetValue());
    if ((nRSeq > MAX_CSEQ) || (nRSeq == SIP_ZERO))
    {
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SipHeaderBase* SipRSeqHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipRSeqHeader(*reinterpret_cast<SipRSeqHeader*>(pHeader));
    }
    return new SipRSeqHeader();
}
