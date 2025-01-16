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
#include "IMtsServiceListener.h"
#include "IServiceFilterCriteria.h"
#include "ISipHeader.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsCore.h"
#include "ImsServiceConfig.h"
#include "IuMtsService.h"
#include "JniEnablerConnector.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "MtsStringDef.h"
#include "MtsTraffic.h"
#include "ServiceImsRadio.h"
#include "ServicePhoneInfo.h"

__IMS_TRACE_TAG_COM_MTS__;

MtsService::MtsService(IN IMS_SINT32 nSlotId) :
        ImsService(AString::ConstNull()),
        m_piImsAos(IMS_NULL),
        m_piImsEmergencyAos(IMS_NULL),
        m_piNetWatcherInfo(IMS_NULL),
        m_strAppId(ImsServiceConfig::GetAppName(ImsAppId::MTS)),
        m_nSlotId(nSlotId),
        m_piCoreService(IMS_NULL),
        m_piEmergencyCoreService(IMS_NULL),
        m_piMtsServiceListener(IMS_NULL),
        m_piMtsServiceState(new MtsServiceState(nSlotId)),
        m_piImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(nSlotId)),
        m_objMtsTraffics(ImsList<IMtsTraffic*>()),
        m_pSmsInfo(IMS_NULL)
{
    IMS_TRACE_I("+MtsService [slot_%d]", m_nSlotId, 0, 0);
    Init();
}

PUBLIC
MtsService::~MtsService()
{
    IMS_TRACE_I("~MtsService [slot_%d]", m_nSlotId, 0, 0);

    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::MTS_SERVICE, IMS_NULL);

    DeInit();
}

PUBLIC
ICoreService* MtsService::GetICoreService(IN IMS_BOOL bEmergency) const
{
    return bEmergency ? m_piEmergencyCoreService : m_piCoreService;
}

PUBLIC VIRTUAL void MtsService::SetListener(IN IMtsServiceListener* piMtsServiceListener)
{
    m_piMtsServiceListener = piMtsServiceListener;
}

PUBLIC VIRTUAL void MtsService::SendMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
        IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency)
{
    IMS_TRACE_I("SendMoSms", 0, 0, 0);

    if (bEmergency &&
            ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId)->GetBoolean(
                    CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL))
    {
        if (m_pSmsInfo != IMS_NULL)
        {
            delete m_pSmsInfo;
        }
        m_pSmsInfo = new SmsSendRequestInfo(eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
        m_piImsEmergencyAos->Control(ImsAosControl::REGISTER_START);

        // Wait until the Emergency REGISTRATION Procedure done
        return;
    }

    IMtsTraffic* piMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    if (piMtsTraffic->IsRadioGuardTimerActive())
    {
        piMtsTraffic->StartRadioGuardTimer();
        m_piMtsServiceListener->NotifyMoSms(eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
    }
    else
    {
        if (m_pSmsInfo != IMS_NULL)
        {
            delete m_pSmsInfo;
        }
        m_pSmsInfo = new SmsSendRequestInfo(eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
        StartRadioTraffic(piMtsTraffic);
    }
}

PUBLIC
void MtsService::ReportMoStatus(
        IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId)
{
    IMS_TRACE_I("ReportMoStatus", 0, 0, 0);

    IJniMtsServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->ReportMoStatus(nReason, eSmsFormat, nSeqId, m_nSlotId);
    }
}

PUBLIC
void MtsService::ReportMtSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objContent)
{
    IMS_TRACE_I("ReportMtSms", 0, 0, 0);

    IJniMtsServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->ReportMtSms(eSmsFormat, objContent, m_nSlotId);
    }
}

PUBLIC
void MtsService::CoreService_PageMessageReceived(
        IN ICoreService* piService, IN IPageMessage* piMessage)
{
    IMS_TRACE_I("CoreService_PageMessageReceived : SMS message has been received", 0, 0, 0);

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
        IMS_TRACE_E(0, "CoreService_PageMessageReceived : not my ICoreService", 0, 0, 0);
        return;
    }

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived : no IPageMessage", 0, 0, 0);
        return;
    }

    IMtsTraffic* piMtsTraffic = GetTraffic(nTrafficType, IImsRadio::DIRECTION_MT);
    IMtsTraffic* piMoSideTraffic = GetTraffic(nTrafficType, IImsRadio::DIRECTION_MO);

    // Mts can skip `StartImsTraffic()` for MT SMS if the MO traffic guard timer is running
    if (!piMoSideTraffic->IsRadioGuardTimerActive())
    {
        IMS_UINT32 nAccessNetworkType =
                ConvertToAccessNetworkType(nTrafficType, m_piNetWatcherInfo->GetNetworkType());

        // In case of MT SMS, no need to check `IsImsTrafficAllowed()`
        m_piImsRadio->StartImsTraffic(
                nTrafficType, nAccessNetworkType, IImsRadio::DIRECTION_MT, piMtsTraffic);
        piMtsTraffic->StartRadioGuardTimer();
    }
    m_piMtsServiceListener->NotifyMtSms(piMessage);
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
    IMS_TRACE_I("ImsAos_Connected : m_nSlotId[%d]", m_nSlotId, 0, 0);
    (void)nFeatures;
    (void)nIpcan;

    m_piMtsServiceState->OnImsConnected();

    if (m_piImsEmergencyAos->IsImsConnected() == IMS_TRUE)
    {
        if ((m_pSmsInfo != IMS_NULL) && m_pSmsInfo->bEmergency)
        {
            IMtsTraffic* piMtsTraffic =
                    GetTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);

            if (piMtsTraffic->IsRadioGuardTimerActive())
            {
                piMtsTraffic->StartRadioGuardTimer();
                m_piMtsServiceListener->NotifyMoSms(m_pSmsInfo->eSmsFormat, m_pSmsInfo->pContent,
                        m_pSmsInfo->strAddress, m_pSmsInfo->nSeqId, m_pSmsInfo->bEmergency);

                delete m_pSmsInfo;
                m_pSmsInfo = IMS_NULL;
            }
            else
            {
                StartRadioTraffic(piMtsTraffic);
            }
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
    m_piMtsServiceListener->OnServiceDisconnected();
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
    m_piMtsServiceListener->OnServiceSuspended();
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

    // TODO(Mts): This notification will be replaced to IMtsMessageController::ClearAllMessages().
    if (m_pSmsInfo != IMS_NULL)
    {
        delete m_pSmsInfo;
        m_pSmsInfo = IMS_NULL;
    }
    m_piMtsServiceListener->OnServiceDisconnected();
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

    IMtsTraffic* piMtsTraffic = GetTraffic(nType, nDirection);
    if (piMtsTraffic == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can not get the matched traffic", 0, 0, 0);
        return;
    }

    piMtsTraffic->StartRadioGuardTimer();
    m_piMtsServiceListener->NotifyMoSms(m_pSmsInfo->eSmsFormat, m_pSmsInfo->pContent,
            m_pSmsInfo->strAddress, m_pSmsInfo->nSeqId, m_pSmsInfo->bEmergency);

    delete m_pSmsInfo;
    m_pSmsInfo = IMS_NULL;
}

PUBLIC
void MtsService::Traffic_GuardTimerExpired(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection)
{
    IMS_TRACE_I("Traffic_GuardTimerExpired : TrafficType[%s], Direction[%s]", PS_TrafficType(nType),
            PS_TrafficDirection(nDirection), 0);

    IMtsTraffic* piMtsTraffic = GetTraffic(nType, nDirection);
    if (piMtsTraffic == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can not get the matched traffic", 0, 0, 0);
        return;
    }

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
    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::MTS_SERVICE, this);
}

PRIVATE
void MtsService::AttachAos()
{
    // Get the normal-type interface of AoS and register as the listener.
    m_piImsAos = ImsAos::GetImsAos(
            m_strAppId, ImsServiceConfig::GetServiceName(ImsServiceId::MTS), m_nSlotId);

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(this);
    }
    else
    {
        IMS_TRACE_E(0, "AttachAos : m_piImsAos is null", 0, 0, 0);
    }

    // Get the emergency-type interface of AoS and register as the listener.
    m_piImsEmergencyAos = ImsAos::GetImsAos(
            m_strAppId, ImsServiceConfig::GetServiceName(ImsServiceId::MTS_EMERGENCY), m_nSlotId);

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
    IMS_TRACE_I("GetTraffic : TrafficType[%s], Direction[%s]", PS_TrafficType(nTrafficType),
            PS_TrafficDirection(nDirection), 0);

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
IJniMtsServiceThread* MtsService::GetJniThread()
{
    IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSlotId, EnablerType::MTS_SERVICE);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtsServiceThread is null", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtsServiceThread*>(piJniEnabler->GetJniThread());
}

PRIVATE
void MtsService::StartRadioTraffic(IN IMtsTraffic* piMtsTraffic)
{
    IMS_UINT32 nDirection = piMtsTraffic->GetDirection();
    IMS_UINT32 nTrafficType = piMtsTraffic->GetTrafficType();

    if (m_piImsRadio->IsImsTrafficAllowed(nTrafficType))
    {
        IMS_TRACE_I("StartRadioTraffic : TrafficType[%s], Direction[%s]",
                PS_TrafficType(nTrafficType), PS_TrafficDirection(nDirection), 0);
        IMS_UINT32 nAccessNetworkType =
                ConvertToAccessNetworkType(nTrafficType, m_piNetWatcherInfo->GetNetworkType());
        m_piImsRadio->StartImsTraffic(nTrafficType, nAccessNetworkType, nDirection, piMtsTraffic);
    }
    else
    {
        IMS_TRACE_E(0, "RadioTraffic was not allowed", 0, 0, 0);
        ReportMoStatus(MO_ERROR_RETRY, m_pSmsInfo->eSmsFormat, m_pSmsInfo->nSeqId);
        delete m_pSmsInfo;
        m_pSmsInfo = IMS_NULL;
    }
}

PRIVATE
void MtsService::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    AttachJni();
    AttachAos();
    AttachCoreService();
    InitMtsServiceState();
    m_piNetWatcherInfo = PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotId);
    m_objMtsTraffics.Append(
            new MtsTraffic(IImsRadio::DIRECTION_MO, IImsRadio::TRAFFIC_TYPE_SMS, *this));
    m_objMtsTraffics.Append(
            new MtsTraffic(IImsRadio::DIRECTION_MT, IImsRadio::TRAFFIC_TYPE_SMS, *this));
    m_objMtsTraffics.Append(
            new MtsTraffic(IImsRadio::DIRECTION_MO, IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, *this));
    m_objMtsTraffics.Append(
            new MtsTraffic(IImsRadio::DIRECTION_MT, IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, *this));
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

    if (m_pSmsInfo != IMS_NULL)
    {
        delete m_pSmsInfo;
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
