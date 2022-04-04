/***********************************************************
 * Project Name : SIP_RTP
 * Group        : MSG-2
 * Security     : Confidential
 ***********************************************************/

/**********************************************************
 * Filename          : SipHeaderList.h
 * Purpose           :
 * Platform          : Windows XP
 * Author(s)         : Saurabh Srivastava
 * E-mail id.        : saurabh31.srivastava@
 * Creation date     : July 26, 2010
 *
 * Modifications:
 * 1. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0a
 *
 * 2. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0b
 **********************************************************/
#ifndef __SIP_HEADER_LIST_H__
#define __SIP_HEADER_LIST_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "msg/SipHeaderBase.h"


/****************************************************************************
  Macro Definitions
 *****************************************************************************/

//class SipHeaderBase;
/****************************************************************************
  Enum Declaration
 *****************************************************************************/
class SipHeaderList : public SipHeaderBase
{
    private:
        SipVector<SipHeaderBase*> m_headerList;

    public:
        SipHeaderList(SIP_INT32 eHdrType);
        SipHeaderList(const SipHeaderList& objHeaderList);
        ~SipHeaderList();
        static SipHeaderBase* GetNewListObj(SIP_INT32 eHdr, SipHeaderBase *pHeader);
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams, SIP_UINT32 nMsgOptions);

        SipHeaderBase* GetObj(SIP_UINT32 nIndex);
        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        SipHeaderBase* GetListObj();
        SipHeaderBase* GetListObj(SipHeaderBase*pHeader);
        SIP_BOOL AddHeader(SipHeaderBase* pHeader);
        SIP_BOOL InsertHdrAtPos(SipHeaderBase* pHeader, SIP_UINT32 nIndex);
        void RemoveHdr(SIP_UINT32 nIndex);

        SIP_BOOL IsOptionalValHdr() const;
        inline SIP_UINT32 GetSize() const
        {
            return m_headerList.GetSize();
        }
};

#endif //__SIP_HEADER_LIST_H__
