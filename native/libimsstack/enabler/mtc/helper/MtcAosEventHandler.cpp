#include "IMSTypeDef.h"
#include "helper/MtcAosEventHandler.h"
#include "ServiceTrace.h"
#include "JniMtcServiceThread.h"
#include "configuration/MtcConfigurationProxy.h"
#include "call/MtcCallController.h"
#include "AString.h"
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "IuMtcService.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcAosEventHandler::MtcAosEventHandler(
        IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration) :
        m_objService(objService),
        m_objConfiguration(objConfiguration)
{
    IMS_TRACE_I("+MtcAosEventHandler", 0, 0, 0);
}

PUBLIC
MtcAosEventHandler::~MtcAosEventHandler()
{
    IMS_TRACE_I("~MtcAosEventHandler", 0, 0, 0);
}

PUBLIC
void MtcAosEventHandler::OnConnected(
        IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan, IN JniMtcServiceThread* pServiceThread)
{
    IMS_TRACE_I("OnConnected emergency[%s] nIpcan[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nIpcan, 0);

    IMS_UINT32 nMmtelConnected =
            nFeatures & ImsAosFeature::MMTEL ? ImsAosFeature::MMTEL : ImsAosFeature::NONE;
    IMS_UINT32 nVideoConnected =
            nFeatures & ImsAosFeature::VIDEO ? ImsAosFeature::VIDEO : ImsAosFeature::NONE;

    if (m_objService.IsEmergency())
    {
        // TODO: OnEServiceChanged / Jni for Emergnecy MtcService.
        // pServiceThread->OnServiceChanged(nMmtelConnected + nVideoConnected, 0);
    }
    else
    {
        if (pServiceThread)
        {
            pServiceThread->OnServiceChanged(nMmtelConnected + nVideoConnected, 0);
        }
        // TODO: this must be called when registration is refreshed?
        m_objConfiguration.OnRegistrationRefreshed();
    }
}

PUBLIC
void MtcAosEventHandler::OnDisconnecting(
        IN IMS_UINT32 nReason, IN MtcCallController& objCallController)
{
    IMS_TRACE_I("OnDisconnecting emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);

    Key nKey;
    nKey.eServiceType = m_objService.GetServiceType();
    if (m_objConfiguration.Is(
                Feature::REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL, nReason))
    {
        objCallController.TerminateCalls(KeyType::SERVICE_TYPE, nKey, nReason);
    }
}

PUBLIC
void MtcAosEventHandler::OnDisconnected(IN IMS_UINT32 nReason,
        IN MtcCallController& objCallController, IN JniMtcServiceThread* pServiceThread)
{
    IMS_TRACE_I("OnDisconnected emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);

    Key nKey;
    nKey.eServiceType = m_objService.GetServiceType();
    if (m_objConfiguration.Is(
                Feature::REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL, nReason))
    {
        objCallController.TerminateCalls(KeyType::SERVICE_TYPE, nKey, nReason);
    }

    if (m_objService.IsEmergency())
    {
        // TODO: OnEServiceChanged / Jni for Emergnecy MtcService.
        // pServiceThread->OnServiceChanged(nMmtelConnected + nVideoConnected, 0);
    }
    else
    {
        if (pServiceThread)
        {
            pServiceThread->OnServiceChanged(IuMtcService::SERVICE_NONE, 0);
        }
    }
}

PUBLIC
void MtcAosEventHandler::OnSuspended(
        IN IMS_UINT32 nReason, IN MtcCallController& /*objCallController*/)
{
    IMS_TRACE_I("OnSuspended emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);
}

PUBLIC
void MtcAosEventHandler::OnResumed()
{
    IMS_TRACE_I("OnResumed emergency[%s]", _TRACE_B_(m_objService.IsEmergency()), 0, 0);
}

PUBLIC
void MtcAosEventHandler::OnServiceConnected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan)
{
    IMS_TRACE_I("OnServiceConnected emergency[%s] nServices[%d] nIpcan[%d]",
            _TRACE_B_(m_objService.IsEmergency()), nServices, nIpcan);
}

PUBLIC
void MtcAosEventHandler::OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("OnEventNotify emergency[%s] nType[%d] nState[%d]",
            _TRACE_B_(m_objService.IsEmergency()), nType, nState);
}
