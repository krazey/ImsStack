#include "helper/MtcAosConnector.h"
#include "ImsAosParameter.h"
#include "MtcEmergencyServiceManager.h"
#include "ServiceTrace.h"
#include "JniMtcServiceThread.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_eStatus(IuMtcService::EmergencyServiceStatus::IDLE)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);

    m_objContext.GetSlotId(); // TODO
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC
void MtcEmergencyServiceManager::OpenEmergencyService(IN JniMtcServiceThread* pServiceThread)
{
    MtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(ImsAosControl::REGISTER_START);
    SetStatus(IuMtcService::EmergencyServiceStatus::OPENING, pServiceThread);
}

PUBLIC
void MtcEmergencyServiceManager::ProcessServiceStatus(IN ServiceStatus eStatus,
        IN JniMtcServiceThread* pServiceThread)
{
    if (eStatus == ServiceStatus::SERVICE_ACTIVE)
    {
        SetStatus(IuMtcService::EmergencyServiceStatus::OPENED, pServiceThread);
    }
    else
    {
        SetStatus(IuMtcService::EmergencyServiceStatus::IDLE, pServiceThread);
    }
}

PUBLIC
void MtcEmergencyServiceManager::SetStatus(IN IuMtcService::EmergencyServiceStatus eStatus,
        IN JniMtcServiceThread* pServiceThread)
{
    if (m_eStatus == eStatus)
    {
        return;
    }

    m_eStatus = eStatus;

    if (pServiceThread == IMS_NULL)
    {
        return;
    }

    pServiceThread->OnEmergencyServiceChanged(static_cast<IMS_SINT32>(eStatus), -1);
}
