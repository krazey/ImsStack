#ifndef MTC_AOS_EVENT_HANDLER_H
#define MTC_AOS_EVENT_HANDLER_H

#include "IMSTypeDef.h"

class IMessage;
class AString;
class MtcCallController;
class MtcConfigurationProxy;
class IMtcService;
class JniMtcServiceThread;

class MtcAosEventHandler
{
public:
    explicit MtcAosEventHandler(IN IMtcService& objService,
            IN MtcConfigurationProxy& objConfiguration);
    ~MtcAosEventHandler();
    MtcAosEventHandler(IN const MtcAosEventHandler&) = delete;
    MtcAosEventHandler& operator=(IN const MtcAosEventHandler&) = delete;

    void OnConnected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan,
            IN JniMtcServiceThread* pServiceThread);
    void OnDisconnecting(IN IMS_UINT32 nReason, IN MtcCallController& objCallController);
    void OnDisconnected(IN IMS_UINT32 nReason, IN MtcCallController& objCallController,
            IN JniMtcServiceThread* pServiceThread);
    void OnSuspended(IN IMS_UINT32 nReason, IN MtcCallController& objCallController);
    void OnResumed();

    void OnServiceConnected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan);
    void OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

private:
    IMtcService& m_objService;
    MtcConfigurationProxy& m_objConfiguration;
};

#endif
