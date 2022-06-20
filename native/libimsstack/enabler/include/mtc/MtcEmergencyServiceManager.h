#ifndef MTC_EMERGENCY_SERVICE_MANAGER_H_
#define MTC_EMERGENCY_SERVICE_MANAGER_H_

#include "IMtcContext.h"
#include "IuMtcService.h"

class JniMtcServiceThread;

class MtcEmergencyServiceManager
{
public:
    MtcEmergencyServiceManager(IN IMtcContext& objContext);
    virtual ~MtcEmergencyServiceManager();
    MtcEmergencyServiceManager(IN const MtcEmergencyServiceManager&) = delete;
    MtcEmergencyServiceManager& operator=(IN const MtcEmergencyServiceManager&) = delete;

    void OpenEmergencyService(IN JniMtcServiceThread* pServiceThread);
    void ProcessServiceStatus(IN ServiceStatus eStatus, IN JniMtcServiceThread* pServiceThread);
    void SetStatus(IN IuMtcService::EmergencyServiceStatus eStatus,
            IN JniMtcServiceThread* pServiceThread);

private:
    IMtcContext& m_objContext;
    IuMtcService::EmergencyServiceStatus m_eStatus;

};

#endif
