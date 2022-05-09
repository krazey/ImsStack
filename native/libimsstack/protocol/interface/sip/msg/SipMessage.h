#ifndef __SIP_MESSAGE_H__
#define __SIP_MESSAGE_H__

#include "msg/SipHeaders.h"
#include "msg/SipMsgBody.h"
#include "msg/SipRequestLine.h"
#include "SipConfiguration.h"
#ifdef SIP_BADMESSAGE_PARSING
#include "msg/SipBadHeader.h"
#endif

/**
  This class represents one SIP message. The stack user can create a suitable message by setting
  the headers in SipMessage. Each header that has to be added should be set using
  SipMessage::SetHdr(). To send a request, the request line along with the mandatory headers must
  be set. SipStackManager::SendMsg() should be used send the message

  To send a response, the request line along with the mandatory headers must be set.
  SipStackManager::SendMsg() should be used send the message

  The object can be either of request type or response type.

  After setting any of the headers, the header being set can be freed using SipDelete(). The stack
  implements reference counting. When ever, the stack stores any header object it will increment
  the reference count.

 */

class SipMessage : public SipRefBase
{
public:
    enum
    {
        METHOD_INVITE = 0,
        METHOD_ACK,
        METHOD_OPTIONS,
        METHOD_BYE,
        METHOD_CANCEL,
        METHOD_REGISTER,
        METHOD_INFO,
        METHOD_PRACK,
        METHOD_SUBSCRIBE,
        METHOD_NOTIFY,
        METHOD_UPDATE,
        METHOD_MESSAGE,
        METHOD_REFER,
        METHOD_PUBLISH,
        METHOD_END,
        METHOD_INVALID
    };

    enum
    {
        MSG_INCOMPLETE = 0,
        MSG_COMPLETE,
        MSG_WRONG
    };

    enum
    {
        REQ_TYPE = 0,
        RESP_TYPE,
        TYPE_INVALID
    };

    enum
    {
        MANDATORY_HDR_NONE = 0x0000,
        MANDATORY_HDR_FROM = 0x0001,
        MANDATORY_HDR_TO = 0x0002,
        MANDATORY_HDR_CALL_ID = 0x0004,
        MANDATORY_HDR_CSEQ = 0x0008,
        MANDATORY_HDR_VIA = 0x0010,
        MANDATORY_HDR_MASK = 0x001F
    };

private:
    /* eSipMsgType will tell the type of the message    */
    SIP_INT32 m_eSipMsgType;

    /*Request Line*/
    SipRequestLine* m_pReqLine;

    /*Status Line*/
    SipStatusLine* m_pStatusLine;

    /*common(Request & Response Both)headers*/
    SipHeaders* m_objHdrs;

    /* List of SipMsgBody*/
    SipMsgBodyList* m_pMsgBodyList;

#ifdef SIP_BADMESSAGE_PARSING
    SIP_INT32 mbitMask;
    SipHeaderList* m_pBadHdrList;
#endif
    SIP_BOOL UpdateMessage(SIP_UINT32 nMsgOptions);

public:
    SipMessage();
    ~SipMessage();
    SipMessage(SIP_INT32 eSipMsgType);
    SipMessage(const SipMessage& objSipMsg);
    SIP_BOOL EncodeMsg(SIP_CHAR** ppSipMsgBuffer, /* in-out parameter*/
            SIP_UINT32* pSipMsgLength, /* in-out parameter*/ SIP_UINT32 nMsgOptions);

    SIP_BOOL DecCompleteMsg(SIP_CHAR* pMsgBuff, SIP_UINT32 nMsgBuffLen);

    SIP_BOOL DecodeMsg(SIP_CHAR* pStartPt, SIP_UINT32 nMsgBuffLen);

    SIP_BOOL DecMultiPartBody(SIP_CHAR* pBuffStart, SIP_CHAR* pBuffEnd, SIP_UINT32 nMsgBuffLen);

    inline SipMsgBodyList* GetMsgBodyList()
    {
        if (m_pMsgBodyList)
        {
            m_pMsgBodyList->increment();
        }
        return m_pMsgBodyList;
    }
    SipMsgBody* GetMessageBody(SIP_INT32 nIndex);

    SIP_BOOL SetRequestline(SipRequestLine* pReqLine);
    SIP_BOOL SetHeader(SipHeaderBase* pHdr);
    SIP_BOOL AppendHeader(SipHeaderBase* pHdr);
    SIP_BOOL InsertHeader(SipHeaderBase* pHdr, SIP_UINT32 nIndex);
    SIP_BOOL SetMessageBody(SipMsgBody* pMsgBody);
    SIP_BOOL SetMessageType(SIP_INT32 eMsgType);
    inline SipHeaders* GetMsgHdrs() { return (m_objHdrs); }
    inline SIP_INT32 GetMsgType() const { return m_eSipMsgType; }
    inline SipRequestLine* GetReqLine()
    {
        if (m_pReqLine != SIP_NULL)
        {
            m_pReqLine->increment();
        }
        return m_pReqLine;
    }
    inline SipStatusLine* GetStatusLine()
    {
        if (m_pStatusLine != SIP_NULL)
        {
            m_pStatusLine->increment();
        }
        return m_pStatusLine;
    }
    SIP_INT32 GetMethodType();
    const SIP_CHAR* GetMethod(); /* Based on Req and Resp*/
    SipHeaderBase* GetHdrObj(SIP_INT32 eHdrType);
    SipUnknownHeader* GetUnknownHdrObj(const SIP_CHAR* pszHdrName);
    SipUnknownHeader* GetUnknownHdrObj(SIP_INT32 eType);
    SIP_BOOL HasHeader(SIP_INT32 eHdrType) const;
    SipHeaderList* GetHdrList(SIP_INT32 eHdrType);
    SIP_VOID SetContentLengthHdr(SIP_UINT32 nLen, SIP_UINT32 nMsgOptions);
    SIP_BOOL SetHdrList(SipHeaderList* pHeaderList);
    SIP_BOOL SetMsgBodyList(SipMsgBodyList* pMsgBodyList);
    inline SIP_UINT16 GetMsgBodyCount() const
    {
        return (m_pMsgBodyList != SIP_NULL) ? m_pMsgBodyList->GetMsgBodyCount() : SIP_ZERO;
    }
    SIP_BOOL AppendMessageBody(SipMsgBody* pMsgBody);
    SipMsgBody* GetMsgBody(SIP_UINT32 nPos);
    SIP_BOOL RemoveHdr(SIP_INT32 eHdrType);
    SIP_BOOL SetStatusLine(SipStatusLine* pStatusLine);
    SIP_BOOL IsReqLineExists();
    SIP_BOOL IsStatusLineExists();
    SIP_BOOL HasMIMEMessageBody();
    SIP_BOOL HasSDPMessageBody();
#ifdef SIP_BADMESSAGE_PARSING
    SIP_BOOL HasMandatoryHdrs();
    SIP_INT32 GetBadHeaderCount() const;
    SipHeaderList* GetBadHdrs();
    SIP_VOID DeleteBadHdrList();
#endif

    void AdjustContentLengthHdr();
    void RemoveAllMessageBodies();

    // Utility methods
    SIP_UINT16 GetStatusCode() const;

    SipHeaderBase* GetHeader(SIP_INT32 nType, SIP_UINT32 nIndex);
    SIP_INT32 GetHeaderCount(SIP_INT32 nType);
};
#endif  //__SIP_MESSAGE_H__
