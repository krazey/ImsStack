#ifndef INTERFACE_MTC_SERVICE_H_
#define INTERFACE_MTC_SERVICE_H_

#include "IMSTypeDef.h"
#include "helper/ISrvccStateListener.h"

class AString;
class ICoreService;
class JniMtcService;
class MtcAosConnector;
class ISrvccStateListener;
enum class ServiceStatus;
enum class ServiceType;
enum class SrvccState;

class IMtcService
{
public:
    virtual ServiceType GetServiceType() const = 0;
    virtual void AddSrvccStateListener(IN ISrvccStateListener* piListener) = 0;
    virtual void RemoveSrvccStateListener(IN ISrvccStateListener* piListener) = 0;

    virtual IMS_BOOL IsActive() const = 0;
    virtual IMS_BOOL IsEmergency() const = 0;
    virtual IMS_BOOL IsWlanIpCanType() const = 0;
    virtual ServiceStatus GetServiceStatus() const = 0;
    virtual ICoreService* GetICoreService() const = 0;
    virtual MtcAosConnector* GetAosConnector() const = 0;

    virtual void UpdateSrvccState(IN SrvccState eState) = 0;
    virtual void SetJniService(IN JniMtcService* pJniService) = 0;
    virtual void SetTerminalBasedCallWaiting(IN IMS_BOOL bProvisioned, IN IMS_BOOL bEnabled) = 0;
    virtual IMS_BOOL IsTerminalBasedCallWaitingEnabled() const = 0;
};

enum class ServiceStatus
{
    SERVICE_IDLE,
    SERVICE_ACTIVE,
    SERVICE_SUSPENDED,
};

enum class ServiceType
{
    UNKNOWN = 0,
    NORMAL = 1 << 0,
    EMERGENCY = 1 << 1,
};

#endif
