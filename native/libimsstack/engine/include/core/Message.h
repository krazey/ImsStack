/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090529  lovil@                    Created
    </table>

    Description

*/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "IMessage.h"
#include "MessageBodyPart.h"

class ISipMessage;
class AppConfig;

class Message : public IMessage
{
public:
    Message(IN AppConfig* pAppConfig_, IN IMS_SINT32 nState_);
    virtual ~Message();

private:
    Message(IN CONST Message& objRHS);
    Message& operator=(IN CONST Message& objRHS);

public:
    virtual ISipMessage* GetMessage() const;

    ISipMessageBodyPart* CreateBodyPartEx();
    void UpdateSentMessage(IN ISipMessage* piSIPMsg);

    static Message* CreateMessage(IN Message* pMessage);
    static Message* CreateUnsentMessage(IN AppConfig* pAppConfig, IN IMS_BOOL bRequest);
    static Message* CreateReceivedMessage(IN AppConfig* pAppConfig, IN ISipMessage* piSIPMsg);
    static const IMS_CHAR* GetMessageType(IN IMS_SINT32 nServiceMethod);

private:
    // IMessage interface implementations
    virtual IMessageBodyPart* CreateBodyPart();
    virtual IMSList<IMessageBodyPart*> GetBodyParts() const;
    virtual IMS_RESULT AddHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual IMSList<AString> GetHeaders(IN CONST AString& strName) const;
    virtual const SipMethod& GetMethod() const;
    virtual const AString& GetReasonPhrase() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_SINT32 GetStatusCode() const;

    IMS_BOOL CreateBodyParts();

    static IMS_BOOL IsInaccessibleHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName);
    static IMS_BOOL IsReadOnlyHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName);

private:
    AppConfig* pAppConfig;

    IMS_SINT32 nState;
    ISipMessage* piSIPMessage;
    IMSList<MessageBodyPart*> objBodyParts;
};

#endif  // _MESSAGE_H_
