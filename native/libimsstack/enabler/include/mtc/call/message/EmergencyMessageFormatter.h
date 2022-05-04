#ifndef MTC_EMERGENCY_MESSAGE_FORMATTER_H_
#define MTC_EMERGENCY_MESSAGE_FORMATTER_H_

#include "IMtcService.h"
#include "call/message/MessageFormatter.h"

class EmergencyMessageFormatter : public MessageFormatter
{
public:
    EmergencyMessageFormatter(IN IMtcSessionContext& objContext);
    virtual ~EmergencyMessageFormatter();
    EmergencyMessageFormatter(IN CONST MessageFormatter&) = delete;
    EmergencyMessageFormatter& operator=(IN CONST MessageFormatter&) = delete;

public:
    virtual IMS_RESULT FormStartMessage() override;

private:
    void SetPPreferredIdentityHeader();
    void SetPPreferredIdentityHeaderByUserId();
    void SetPPreferredIdentityHeaderByDeviceId();
    void SetSipInstanceFeature();

    IMS_UINT32 GetAoSRegMode(IN ServiceType eServiceType);
    IMS_RESULT GetLocalIpAddress(OUT AString& strIpAddress);
    IMS_UINT32 GetLocalPort();

private:
    IMS_UINT32 m_eNormalAosRegMode;
    IMS_UINT32 m_eEmergencyAosRegMode;
};

#endif
