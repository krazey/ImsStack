#ifndef INTERFACE_MTC_CONTEXT_H_
#define INTERFACE_MTC_CONTEXT_H_

#include "IMSTypeDef.h"
#include "IMtcService.h"

class MtcDialingPlan;
class MtcCallController;
class IMtcCallManager;
class MtcVonrManager;
class MtcConfigurationProxy;
class CallStateProxy;
class MtcAosConnector;
class MtcImsEventReceiver;
class MtcSipInterfaceFactory;
class ConferenceManager;

class IMtcContext
{
public:
    virtual IMS_SINT32 GetSlotId() = 0;
    virtual IMtcService* GetServiceByType(IN ServiceType eServiceType) = 0;
    virtual MtcDialingPlan& GetDialingPlan() = 0;
    virtual MtcCallController& GetCallController() = 0;
    virtual IMtcCallManager& GetCallManager() = 0;
    virtual MtcVonrManager& GetVonrManager() = 0;
    virtual MtcConfigurationProxy& GetConfigurationProxy() = 0;
    virtual CallStateProxy& GetCallStateProxy() = 0;
    virtual MtcImsEventReceiver& GetImsEventReceiver() = 0;
    virtual MtcAosConnector* GetAosConnector(IN ServiceType eServiceType) = 0;
    virtual MtcSipInterfaceFactory& GetSipInterfaceFactory() = 0;
    virtual ConferenceManager& GetConferenceManager() = 0;
};

#endif
