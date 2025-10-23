/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CarrierConfig.h"
#include "Connector.h"
#include "ServiceConfig.h"
#include "ICoreService.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "IJniEnabler.h"
#include "IJniMtsAppThread.h"
#include "IMtsContext.h"
#include "IMtsNetworkTracker.h"
#include "IServiceFilterCriteria.h"
#include "ISipHeader.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsCore.h"
#include "ImsServiceConfig.h"
#include "ImsServiceConfigTypeDef.h"
#include "IuMtsApp.h"
#include "MtsDef.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "MtsStringDef.h"
#include "MtsTraffic.h"
#include "ServiceImsRadio.h"
#include "ServicePhoneInfo.h"
#include "message/IMtsMessageController.h"
#include "utility/IMtsDynamicLoader.h"
#include "utility/MtsAosUtils.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTS__;

MtsService::MtsService(IN IMtsContext& objContext, IN MtsServiceType eServiceType) :
        ImsService(AString::ConstNull()),
        m_objContext(objContext),
        m_eServiceType(eServiceType),
        m_strAppId(ImsServiceConfig::GetAppName(ImsAppId::MTS)),
        m_piImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(m_objContext.GetSlotId())),
        m_objMtsTraffics(ImsList<IMtsTraffic*>()),
        m_pSmsInfo(IMS_NULL),
        m_piCoreService(IMS_NULL),
        m_piImsAos(IMS_NULL),
        m_piMtsServiceState(new MtsServiceState(m_objContext.GetSlotId()))
{
    IMS_TRACE_I("+MtsService [slot_%d]", m_objContext.GetSlotId(), 0, 0);
}

PUBLIC
MtsService::~MtsService()
{
    IMS_TRACE_I("~MtsService [slot_%d]", m_objContext.GetSlotId(), 0, 0);

    DeInit();
}

PUBLIC
void MtsService::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    AttachAos();
    AttachCoreService();
    InitMtsServiceState();

    IMS_UINT32 nTrafficType = GetTrafficTypeOfService();
    m_objMtsTraffics.Append(
            new MtsTraffic(m_objContext, IImsRadio::DIRECTION_MO, nTrafficType, *this));
    m_objMtsTraffics.Append(
            new MtsTraffic(m_objContext, IImsRadio::DIRECTION_MT, nTrafficType, *this));
}

PUBLIC VIRTUAL void MtsService::SendMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
        IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergencyNumber,
        IN IMS_UINT32 nRetryCount)
{
    IMS_TRACE_I("SendMoSms", 0, 0, 0);

    if (m_eServiceType == MtsServiceType::EMERGENCY)
    {
        // Wait until the Emergency REGISTRATION Procedure done
        m_piImsAos->Control(ImsAosControl::REGISTER_START);

        // Will be reset in Traffic_OnConnectionSetupPrepared upon successful connection setup.
        m_pSmsInfo = std::make_unique<SmsSendRequestInfo>(
                eSmsFormat, pContent, strAddress, nSeqId, bEmergencyNumber, nRetryCount);
        return;
    }

    switch (StartMoTrafficIfNeeded())
    {
        case MtsTrafficStartResult::TRAFFIC_READY:
            m_objContext.GetMessageController().ProcessMoSms(eSmsFormat, pContent, strAddress,
                    nSeqId, bEmergencyNumber, m_eServiceType, nRetryCount);
            break;

        case MtsTrafficStartResult::TRAFFIC_AWAITING_SETUP:
            // Radio connection is not yet ready. Store MO SMS request and this will be reset in
            // Traffic_OnConnectionSetupPrepared upon successful connection setup.
            m_pSmsInfo = std::make_unique<SmsSendRequestInfo>(
                    eSmsFormat, pContent, strAddress, nSeqId, bEmergencyNumber, nRetryCount);
            break;

        default:  // TRAFFIC_NOT_ALLOWED, TRAFFIC_NOT_FOUND
        {
            IJniMtsAppThread* piAppThread = m_objContext.GetJniAppThread();
            if (piAppThread)
            {
                piAppThread->ReportMoStatus(
                        MO_ERROR_RETRY, eSmsFormat, nSeqId, m_objContext.GetSlotId());
            }
            break;
        }
    }
}

PUBLIC
void MtsService::CoreService_PageMessageReceived(
        IN ICoreService* piService, IN IPageMessage* piMessage)
{
    (void)piService;

    IMS_TRACE_I("CoreService_PageMessageReceived : SMS message has been received", 0, 0, 0);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived : no IPageMessage", 0, 0, 0);
        return;
    }

    if (StartMtTraffic() != MtsTrafficStartResult::TRAFFIC_READY)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived: Failed to handle SMS traffic.", 0, 0, 0);
        return;
    }

    m_objContext.GetMessageController().ProcessMtSms(piMessage, m_eServiceType);
}

PUBLIC
void MtsService::CoreService_ReferenceReceived(
        IN ICoreService* piService, IN IReference* piReference)
{
    (void)piService;
    (void)piReference;

    IMS_TRACE_I("CoreService_ReferenceReceived : Service Name[%s]", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_ServiceClosed(IN ICoreService* piService, IN IReasonInfo* piReasonInfo)
{
    (void)piService;
    (void)piReasonInfo;

    IMS_TRACE_I("CoreService_ServiceClosed : Service Name[%s]", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_SessionInvitationReceived(
        IN ICoreService* piService, IN ISession* piSession)
{
    (void)piService;
    (void)piSession;

    IMS_TRACE_I(
            "CoreService_SessionInvitationReceived : Service Name[%s]", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_UnsolicitedNotifyReceived(
        IN ICoreService* piService, IN IMessage* piNotify)
{
    (void)piService;
    (void)piNotify;

    IMS_TRACE_I(
            "CoreService_UnsolicitedNotifyReceived : Service Name[%s]", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_CapabilityQueryReceived(
        IN ICoreService* piService, IN ICapabilities* piCapabilities)
{
    (void)piService;
    (void)piCapabilities;

    IMS_TRACE_I("CoreService_CapabilityQueryReceived : Service Name[%s]", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan)
{
    IMS_TRACE_I("ImsAos_Connected : slot_[%d], ServiceType[%s]", m_objContext.GetSlotId(),
            PS_ServiceType(m_eServiceType), 0);
    (void)nFeatures;
    (void)nIpcan;

    m_piMtsServiceState->OnImsConnected();

    if ((m_eServiceType == MtsServiceType::NORMAL) || (!IsEmergencySmsReadyToSend()))
    {
        return;
    }
    else if (m_piImsAos->GetAosInfo()->GetRegistrationMode() != IImsAosInfo::REG_MODE_NORMAL)
    {
        IJniMtsAppThread* piAppThread = m_objContext.GetJniAppThread();
        if (piAppThread)
        {
            IMS_TRACE_E(0, "Emergency Registration Mode is not NORMAL", 0, 0, 0);
            piAppThread->ReportMoStatus(MO_ERROR_GENERIC, m_pSmsInfo->eSmsFormat,
                    m_pSmsInfo->nSeqId, m_objContext.GetSlotId());
        }
        m_pSmsInfo.reset();
        return;
    }

    switch (StartMoTrafficIfNeeded())
    {
        case MtsTrafficStartResult::TRAFFIC_READY:
            m_objContext.GetMessageController().ProcessMoSms(m_pSmsInfo->eSmsFormat,
                    m_pSmsInfo->pContent, m_pSmsInfo->strAddress, m_pSmsInfo->nSeqId,
                    m_pSmsInfo->bEmergencyNumber, m_eServiceType, m_pSmsInfo->nRetryCount);
            break;

        case MtsTrafficStartResult::TRAFFIC_AWAITING_SETUP:
            // MO SMS request already stored. Reset in Traffic_OnConnectionSetupPrepared.
            break;

        default:  // TRAFFIC_NOT_ALLOWED, TRAFFIC_NOT_FOUND
        {
            IJniMtsAppThread* piAppThread = m_objContext.GetJniAppThread();
            if (piAppThread)
            {
                piAppThread->ReportMoStatus(MO_ERROR_RETRY, m_pSmsInfo->eSmsFormat,
                        m_pSmsInfo->nSeqId, m_objContext.GetSlotId());
            }
            m_pSmsInfo.reset();
            break;
        }
    }
}

PUBLIC
void MtsService::ImsAos_Connecting() {}

PUBLIC
void MtsService::ImsAos_Disconnected(IN IMS_UINT32 nReason, IN IMS_SINT32 /* nDataFailureReason */)
{
    IMS_TRACE_I("ImsAos_Disconnected : Reason[%d], ServiceType[%s]", nReason,
            PS_ServiceType(m_eServiceType), 0);

    m_piMtsServiceState->OnImsDisconnected(nReason);

    MtsServiceType eServiceTypeUsed = IsSmsOverEmergencyPdnSupported()
            ? MtsServiceType::EMERGENCY : MtsServiceType::NORMAL;
    if (eServiceTypeUsed == m_eServiceType)
    {
        m_objContext.GetMessageController().TriggerEmergencySmsStateNotification(
                IMS_FALSE, m_objContext.GetMessageController().GetLastEmergencyMessageReference());
    }

    if (m_eServiceType == MtsServiceType::NORMAL)
    {
        // if ims data connection is disconnected, terminate all pending messages.
        m_objContext.GetMessageController().ClearAllMessages();
    }
    else
    {
        if (m_pSmsInfo == IMS_NULL)
        {
            return;
        }
        IJniMtsAppThread* piAppThread = m_objContext.GetJniAppThread();
        if (piAppThread)
        {
            piAppThread->ReportMoStatus(MO_ERROR_GENERIC, m_pSmsInfo->eSmsFormat,
                    m_pSmsInfo->nSeqId, m_objContext.GetSlotId());
        }
        m_pSmsInfo.reset();
    }
}

PUBLIC
void MtsService::ImsAos_Disconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Disconnecting : Reason[%d], ServiceType[%s]", nReason,
            PS_ServiceType(m_eServiceType), 0);

    m_piMtsServiceState->OnImsDisconnecting(nReason);
}

PUBLIC
void MtsService::ImsAos_Suspended(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Suspended : Reason[%d], ServiceType[%s]", nReason,
            PS_ServiceType(m_eServiceType), 0);

    m_piMtsServiceState->OnImsSuspended(nReason);

    if (m_eServiceType == MtsServiceType::NORMAL)
    {
        // if ims data connection is suspended, terminate all pending messages.
        m_objContext.GetMessageController().ClearAllMessages();
    }
}

PUBLIC
void MtsService::ImsAos_Resumed()
{
    IMS_TRACE_I("ImsAos_Resumed : ServiceType[%s]", PS_ServiceType(m_eServiceType), 0, 0);

    m_piMtsServiceState->OnImsResumed();
}

PUBLIC
void MtsService::RequestRegistrationRecovery(IN IMS_UINT32 nRecoveryType) const
{
    if (m_piImsAos != IMS_NULL)
    {
        IMS_SINT32 nAosControlPolicy =
                m_objContext.GetDynamicLoader().GetMtsAosUtils()->ConvertToAosControl(
                        nRecoveryType);
        if (nAosControlPolicy != MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE)
        {
            m_piImsAos->Control(nAosControlPolicy);
        }
    }
    else
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
    }
}

PUBLIC
void MtsService::RequestRegisterWithNextPcscf(IN const IMS_UINT32 nRetryAfterValue) const
{
    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->RegisterWithNextPcscf(nRetryAfterValue);
    }
    else
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
    }
}

PUBLIC
void MtsService::Traffic_OnConnectionFailed(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection,
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    IMS_TRACE_I("Traffic_OnConnectionFailed", 0, 0, 0);
    /*
     * TODO(Mts): Consider of nFailureReason, nCauseCode and nWaitTimeMillis
     *            if there are some further required actions
     */
    (void)nType;
    (void)nDirection;
    (void)nFailureReason;
    (void)nCauseCode;
    (void)nWaitTimeMillis;

    m_pSmsInfo.reset();
    m_objContext.GetMessageController().ClearAllMessages();
}

PUBLIC
void MtsService::Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection)
{
    IMS_TRACE_I("Traffic_OnConnectionSetupPrepared", 0, 0, 0);

    if (nDirection == IImsRadio::DIRECTION_MT)
    {
        // Mts does not wait `Traffic_OnConnectionSetupPrepared()` for MT SMS case
        return;
    }

    if (m_pSmsInfo == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pSmsInfo is null", 0, 0, 0);
        return;
    }

    IMtsTraffic* piMoTraffic = GetTraffic(nType, nDirection);
    if (piMoTraffic == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can not get the matched traffic", 0, 0, 0);
        m_pSmsInfo.reset();
        return;
    }

    piMoTraffic->StartRadioGuardTimer();
    m_objContext.GetMessageController().ProcessMoSms(m_pSmsInfo->eSmsFormat, m_pSmsInfo->pContent,
            m_pSmsInfo->strAddress, m_pSmsInfo->nSeqId, m_pSmsInfo->bEmergencyNumber,
            m_eServiceType, m_pSmsInfo->nRetryCount);
    m_pSmsInfo.reset();
}

PUBLIC
void MtsService::Traffic_GuardTimerExpired(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection)
{
    IMtsTraffic* piMtsTraffic = GetTraffic(nType, nDirection);
    if (piMtsTraffic == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can not get the matched traffic", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("Traffic_GuardTimerExpired : StopRadioTraffic[%s][%s]", PS_TrafficType(nType),
            PS_TrafficDirection(nDirection), 0);
    m_piImsRadio->StopImsTraffic(piMtsTraffic);
}

PUBLIC
void MtsService::NotifyEmergencySmsStateToAos(IN IMS_BOOL bInitialized) const
{
    IMS_TRACE_I("NotifyEmergencySmsStateToAos", 0, 0, 0);

    if (m_piImsAos == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
        return;
    }

    IImsAosInfo* pIImsAosInfo = m_piImsAos->GetAosInfo();
    if (pIImsAosInfo == IMS_NULL)
    {
        IMS_TRACE_E(0, "pIImsAosInfo is null", 0, 0, 0);
        return;
    }

    EmergencyServicePdn eEmergencyServicePdn = EmergencyServicePdn::IMS;
    if (IsSmsOverEmergencyPdnSupported())
    {
        eEmergencyServicePdn = EmergencyServicePdn::EMERGENCY;
    }

    pIImsAosInfo->NotifyEmergencySmsState(bInitialized, eEmergencyServicePdn);
}

PUBLIC
IMS_BOOL MtsService::IsWlan() const
{
    IMS_UINT32 nReportedIpcan = IIpcan::CATEGORY_MOBILE;
    if (m_piImsAos != IMS_NULL)
    {
        nReportedIpcan = m_piImsAos->GetAosInfo()->GetIpcanType();
    }
    else
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
    }
    return nReportedIpcan == IIpcan::CATEGORY_WLAN;
}

PRIVATE
void MtsService::AttachAos()
{
    IMS_TRACE_I("AttachAos : ServiceType[%s]", PS_ServiceType(m_eServiceType), 0, 0);
    ImsServiceId eTargetServiceId = (m_eServiceType == MtsServiceType::NORMAL)
            ? ImsServiceId::MTS
            : ImsServiceId::MTS_EMERGENCY;
    m_piImsAos = ImsAos::GetImsAos(m_strAppId, ImsServiceConfig::GetServiceName(eTargetServiceId),
            m_objContext.GetSlotId());

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(this);
    }
    else
    {
        IMS_TRACE_E(0, "AttachAos : m_piImsAos is null, ServiceType[%s]",
                PS_ServiceType(m_eServiceType), 0, 0);
    }
}

PRIVATE
void MtsService::AttachCoreService()
{
    IMS_TRACE_I("AttachCoreService : ServiceType[%s]", PS_ServiceType(m_eServiceType), 0, 0);
    ImsServiceId eTargetServiceId = (m_eServiceType == MtsServiceType::NORMAL)
            ? ImsServiceId::MTS
            : ImsServiceId::MTS_EMERGENCY;
    AString strServiceId("serviceId=");
    strServiceId += ImsServiceConfig::GetServiceName(eTargetServiceId);

    m_piCoreService = DYNAMIC_CAST(
            ICoreService*, (Connector::Open(ImsCore::CONNECTION_SCHEME, m_strAppId, strServiceId)));

    if (m_piCoreService != IMS_NULL)
    {
        m_piCoreService->SetListener(this);

        //// iFC normal -- starts
        // It MUST be applied if the feature-tag property is not supported (no Accept-Contact).
        IServiceFilterCriteria* piSfc = m_piCoreService->GetFilterCriteria();

        if (piSfc != IMS_NULL)
        {
            SipMethod objMethod(SipMethod::MESSAGE);
            TriggerPoint objTp(objMethod);

            // Add iFC for 3GPP2 SMS format (Content-Type: application/vnd.3gpp2.sms)
            objTp.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp2.sms");
            piSfc->AddTriggerPoint(objTp);

            // Add iFC for 3GPP SMS format (Content-Type: application/vnd.3gpp.sms)
            objTp.RemoveAllHeaders();
            objTp.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp.sms");
            piSfc->AddTriggerPoint(objTp);
        }
        //// iFC -- ends
    }
    else
    {
        IMS_TRACE_E(0, "AttachCoreService : m_piCoreService is null, ServiceType[%s]",
                PS_ServiceType(m_eServiceType), 0, 0);
    }
}

PRIVATE
void MtsService::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);
    if (m_piCoreService != IMS_NULL)
    {
        m_piCoreService->Close();
        m_piCoreService = IMS_NULL;
    }

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(IMS_NULL);
        m_piImsAos = IMS_NULL;
    }

    if (m_piMtsServiceState != IMS_NULL)
    {
        delete m_piMtsServiceState;
    }

    for (IMS_UINT32 i = 0; i < m_objMtsTraffics.GetSize(); i++)
    {
        IMtsTraffic* piTmpMtsTraffic = m_objMtsTraffics.GetAt(i);

        if (piTmpMtsTraffic)
        {
            delete piTmpMtsTraffic;
        }
    }

    m_objMtsTraffics.Clear();
}

PRIVATE
void MtsService::InitMtsServiceState()
{
    m_piMtsServiceState->Init(m_piImsAos);
}

PRIVATE
IMS_UINT32 MtsService::GetCurrentAccessNetworkType() const
{
    IMS_UINT32 nResult;

    if (IsWlan())
    {
        nResult = IImsRadio::ACCESS_NETWORK_TYPE_IWLAN;
    }
    else
    {
        switch (m_objContext.GetNetworkTracker().GetNetworkType())
        {
            case INetworkWatcher::RADIOTECH_TYPE_HSPA:
            case INetworkWatcher::RADIOTECH_TYPE_HSPAP:
            case INetworkWatcher::RADIOTECH_TYPE_UMTS:
                nResult = IImsRadio::ACCESS_NETWORK_TYPE_UTRAN;
                break;

            case INetworkWatcher::RADIOTECH_TYPE_LTE:
                nResult = IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN;
                break;

            case INetworkWatcher::RADIOTECH_TYPE_NR:
                nResult = IImsRadio::ACCESS_NETWORK_TYPE_NGRAN;
                break;

            default:
                nResult = IImsRadio::ACCESS_NETWORK_TYPE_UNKNOWN;
                break;
        }
    }

    IMS_TRACE_I("GetCurrentAccessNetworkType : Result[%s]", PS_AccessNetworkType(nResult), 0, 0);

    return nResult;
}

PRIVATE
IMtsTraffic* MtsService::GetTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nDirection)
{
    for (IMS_UINT32 i = 0; i < m_objMtsTraffics.GetSize(); i++)
    {
        IMtsTraffic* piTmpMtsTraffic = m_objMtsTraffics.GetAt(i);

        if ((piTmpMtsTraffic->GetDirection() == nDirection) &&
                (piTmpMtsTraffic->GetTrafficType() == nTrafficType))
        {
            return piTmpMtsTraffic;
        }
    }

    return IMS_NULL;
}

PRIVATE
IMS_UINT32 MtsService::GetTrafficTypeOfService() const
{
    return (m_eServiceType == MtsServiceType::NORMAL) ? IImsRadio::TRAFFIC_TYPE_SMS
                                                      : IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS;
}

PRIVATE
IMS_BOOL MtsService::IsEmergencySmsReadyToSend() const
{
    return (m_pSmsInfo != IMS_NULL && m_pSmsInfo->bEmergencyNumber);
}

PRIVATE
MtsTrafficStartResult MtsService::StartMtTraffic()
{
    IMS_UINT32 nTrafficType = GetTrafficTypeOfService();
    IMtsTraffic* piMtTraffic = GetTraffic(nTrafficType, IImsRadio::DIRECTION_MT);
    if (piMtTraffic == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can not get the matched traffic", 0, 0, 0);
        return MtsTrafficStartResult::TRAFFIC_NOT_FOUND;
    }

    // In case of MT SMS, there's no need to check `IsImsTrafficAllowed()` and wait for
    // `Traffic_OnConnectionSetupPrepared()`, as the SMS traffic has already been allowed.
    if (!piMtTraffic->IsRadioGuardTimerActive())
    {
        IMS_TRACE_I("StartMtTraffic : StartImsTraffic[%s][%s]",
                PS_TrafficType(piMtTraffic->GetTrafficType()),
                PS_TrafficDirection(piMtTraffic->GetDirection()), 0);
        m_piImsRadio->StartImsTraffic(
                nTrafficType, GetCurrentAccessNetworkType(), IImsRadio::DIRECTION_MT, piMtTraffic);
    }

    piMtTraffic->StartRadioGuardTimer();

    return MtsTrafficStartResult::TRAFFIC_READY;
}

PRIVATE
MtsTrafficStartResult MtsService::StartMoTrafficIfNeeded()
{
    IMS_UINT32 nTrafficType = GetTrafficTypeOfService();
    IMtsTraffic* piMoTraffic = GetTraffic(nTrafficType, IImsRadio::DIRECTION_MO);
    if (piMoTraffic == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can not get the matched traffic", 0, 0, 0);
        return MtsTrafficStartResult::TRAFFIC_NOT_FOUND;
    }

    if (piMoTraffic->IsRadioGuardTimerActive())
    {
        piMoTraffic->StartRadioGuardTimer();
        return MtsTrafficStartResult::TRAFFIC_READY;
    }

    if (m_piImsRadio->IsImsTrafficAllowed(nTrafficType))
    {
        IMS_UINT32 nDirection = piMoTraffic->GetDirection();
        IMS_TRACE_I("StartMoTrafficIfNeeded : StartImsTraffic[%s][%s]",
                PS_TrafficType(nTrafficType), PS_TrafficDirection(nDirection), 0);
        m_piImsRadio->StartImsTraffic(
                nTrafficType, GetCurrentAccessNetworkType(), nDirection, piMoTraffic);
        return MtsTrafficStartResult::TRAFFIC_AWAITING_SETUP;
    }
    else
    {
        IMS_TRACE_E(0, "RadioTraffic was not allowed", 0, 0, 0);
        return MtsTrafficStartResult::TRAFFIC_NOT_ALLOWED;
    }
}

PRIVATE
IMS_BOOL MtsService::IsSmsOverEmergencyPdnSupported() const
{
    return ConfigService::GetConfigService()
            ->GetCarrierConfig(m_objContext.GetSlotId())
            ->GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL);
}
