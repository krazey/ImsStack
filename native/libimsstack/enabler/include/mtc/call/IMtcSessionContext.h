#ifndef INTERFACE_MTC_SESSION_CONTEXT_H_
#define INTERFACE_MTC_SESSION_CONTEXT_H_

#include "IMSTypeDef.h"
#include "call/IMtcCallContext.h"

class MessageSender;
class MtcExtensionSet;
class ISession;

class IMtcSessionContext : public IMtcCallContext
{
public:
    virtual ~IMtcSessionContext(){};

    virtual ISession& GetISession() = 0;
    virtual MessageSender& GetMessageSender() = 0;
    virtual MtcExtensionSet& GetExtensionSet() = 0;
};

#endif
