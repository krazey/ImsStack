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
#include "IJniMtsServiceThread.h"
#include "IMtsContext.h"
#include "IServiceFilterCriteria.h"
#include "ISipHeader.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsCore.h"
#include "ImsServiceConfig.h"
#include "IuMtsService.h"
#include "JniEnablerConnector.h"
#include "MtsDef.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "MtsStringDef.h"
#include "MtsTraffic.h"
#include "ServiceImsRadio.h"
#include "ServicePhoneInfo.h"
#include "message/IMtsMessageController.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTS__;

MtsService::MtsService(IN IMtsContext& objContext) :
        ImsService(AString::ConstNull()),
        m_objContext(objContext),
        m_piImsAos(IMS_NULL),
        m_piImsEmergencyAos(IMS_NULL),
        m_piNetWatcherInfo(IMS_NULL),
        m_strAppId(ImsServiceConfig::GetAppName(ImsAppId::MTS)),
        m_piCoreService(IMS_NULL),
        m_piEmergencyCoreService(IMS_NULL),
        m_piMtsServiceState(new MtsServiceState(m_objContext.GetSlotId())),
        m_piImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(m_objContext.GetSlotId())),
        m_objMtsTraffics(ImsList<IMtsTraffic*>()),
        m_pSmsInfo(IMS_NULL)
{
    IMS_TRACE_I("+MtsService [slot_%d]", m_objContext.GetSlotId(), 0, 0);
}

PUBLIC
MtsService::~MtsService()
{
    IMS_TRACE_I("~MtsService [slot_%d]", m_objContext.GetSlotId(), 0, 0);

    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_objContext.GetSlotId(), EnablerType::MTS_SERVICE, IMS_NULL);

    DeInit();
}

PUBLIC
void MtsService::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    AttachJni();
    AttachAos();
    AttachCoreService();
    InitMtsServiceState();
    m_piNetWatcherInfo =
            PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_objContext.GetSlotId());
    m_objMtsTraffics.Append(new MtsTraffic(
            m_objContext, IImsRadio::DIRECTION_MO, IImsRadio::TRAFFIC_TYPE_SMS, *this));
    m_objMtsTraffics.Append(new MtsTraffic(
            m_objContext, IImsRadio::DIRECTION_MT, IImsRadio::TRAFFIC_TYPE_SMS, *this));
    m_objMtsTraffics.Append(new MtsTraffic(
            m_objContext, IImsRadio::DIRECTION_MO, IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, *this));
    m_objMtsTraffics.Append(new MtsTraffic(
            m_objContext, IImsRadio::DIRECTION_MT, IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, *this));
}

PUBLIC
ICoreService* MtsService::GetICoreService(IN IMS_BOOL bEmergency) const
{
    return bEmergency ? m_piEmergencyCoreService : m_piCoreService;
}

PUBLIC VIRTUAL IJniMtsServiceThread* MtsService::GetJniServiceThread() const
{
    IJniEnabler* piJniEnabler = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_objContext.GetSlotId(), EnablerType::MTS_SERVICE);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtsServiceThread is null", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtsServiceThread*>(piJniEnabler->GetJniThread());
}

PUBLIC VIRTUAL void MtsService::SendMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
        IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency)
{
    IMS_TRACE_I("SendMoSms", 0, 0, 0);

    if (bEmergency && IsEmergencySmsOverImsSupported())
    {
        // Wait until the Emergency REGISTRATION Procedure done
        m_piImsEmergencyAos->Control(ImsAosControl::REGISTER_START);

        // Will be reset in Traffic_OnConnectionSetupPrepared upon successful connection setup.
        m_pSmsInfo = std::make_unique<SmsSendRequestInfo>(
                eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
        return;
    }

    switch (StartMoTrafficIfNeeded(bEmergency))
    {
        case MtsTrafficStartResult::TRAFFIC_READY:
            m_objContext.GetMessageController().ProcessMoSms(
                    eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
            break;

        case MtsTrafficStartResult::TRAFFIC_AWAITING_SETUP:
            // Radio connection is not yet ready. Store MO SMS request and this will be reset in
            // Traffic_OnConnectionSetupPrepared upon successful connection setup.
            m_pSmsInfo = std::make_unique<SmsSendRequestInfo>(
                    eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
            break;

        default:  // TRAFFIC_NOT_ALLOWED, TRAFFIC_NOT_FOUND
        {
            IJniMtsServiceThread* piServiceThread = GetJniServiceThread();
            if (piServiceThread)
            {
                piServiceThread->ReportMoStatus(
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
    IMS_TRACE_I("CoreService_PageMessageReceived : SMS message has been received", 0, 0, 0);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived : no IPageMessage", 0, 0, 0);
        return;
    }

    if (StartMtTraffic(piService) != MtsTrafficStartResult::TRAFFIC_READY)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived: Failed to handle SMS traffic.", 0, 0, 0);
        return;
    }

    m_objContext.GetMessageController().ProcessMtSms(piMessage);
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
    IMS_TRACE_I("ImsAos_Connected : slot_[%d]", m_objContext.GetSlotId(), 0, 0);
    (void)nFeatures;
    (void)nIpcan;

    m_piMtsServiceState->OnImsConnected();

    if (!IsEmergencySmsReadyToSend())
    {
        return;
    }

    switch (StartMoTrafficIfNeeded(m_pSmsInfo->bEmergency))
    {
        case MtsTrafficStartResult::TRAFFIC_READY:
            m_objContext.GetMessageController().ProcessMoSms(m_pSmsInfo->eSmsFormat,
                    m_pSmsInfo->pContent, m_pSmsInfo->strAddress, m_pSmsInfo->nSeqId,
                    m_pSmsInfo->bEmergency);
            break;

        case MtsTrafficStartResult::TRAFFIC_AWAITING_SETUP:
            // MO SMS request already stored. Reset in Traffic_OnConnectionSetupPrepared.
            break;

        default:  // TRAFFIC_NOT_ALLOWED, TRAFFIC_NOT_FOUND
        {
            IJniMtsServiceThread* piServiceThread = GetJniServiceThread();
            if (piServiceThread)
            {
                piServiceThread->ReportMoStatus(MO_ERROR_RETRY, m_pSmsInfo->eSmsFormat,
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
void MtsService::ImsAos_Disconnected(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Disconnected : Reason[%d]", nReason, 0, 0);

    m_piMtsServiceState->OnImsDisconnected(nReason);
    // if ims data connection is disconnected, terminate all pending messages.
    m_objContext.GetMessageController().ClearAllMessages();
}

PUBLIC
void MtsService::ImsAos_Disconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Disconnecting : Reason[%d]", nReason, 0, 0);

    m_piMtsServiceState->OnImsDisconnecting(nReason);
}

PUBLIC
void MtsService::ImsAos_Suspended(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Suspended : Reason[%d]", nReason, 0, 0);

    m_piMtsServiceState->OnImsSuspended(nReason);
    m_objContext.GetMessageController().ClearAllMessages();
}

PUBLIC
void MtsService::ImsAos_Resumed()
{
    IMS_TRACE_I("ImsAos_Resumed", 0, 0, 0);

    m_piMtsServiceState->OnImsResumed();
}

PUBLIC
void MtsService::RequestRegistrationRecovery(IN IMS_UINT32 nRecoveryType)
{
    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->Control(nRecoveryType);
    }
    else
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
    }
}

PUBLIC
void MtsService::RequestRegisterWithNextPcscf(IN const IMS_UINT32 nRetryAfterValue)
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
            m_pSmsInfo->strAddress, m_pSmsInfo->nSeqId, m_pSmsInfo->bEmergency);
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
void MtsService::InitMtsServiceState()
{
    m_piMtsServiceState->Init(m_piImsAos);
}

PRIVATE
void MtsService::AttachJni()
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_objContext.GetSlotId(), EnablerType::MTS_SERVICE, this);
}

PRIVATE
void MtsService::AttachAos()
{
    // Get the normal-type interface of AoS and register as the listener.
    m_piImsAos = ImsAos::GetImsAos(m_strAppId, ImsServiceConfig::GetServiceName(ImsServiceId::MTS),
            m_objContext.GetSlotId());

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(this);
    }
    else
    {
        IMS_TRACE_E(0, "AttachAos : m_piImsAos is null", 0, 0, 0);
    }

    // Get the emergency-type interface of AoS and register as the listener.
    m_piImsEmergencyAos = ImsAos::GetImsAos(m_strAppId,
            ImsServiceConfig::GetServiceName(ImsServiceId::MTS_EMERGENCY),
            m_objContext.GetSlotId());

    if (m_piImsEmergencyAos != IMS_NULL)
    {
        m_piImsEmergencyAos->SetListener(this);
    }
    else
    {
        IMS_TRACE_E(0, "AttachAos : m_piImsEmergencyAos is null", 0, 0, 0);
    }
}

PRIVATE
void MtsService::AttachCoreService()
{
    // Get the normal-type ICoreService and register as the listener.
    AString strServiceId("serviceId=");
    strServiceId += ImsServiceConfig::GetServiceName(ImsServiceId::MTS);

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
        IMS_TRACE_E(0, "AttachCoreService : m_piCoreService is null", 0, 0, 0);
    }

    // Get the emergency-type ICoreService and register as the listener.
    AString strEmergencyServiceId("serviceId=");
    strEmergencyServiceId += ImsServiceConfig::GetServiceName(ImsServiceId::MTS_EMERGENCY);

    m_piEmergencyCoreService = DYNAMIC_CAST(ICoreService*,
            (Connector::Open(ImsCore::CONNECTION_SCHEME, m_strAppId, strEmergencyServiceId)));

    if (m_piEmergencyCoreService != IMS_NULL)
    {
        m_piEmergencyCoreService->SetListener(this);

        //// iFC emergency -- starts
        // It MUST be applied if the feature-tag property is not supported (no Accept-Contact).
        IServiceFilterCriteria* piEmergencySfc = m_piEmergencyCoreService->GetFilterCriteria();

        if (piEmergencySfc != IMS_NULL)
        {
            SipMethod objMethod(SipMethod::MESSAGE);
            TriggerPoint objTp(objMethod);

            // Add iFC for 3GPP2 SMS format (Content-Type: application/vnd.3gpp2.sms)
            objTp.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp2.sms");
            piEmergencySfc->AddTriggerPoint(objTp);

            // Add iFC for 3GPP SMS format (Content-Type: application/vnd.3gpp.sms)
            objTp.RemoveAllHeaders();
            objTp.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp.sms");
            piEmergencySfc->AddTriggerPoint(objTp);
        }
        //// iFC -- ends
    }
    else
    {
        IMS_TRACE_E(0, "AttachCoreService : m_piEmergencyCoreService is null", 0, 0, 0);
    }
}

PRIVATE
IMS_UINT32 MtsService::ConvertToAccessNetworkType(
        IN IMS_UINT32 nTrafficType, IN IMS_SINT32 nReportedNetwork)
{
    IMS_UINT32 nReportedIpcan = IIpcan::CATEGORY_MOBILE;

    if (nTrafficType == IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS)
    {
        if (m_piImsEmergencyAos != IMS_NULL)
        {
            nReportedIpcan = m_piImsEmergencyAos->GetAosInfo()->GetIpcanType();
        }
        else
        {
            IMS_TRACE_E(0, "m_piImsEmergencyAos is null", 0, 0, 0);
        }
    }
    else
    {
        if (m_piImsAos != IMS_NULL)
        {
            nReportedIpcan = m_piImsAos->GetAosInfo()->GetIpcanType();
        }
        else
        {
            IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
        }
    }

    IMS_UINT32 nResult;

    if (nReportedIpcan == IIpcan::CATEGORY_WLAN)
    {
        nResult = IImsRadio::ACCESS_NETWORK_TYPE_IWLAN;
    }
    else
    {
        switch (nReportedNetwork)
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

    IMS_TRACE_I("ConvertToAccessNetworkType : Network[%s], Ipcan[%s], AccessNetwork[%s]",
            PS_RadioTechType(nReportedNetwork), PS_Ipcan(nReportedIpcan),
            PS_AccessNetworkType(nResult));

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
IMS_BOOL MtsService::IsEmergencySmsOverImsSupported() const
{
    return ConfigService::GetConfigService()
            ->GetCarrierConfig(m_objContext.GetSlotId())
            ->GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL);
}

PRIVATE
IMS_BOOL MtsService::IsEmergencySmsReadyToSend() const
{
    return (m_piImsEmergencyAos != IMS_NULL && m_piImsEmergencyAos->IsImsConnected() == IMS_TRUE &&
            m_pSmsInfo != IMS_NULL && m_pSmsInfo->bEmergency);
}

PRIVATE
MtsTrafficStartResult MtsService::StartMtTraffic(IN ICoreService* piService)
{
    IMS_UINT32 nTrafficType;
    if (piService == m_piCoreService)
    {
        nTrafficType = IImsRadio::TRAFFIC_TYPE_SMS;
    }
    else if (piService == m_piEmergencyCoreService)
    {
        nTrafficType = IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS;
    }
    else
    {
        return MtsTrafficStartResult::TRAFFIC_NOT_FOUND;
    }

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
        IMS_UINT32 nAccessNetworkType =
                ConvertToAccessNetworkType(nTrafficType, m_piNetWatcherInfo->GetNetworkType());
        m_piImsRadio->StartImsTraffic(
                nTrafficType, nAccessNetworkType, IImsRadio::DIRECTION_MT, piMtTraffic);
    }

    piMtTraffic->StartRadioGuardTimer();

    return MtsTrafficStartResult::TRAFFIC_READY;
}

PRIVATE
MtsTrafficStartResult MtsService::StartMoTrafficIfNeeded(IN IMS_BOOL bEmergency)
{
    IMS_UINT32 nTrafficType = (bEmergency && IsEmergencySmsOverImsSupported())
            ? IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS
            : IImsRadio::TRAFFIC_TYPE_SMS;

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
        IMS_UINT32 nAccessNetworkType =
                ConvertToAccessNetworkType(nTrafficType, m_piNetWatcherInfo->GetNetworkType());
        m_piImsRadio->StartImsTraffic(nTrafficType, nAccessNetworkType, nDirection, piMoTraffic);
        return MtsTrafficStartResult::TRAFFIC_AWAITING_SETUP;
    }
    else
    {
        IMS_TRACE_E(0, "RadioTraffic was not allowed", 0, 0, 0);
        return MtsTrafficStartResult::TRAFFIC_NOT_ALLOWED;
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

    if (m_piEmergencyCoreService != IMS_NULL)
    {
        m_piEmergencyCoreService->Close();
        m_piEmergencyCoreService = IMS_NULL;
    }

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(IMS_NULL);
        m_piImsAos = IMS_NULL;
    }

    if (m_piImsEmergencyAos != IMS_NULL)
    {
        m_piImsEmergencyAos->SetListener(IMS_NULL);
        m_piImsEmergencyAos = IMS_NULL;
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
