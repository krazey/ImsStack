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
#include "SIPHeader.h"
#include "SIPMessageBodyPart.h"



class SIPMessage
    : public ISIPMessage
{
public:
    explicit SIPMessage(IN IMS_SINT32 nType_ = ISIPMessage::TYPE_REQUEST);
    explicit SIPMessage(IN SipMessage *pstMessage_);
    explicit SIPMessage(IN SipMessage *pstMessage_, IN IMS_BOOL bMessageClone);
    virtual ~SIPMessage();

private:
    SIPMessage(IN CONST SIPMessage &objRHS);

public:
    SIPMessage& operator=(IN CONST SIPMessage &objRHS);

public:
    // ISIPObject interface
    virtual void Destroy();
    // ISIPMessage interface
    virtual ISIPMessage* Clone() const;
    virtual IMS_RESULT AddHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull());
    virtual IMS_UINT32 GetCSeqNumber() const;
    virtual AString GetHeader(IN IMS_SINT32 nType, IN IMS_SINT32 nIndex = 0,
            IN CONST AString &strName = AString::ConstNull()) const;
    virtual IMS_SINT32 GetHeaderCount(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull()) const;
    virtual IMSList<AString> GetHeaders(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull()) const;
    inline virtual const SIPMethod& GetMethod() const
    { return objMethod; }
    inline virtual const AString& GetReasonPhrase() const
    { return objStatusCode.GetReasonPhrase(); }
    inline virtual const AString& GetRequestURI() const
    { return (nType == ISIPMessage::TYPE_RESPONSE) ? AString::ConstNull() : strRequestURI; }
    inline virtual IMS_SINT32 GetStatusCode() const
    { return objStatusCode.ToInt(); }
    inline virtual IMS_SINT32 GetType() const
    { return nType; }
    virtual IMS_RESULT PrependHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull());
    virtual void RemoveHeader(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull());
    virtual IMS_RESULT SetHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull());

    virtual ISIPMessageBodyPart* CreateBodyPart();
    virtual ISIPMessageBodyPart* CreateSDPBodyPart();
    virtual IMSList<ISIPMessageBodyPart*> GetBodyParts() const;
    virtual ISIPMessageBodyPart* GetSDPBodyPart() const;
    virtual IMSList<ISIPMessageBodyPart*> GetSDPBodyParts() const;

    virtual IMS_RESULT CopyHeadersAndBodyParts(IN CONST ISIPMessage *piSIPMsg);
    virtual IMS_BOOL IsHeaderPresent(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull()) const;
    virtual IMS_BOOL IsMessageRPR() const;
    virtual IMS_BOOL IsOptionRequired(IN CONST AString &strOption) const;
    virtual IMS_BOOL IsOptionSupported(IN CONST AString &strOption) const;
    virtual void RemoveBodyParts();
    virtual ByteArray ToByteArray(IN IMS_SINT32 nOptions = OPT_ALL) const;

    // ISIPConnection interface
    SIPMessageBodyPart* GetBodyPart() const;

    // ISIPClientConnection interface
    IMS_RESULT SetRequestURI(IN CONST AString &strURI);
    void UpdateRequestURI();

    // ISIPServerConnection interface
    inline void SetStatusCode(IN IMS_SINT32 nStatusCode)
    { objStatusCode = nStatusCode; }
    inline void SetReasonPhrase(IN CONST AString &strPhrase)
    { objStatusCode = strPhrase; }

    // General-purpose methods
    IMS_BOOL CreateBodyParts();
    IMS_BOOL FormMessage();
    IMS_BOOL FormMessageOnChallenge();
    IMS_BOOL FormMessageOnRetransmission();
    inline SipMessage* GetMessage() const
    { return pstMessage; }
    inline void SetMethod(IN CONST SIPMethod &objMethod)
    { this->objMethod = objMethod; }

    static SIPMessage* CreateMessage(IN CONST ByteArray &objMessage);

private:
    void Init(IN IMS_BOOL bMessageClone);
    IMS_BOOL ExtractBodyParts();
    IMS_BOOL ExtractProperties();
    IMS_BOOL ExtractUnknownHeaders();

private:
    IMS_SINT32 nType;

    SIPMethod objMethod;
    // union type ???
    AString strRequestURI;
    SIPStatusCode objStatusCode;

    SIPUnknownHeaders objUnknownHeaders;
    IMS_BOOL bBodyPartParsed;
    IMSList<SIPMessageBodyPart*> objBodyParts;

    SipMessage *pstMessage;
};

#endif // _SIP_MESSAGE_H_
