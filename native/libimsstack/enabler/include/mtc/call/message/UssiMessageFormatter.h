#ifndef MTC_USSI_MESSAGE_FORMATTER_H_
#define MTC_USSI_MESSAGE_FORMATTER_H_

#include "call/message/MessageFormatter.h"

class UssiMessageFormatter : public MessageFormatter
{
public:
    UssiMessageFormatter(IN IMtcSessionContext& objContext);
    virtual ~UssiMessageFormatter();
    UssiMessageFormatter(IN CONST MessageFormatter&) = delete;
    UssiMessageFormatter& operator=(IN CONST MessageFormatter&) = delete;

public:
    virtual IMS_RESULT FormStartMessage() override;
    virtual IMS_RESULT FormAcceptMessage() override;

private:
    void SetRecvInfoHeader();
    void SetAcceptHeader();
    void SetContentTypeHeader();
};

#endif
