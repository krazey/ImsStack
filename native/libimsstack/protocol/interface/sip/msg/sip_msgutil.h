/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : sip_msgutil.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)             : Vijay Nair
 * E-mail id.            : vijay.nair@
 * Creation date         : Jan 7,2014
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_MSGUTIL_H__
#define __SIP_MSGUTIL_H__

#include "sip_pf_datatypes.h"

#include "msg/sip_comdef.h"
#include "sip_abnfUtil.h"
#include "msg/SipHeaderBase.h"
#include "SipTrace.h"
#include "sip_error.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

#define SIP_SIP_VERSION     "SIP/2.0"
#define ZERO "0"

#define SIP_ENC_SP(pC)         *(pC) = 32;(pC)++
#define SIP_ENC_CRLF(pC)     *(pC) = 13;(pC)++;*(pC) = 10;(pC)++
#define SIP_ENC_DASH(pC)   *(pC) = 45;(pC)++;*(pC) = 45;(pC)++
#define SIP_ENC_STAR(pC)     *(pC) = 42;(pC)++
#define SIP_ENC_SLASH(pC)     *(pC) = 47;(pC)++
#define SIP_ENC_EQUAL(pC)     *(pC) = 61;(pC)++
#define SIP_ENC_LPAREN(pC)     *(pC) = 40;(pC)++
#define SIP_ENC_RPAREN(pC)     *(pC) = 41;(pC)++
#define SIP_ENC_LAQUOT(pC)     *(pC) = 60;(pC)++
#define SIP_ENC_RAQUOT(pC)     *(pC) = 62;(pC)++
#define SIP_ENC_COMMA(pC)     *(pC) = 44;(pC)++
#define SIP_ENC_SEMI(pC)     *(pC) = 59;(pC)++
#define SIP_ENC_COLON(pC)     *(pC) = 58;(pC)++
#define SIP_ENC_LDQUOT(pC)     *(pC) = 34;(pC)++
#define SIP_ENC_RDQUOT(pC)     *(pC) = 34;(pC)++
#define SIP_ENC_ATTHERATE(pC)     *(pC) = 64;(pC)++
#define SIP_ENC_QMARK(pC)     *(pC) = 63;(pC)++
#define SIP_ENC_AMPERSAND(pC)     *(pC) = 38;(pC)++
#define SIP_ENC_BSLASH(pC)     *(pC) = 92;(pC)++
#define SIP_ENC_DOT(pC)     *(pC) = 46;(pC)++

#define SIP_MAX_HDR_LEN        32
#define SIP_MAX_MTD_LEN        16
#define SIP_MAX_RESP_CODE_LEN  4
#define MAX_RESP_PHRS_LEN       38
#define SIP_MAX_URI_LEN        6
#define SIP_CONTENT_HDRS_LEN   5
#define SIP_CONTLEN_LEN        12

//keeping same SIP message buffer size which is mentioned in SIPMessageBuffer.h
#define SIP_MAX_MSG_SIZE 65535

#define SIP_ENC_SHORT_ACCEPT_CONTACT(ch)         *(ch) = 'a';(ch)++
#define SIP_ENC_SHORT_REFERRED_BY(ch)            *(ch) = 'b';(ch)++
#define SIP_ENC_SHORT_CONTENT_TYPE(ch)           *(ch) = 'c';(ch)++
#define SIP_ENC_SHORT_REQUEST_DISPOSITION(ch)    *(ch) = 'd';(ch)++
#define SIP_ENC_SHORT_CONTENT_ENCODING(ch)       *(ch) = 'e';(ch)++
#define SIP_ENC_SHORT_FROM(ch)                   *(ch) = 'f';(ch)++
#define SIP_ENC_SHORT_CALLID(ch)                 *(ch) = 'i';(ch)++
#define SIP_ENC_SHORT_REJECT_CONTACT(ch)         *(ch) = 'j';(ch)++
#define SIP_ENC_SHORT_SUPPORTED(ch)              *(ch) = 'k';(ch)++
#define SIP_ENC_SHORT_CONTENT_LENGTH(ch)         *(ch) = 'l';(ch)++
#define SIP_ENC_SHORT_CONTACT(ch)                *(ch) = 'm';(ch)++
#define SIP_ENC_SHORT_IDENTITY_INFO(ch)          *(ch) = 'n';(ch)++
#define SIP_ENC_SHORT_EVENT(ch)                  *(ch) = 'o';(ch)++
#define SIP_ENC_SHORT_REFER_TO(ch)               *(ch) = 'r';(ch)++
#define SIP_ENC_SHORT_SUBJECT(ch)                *(ch) = 's';(ch)++
#define SIP_ENC_SHORT_TO(ch)                     *(ch) = 't';(ch)++
#define SIP_ENC_SHORT_ALLOW_EVENTS(ch)           *(ch) = 'u';(ch)++
#define SIP_ENC_SHORT_VIA(ch)                    *(ch) = 'v';(ch)++
#define SIP_ENC_SHORT_SESSION_EXPIRES(ch)        *(ch) = 'x';(ch)++
#define SIP_ENC_SHORT_IDENTITY(ch)               *(ch) = 'y';(ch)++

#define MULTIPART "Multipart"
#define MIXED "Mixed"
#define SDP "Sdp"
#define SIP_ENC_GMT(ch) *(ch) ='G';(ch)++;*(ch) ='M';(ch)++;*(ch) ='T';(ch)++

#define SIP_AINFO_CNT 5

#define SIP_METHOD_COUNT 14

#define SIP_AINFO_LEN 20


#define MAXLETDIG 27

#define MAX_CSEQ 4294967295

#define MAX_WARNCODE 999

#define MAX_EXPIRES 4294967295

#define MAX_MAXFD 255

#define MAX_ERROR_CODE 9999

#define MAX_CIDLEN 48

#define MAX_FEIDLEN 16

#define SIP_DIRECTIVE_SIZE 12

#define SIP_DIRECTIVE_LEN 11

/******************************************************************************
 * Function name      : SetCharVar
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/

SIP_BOOL SetCharVar(const SIP_CHAR* pszValue, SIP_CHAR*& pszVar);
/******************************************************************************
 * Function name      : HasSpace
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_BOOL HasSpace(const SIP_CHAR* pszValue);

/*****************************************************************************
 * Function name      : sipGetMsgType
 *
 * Description        :
 *
 * Preconditions      :     Accept
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetMsgType(SIP_CHAR* pStartPoint);

/*****************************************************************************
 * Function name      : sipFindTerminatingCRLF
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_BOOL  sipFindTerminatingCRLF(SIP_CHAR* pStartPoint, SIP_CHAR* pEndPoint, SIP_CHAR** ppLocation,
        SIP_BOOL* pbHdrEnd);

/******************************************************************************
 * Function name      : sipSkipFwSP
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_CHAR* sipSkipFwSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);


/******************************************************************************
 * Function name      : sipSkipRwSP
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_CHAR* sipSkipRwSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);


/******************************************************************************
 * Function name      : sipSkipFwWSP
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_CHAR* sipSkipFwWSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);


/******************************************************************************
 * Function name      : sipSkipRwWSP
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_CHAR* sipSkipRwWSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);


/*****************************************************************************
 * Function name      : sipSkipFwLWS
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/

SIP_CHAR* sipSkipFwLWS(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);





/******************************************************************************
 * Function name      : sipFindPostDelimiter
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindPostDelimiter(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc,
        SIP_CHAR cDelimiter);


/******************************************************************************
 * Function name      : sipFindPreDelimiter
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindPreDelimiter(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc,
        SIP_CHAR cDelimiter);



/******************************************************************************
 * Function name      : sipFindActualPos
 *
 * Description     : this Api will find the delimiter and Remove LWS form
 *                    both the Side
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindActualPos(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempPre,
        SIP_CHAR** ppTempNext, SIP_CHAR cDelimiter);

/******************************************************************************
 * Function name  : SipEnc_UpdateCurrPos
 * Description     :  This api will update the current position of the sip msg
 *****************************************************************************/
SIP_VOID SipEnc_UpdateCurrPos(SIP_CHAR** ppMsgBuffer/*in -out param*/);

/*****************************************************************************
 * Function name      : sipGetUriType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetUriType(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);



/*****************************************************************************
 * Function name      : sipGetHdrType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetHdrType(const SIP_CHAR* pszHdrName);

SIP_INT32 CheckAndGetHdrEnumType(SIP_INT32 nType);

SIP_BOOL IsValidAddress(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

const SIP_INT16 arrSipHeadersType[SipHeaderBase::TYPE_END + 1]=
{
    1,1,0,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,1,1,0,1,1,0,1,1,1,0,1,1,1,1,
    1,0,0,0,0,0,0,1,0,0,0,1,1,0,1,1,1,0,
    0,1,0,0,1,1,1,0,1,0,0,0,1,1,1,0,1,1,
    1,0,1,1,1,1,0,0,0,1,0,0,1,1,0,1,0,0,
    1,0,0,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,
    0,1,1,1,1,1,0,0
};


struct HdrNameType
{
    SIP_INT32 HdrType;
    SIP_CHAR HdrName[SIP_MAX_HDR_LEN];
};

struct HdrLenRecord
{
    SIP_INT16 Hdrlen;
    SIP_INT16 NoOfEntries;
    HdrNameType objHeaders[SIP_MAX_HDR_LEN];
};


class SIPHdrAccess
{
    private:

        static SIPHdrAccess *s_pInstance;
        HdrLenRecord objHdrLenRecord[SIP_MAX_HDR_LEN];

        SIPHdrAccess();
        SIPHdrAccess(SIPHdrAccess const& copy);

        SIP_INT32 GetHdrTypeCompact(SIP_CHAR cHdrName);

    public:
        static SIPHdrAccess* GetInstance();
        ~SIPHdrAccess(){}
        static void DestroyInstance();
        SIP_INT32 GetHdrType(const SIP_CHAR* pszHdrName);
};
#endif
