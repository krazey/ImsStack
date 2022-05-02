#ifndef MTC_MESSAGE_SENDER_H_
#define MTC_MESSAGE_SENDER_H_

#include "call/message/MessageFormatter.h"
#include "SipMethod.h"
#include "MtcDef.h"

class IMtcSessionContext;
class ISession;
struct FailReason;

class MessageSender
{
public:
    MessageSender(IN IMtcSessionContext& objContext);
    ~MessageSender();
    MessageSender(IN CONST MessageSender&) = delete;
    MessageSender& operator=(IN CONST MessageSender&) = delete;

public:
    IMS_RESULT Start();
    IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
            IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo);
    IMS_RESULT SendPrack();
    IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode);
    IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType);
    IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode);
    IMS_RESULT Accept();
    IMS_RESULT Reject(IN const FailReason& objReason);
    IMS_RESULT SendAck();
    IMS_RESULT Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
            IN IMS_SINT32 eMethod = SIPMethod::INVITE, IN IMS_BOOL bSessionRefresh = IMS_FALSE);
    IMS_RESULT AcceptUpdate();
    IMS_RESULT CancelUpdate(IN const FailReason& objReason);
    IMS_RESULT Terminate(IN IMS_BOOL bUseBye, IN const FailReason& objReason);

private:
    void CreateFormatter();

private:
    IMtcSessionContext& m_objContext;
    ISession& m_objSession;
    MessageFormatter* m_pFormatter;
};

#endif
