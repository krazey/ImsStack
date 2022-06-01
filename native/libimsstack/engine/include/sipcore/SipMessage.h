/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_MESSAGE_H_
#define _SIP_MESSAGE_H_

#include "ISipMessage.h"
#include "SipHeader.h"
#include "SipMessageBodyPart.h"

class SIPMessage : public ISipMessage
{
public:
    explicit SIPMessage(IN IMS_SINT32 nType_ = ISipMessage::TYPE_REQUEST);
    explicit SIPMessage(IN SipMessage* pstMessage_);
    explicit SIPMessage(IN SipMessage* pstMessage_, IN IMS_BOOL bMessageClone);
    virtual ~SIPMessage();

private:
    SIPMessage(IN CONST SIPMessage& objRHS);

public:
    SIPMessage& operator=(IN CONST SIPMessage& objRHS);

public:
    // ISipObject interface
    virtual void Destroy();
    // ISipMessage interface
    virtual ISipMessage* Clone() const;
    virtual IMS_RESULT AddHeader(IN IMS_SINT32 nType, IN CONST AString& strValue,
            IN CONST AString& strName = AString::ConstNull());
    virtual IMS_UINT32 GetCSeqNumber() const;
    virtual AString GetHeader(IN IMS_SINT32 nType, IN IMS_SINT32 nIndex = 0,
            IN CONST AString& strName = AString::ConstNull()) const;
    virtual IMS_SINT32 GetHeaderCount(
            IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull()) const;
    virtual IMSList<AString> GetHeaders(
            IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull()) const;
    inline virtual const SipMethod& GetMethod() const { return objMethod; }
    inline virtual const AString& GetReasonPhrase() const
    {
        return objStatusCode.GetReasonPhrase();
    }
    inline virtual const AString& GetRequestUri() const
    {
        return (nType == ISipMessage::TYPE_RESPONSE) ? AString::ConstNull() : strRequestURI;
    }
    inline virtual IMS_SINT32 GetStatusCode() const { return objStatusCode.ToInt(); }
    inline virtual IMS_SINT32 GetType() const { return nType; }
    virtual IMS_RESULT PrependHeader(IN IMS_SINT32 nType, IN CONST AString& strValue,
            IN CONST AString& strName = AString::ConstNull());
    virtual void RemoveHeader(
            IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull());
    virtual IMS_RESULT SetHeader(IN IMS_SINT32 nType, IN CONST AString& strValue,
            IN CONST AString& strName = AString::ConstNull());

    virtual ISipMessageBodyPart* CreateBodyPart();
    virtual ISipMessageBodyPart* CreateSdpBodyPart();
    virtual IMSList<ISipMessageBodyPart*> GetBodyParts() const;
    virtual ISipMessageBodyPart* GetSdpBodyPart() const;
    virtual IMSList<ISipMessageBodyPart*> GetSdpBodyParts() const;

    virtual IMS_RESULT CopyHeadersAndBodyParts(IN CONST ISipMessage* piSIPMsg);
    virtual IMS_BOOL IsHeaderPresent(
            IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull()) const;
    virtual IMS_BOOL IsMessageRpr() const;
    virtual IMS_BOOL IsOptionRequired(IN CONST AString& strOption) const;
    virtual IMS_BOOL IsOptionSupported(IN CONST AString& strOption) const;
    virtual void RemoveBodyParts();
    virtual ByteArray ToByteArray(IN IMS_SINT32 nOptions = OPT_ALL) const;

    // ISipConnection interface
    SIPMessageBodyPart* GetBodyPart() const;

    // ISipClientConnection interface
    IMS_RESULT SetRequestURI(IN CONST AString& strURI);
    void UpdateRequestURI();

    // ISipServerConnection interface
    inline void SetStatusCode(IN IMS_SINT32 nStatusCode) { objStatusCode = nStatusCode; }
    inline void SetReasonPhrase(IN CONST AString& strPhrase) { objStatusCode = strPhrase; }

    // General-purpose methods
    IMS_BOOL CreateBodyParts();
    IMS_BOOL FormMessage();
    IMS_BOOL FormMessageOnChallenge();
    IMS_BOOL FormMessageOnRetransmission();
    inline SipMessage* GetMessage() const { return pstMessage; }
    inline void SetMethod(IN CONST SipMethod& objMethod) { this->objMethod = objMethod; }

    static SIPMessage* CreateMessage(IN CONST ByteArray& objMessage);

private:
    void Init(IN IMS_BOOL bMessageClone);
    IMS_BOOL ExtractBodyParts();
    IMS_BOOL ExtractProperties();
    IMS_BOOL ExtractUnknownHeaders();

private:
    IMS_SINT32 nType;

    SipMethod objMethod;
    // union type ???
    AString strRequestURI;
    SipStatusCode objStatusCode;

    SIPUnknownHeaders objUnknownHeaders;
    IMS_BOOL bBodyPartParsed;
    IMSList<SIPMessageBodyPart*> objBodyParts;

    SipMessage* pstMessage;
};

#endif  // _SIP_MESSAGE_H_
