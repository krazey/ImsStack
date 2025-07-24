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
#include "Configuration.h"
#include "IMessage.h"
#include "IMtcImsEventReceiver.h"
#include "INetworkWatcher.h"
#include "ISipHeader.h"
#include "ISubscriberConfig.h"
#include "ImsEventDef.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcSession.h"
#include "call/MtcCallStringUtils.h"
#include "call/UpdatingInfo.h"
#include "call/extension/MtcExtensionSet.h"
#include "configuration/MtcConfigurationProxy.h"
#include "media/IMedia.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaStringUtils.h"
#include "media/MtcMediaUtil.h"
#include "precondition/MtcPreconditionManager.h"
#include "precondition/QosStringUtils.h"
#include "precondition/SdpPreconditionHelper.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"
#include <algorithm>
#include <numeric>
#include <vector>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcPreconditionManager::MtcPreconditionManager(IN IMtcCallContext& objContext) :
        m_objQosInfos(ImsMap<ISession*, QosInfo*>()),
        m_pListener(IMS_NULL),
        m_objContext(objContext),
        m_pSdpPreconditionHelper(new SdpPreconditionHelper()),
        m_bOnWlan(objContext.GetService().IsWlanIpCanType()),
        m_ePreviousRatType(m_objContext.GetService().GetMobileRatType()),
        m_eCurrentRatType(m_ePreviousRatType)
{
    IMS_TRACE_D("+MtcPreconditionManager Callkey[%d]", m_objContext.GetCallKey(), 0, 0);
    m_objContext.GetMediaManager().SetQosListener(this);
}

PUBLIC VIRTUAL MtcPreconditionManager::~MtcPreconditionManager()
{
    IMS_TRACE_D("~MtcPreconditionManager Callkey[%d]", m_objContext.GetCallKey(), 0, 0);
    m_objContext.GetMediaManager().SetQosListener(IMS_NULL);
    delete m_pSdpPreconditionHelper;
    DestroyAllQosInfo();
}

PUBLIC VIRTUAL void MtcPreconditionManager::CreateQos(IN ISession* piSession)
{
    IMS_SLONG nIndex = m_objQosInfos.GetIndexOfKey(piSession);
    if (nIndex >= 0)
    {
        IMS_TRACE_D("CreateQos already created", 0, 0, 0);
        return;
    }

    QosInfo* pInfo = new QosInfo(this);
    m_objQosInfos.Add(piSession, pInfo);
    pInfo->SetSupportingPrecondition(IsPreconditionSupportedInLocal());

    IMS_TRACE_D("CreateQos [%d]", m_objQosInfos.GetSize(), 0, 0);
}

PUBLIC VIRTUAL void MtcPreconditionManager::DestroyQos(IN ISession* piSession)
{
    QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return;
    }

    delete pInfo;
    m_objQosInfos.Remove(piSession);

    IMS_TRACE_D("DestroyQos [%d]", m_objQosInfos.GetSize(), 0, 0);
}

PUBLIC VIRTUAL void MtcPreconditionManager::SetListener(IN IMtcPreconditionListener* pListener)
{
    m_pListener = pListener;
}

PUBLIC VIRTUAL void MtcPreconditionManager::InitializeMobileRatInformation()
{
    m_ePreviousRatType = m_objContext.GetService().GetMobileRatType();
    m_eCurrentRatType = m_ePreviousRatType;

    IMS_TRACE_D("InitializeMobileRatInformation previous[%s] current[%s]",
            MtcCallStringUtils::ConvertRatType(m_ePreviousRatType),
            MtcCallStringUtils::ConvertRatType(m_eCurrentRatType), 0);
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsPreconditionSupportedInLocal() const
{
    IMS_BOOL bSupport;
    if (m_objContext.GetCallInfo().bUssi)
    {
        IMS_TRACE_D("IsPreconditionSupportedInLocal USSD over IMS", 0, 0, 0);
        bSupport = IMS_FALSE;
        return bSupport;
    }
    else if (m_objContext.GetCallInfo().IsEmergency())
    {
        bSupport = m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL);
        IMS_TRACE_D("IsPreconditionSupportedInLocal Emergency Call[%s]", _TRACE_B_(bSupport), 0, 0);
        return bSupport;
    }

    CallType eCallType = m_objContext.GetSession()->GetCallType();
    switch (eCallType)
    {
        case CallType::VT:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_VIDEO);
            break;
        case CallType::RTT:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_TEXT);
            break;
        case CallType::VIDEO_RTT:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_VIDEO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_TEXT);
            break;
        default:  // CallType::VOIP
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO);
            break;
    }

    IMS_TRACE_D("IsPreconditionSupportedInLocal CallType[%s][%s]",
            MtcCallStringUtils::ConvertCallType(eCallType), _TRACE_B_(bSupport), 0);
    return bSupport;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsDedicatedBearerAllocated(
        IN ISession* piSession, IN IMS_UINT32 eMediaType) const
{
    QosStatus eStatus = GetQosStatus(piSession, eMediaType);
    IMS_TRACE_D("IsDedicatedBearerAllocated [%s][%s]",
            MtcMediaStringUtils::ConvertContentType(eMediaType),
            QosStringUtils::ConvertQosStatus(eStatus), 0);

    return eStatus == QosStatus::AVAILABLE;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsCheckingResourcesRequiredToAlertUser() const
{
    IMS_TRACE_D("IsCheckingResourcesRequiredToAlertUser", 0, 0, 0);

    if (IsDefaultBearerAllowed(MEDIATYPE_AUDIO))
    {
        return IMS_FALSE;
    }

    return IsPreconditionSupportedInLocal()
            ? IMS_TRUE
            : m_objContext.GetConfigurationProxy().GetBoolean(
                      ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL);
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsAvailableToAlertUser(IN ISession* piSession) const
{
    IMS_TRACE_D("IsAvailableToAlertUser", 0, 0, 0);
    if (piSession == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bLocalReserved = IsLocalResourceReserved(piSession, !IsConfirmedDialog(piSession)) ||
            GetQosTimer(piSession)->IsQosTimerActivated(
                    QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);
    if (!IsPreconditionSupported(piSession))
    {
        return bLocalReserved;
    }
    return bLocalReserved && IsRemoteResourceReserved(piSession);
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsLocalResourceConfirmationRequired(
        IN ISession* piSession) const
{
    if (!IsPreconditionSupported(piSession))
    {
        return IMS_FALSE;
    }

    // IsPreconditionSupported() guarantees that it's not null
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    std::vector<IMS_UINT32> objMediaTypeList =
            MtcMediaUtil::GetMediaTypeListFromCallType(eCallType);

    IMS_BOOL bResult = std::any_of(objMediaTypeList.begin(), objMediaTypeList.end(),
            [this, piSession, pStatusTable](IMS_UINT32 eMediaType)
            {
                return !pStatusTable->IsLocalResourceConfirmed(GetSdpMediaType(eMediaType)) &&
                        IsLocalResourceReservedByMediaType(piSession, eMediaType);
            });

    IMS_TRACE_D("IsLocalResourceConfirmationRequired (%s)", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsAvailableToSendLocalResourceConfirmation(
        IN ISession* piSession) const
{
    QosTimer* pQosTimer = GetQosTimer(piSession);
    if (pQosTimer)
    {
        if (IsLocalResourceReserved(piSession, !IsConfirmedDialog(piSession)) &&
                !pQosTimer->IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE))
        {
            return IMS_TRUE;
        }

        return pQosTimer->IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);
    }

    IMS_TRACE_D("IsAvailableToSendLocalResourceConfirmation : not available", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC VIRTUAL void MtcPreconditionManager::FormPreconditionSdp(
        IN ISession* piSession, IN IMS_BOOL bFailure)
{
    if (!IsPreconditionSupported(piSession))
    {
        IMS_TRACE_D("FormPreconditionSdp : UE don't use precondition mechanism", 0, 0, 0);
        m_pSdpPreconditionHelper->RemovePreconditionSdp(piSession);
        return;
    }

    IMS_TRACE_D("FormPreconditionSdp", 0, 0, 0);
    if (bFailure)
    {
        m_pSdpPreconditionHelper->FormFailurePreconditionSdp(piSession);
        return;
    }

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    ImsList<IMedia*> lstMedias = piSession->GetMedia();
    for (IMS_UINT32 index = 0; index < lstMedias.GetSize(); index++)
    {
        const IMedia* piMedia = lstMedias.GetAt(index);
        const SdpMedia* pLocalSdp = GetSdpMedia(piMedia, IMS_FALSE);
        if (pLocalSdp == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 eSdpMediaType = pLocalSdp->GetType();
        if (pLocalSdp->GetPort() <= 0 || !pStatusTable->GetRecords(eSdpMediaType).IsEmpty())
        {
            continue;
        }

        CreateStatusRecords(
                piSession, m_pSdpPreconditionHelper->GetMediaType(pLocalSdp, piMedia->GetState()));
    }

    m_pSdpPreconditionHelper->FormPreconditionSdp(
            piSession, pStatusTable, IsConfirmationRequired(*piSession));
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnSdpReceived(IN ISession* piSession)
{
    IMS_TRACE_D("OnSdpReceived QoS[%s]", _TRACE_B_(IsPreconditionIncludedInSdp(piSession)), 0, 0);
    UpdateQosAttributesFromRemoteSdp(piSession);

    if (IsNeedToStartWaitAudioDedicatedBearerTimer(piSession, IMS_FALSE))
    {
        StartQosTimer(piSession, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnSdpSent(
        IN ISession* piSession, IN IMS_BOOL bInitialInvite /* = IMS_FALSE */)
{
    IMS_TRACE_D("OnSdpSent", 0, 0, 0);
    if (IsNeedToStartWaitAudioDedicatedBearerTimer(piSession, bInitialInvite))
    {
        StartQosTimer(piSession, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnMessageReceived(
        IN ISession* piSession, IN IMessage* piMessage)
{
    if (!IsPreconditionSupportedInLocal())
    {
        return;
    }

    IMS_TRACE_D("OnMessageReceived", 0, 0, 0);
    IMS_SINT32 nStatusCode = piMessage->GetStatusCode();
    if (SipStatusCode::IsFinalSuccess(nStatusCode) || nStatusCode == SipStatusCode::SC_180)
    {
        SetRemoteResourceAvailable(piSession);
    }

    if (SipStatusCode::IsProvisional(nStatusCode) && !piMessage->GetMessage()->IsMessageRpr())
    {
        return;
    }

    // sets bCheckSdp as true if UE receives RPR, PRACK, early UPDATE and 200 OK to INVITE.
    IMS_BOOL bCheckSdp = !(piMessage->GetMethod().Equals(SipMethod::INVITE) &&
            piMessage->GetMessage()->GetType() == ISipMessage::TYPE_REQUEST);
    IMS_BOOL bSdpIncluded = m_objContext.GetMessageUtils().HasSdp(piMessage);
    if (bCheckSdp && !bSdpIncluded)
    {
        return;
    }

    IMS_BOOL bHasSupportedHeader = m_objContext.GetMessageUtils().HasValue(
            piMessage, MessageUtil::STR_PRECONDITION, ISipHeader::SUPPORTED);
    IMS_BOOL bHasRequireHeader = m_objContext.GetMessageUtils().HasValue(
            piMessage, MessageUtil::STR_PRECONDITION, ISipHeader::REQUIRE);

    IMS_BOOL bRemoteSupported = (bHasSupportedHeader || bHasRequireHeader) &&
            (!bSdpIncluded || m_pSdpPreconditionHelper->IsPreconditionIncludedInSdp(piSession));

    IMS_TRACE_D("OnMessageReceived supported[%s] required[%s] sdp[%s]",
            _TRACE_B_(bHasSupportedHeader), _TRACE_B_(bHasRequireHeader), _TRACE_B_(bSdpIncluded));

    UpdateSupportingPrecondition(piSession, bRemoteSupported);
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnCallEstablished(IN ISession* piSession)
{
    IMS_TRACE_D("OnCallEstablished", 0, 0, 0);

    if (IsNotUsingDedicatedWaitTimerByRatCondition())
    {
        return;
    }

    if (IsVideoOrTextIncluded(m_objContext.GetSession()->GetCallType()) &&
            !IsLocalResourceReservedForVideoOrText(piSession))
    {
        StartQosTimer(piSession, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnCallModified(IN ISession* piSession)
{
    IMS_TRACE_D("OnCallModified", 0, 0, 0);

    IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT);
    if (nPolicy == ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE)
    {
        if (IsNotUsingDedicatedWaitTimerByRatCondition())
        {
            return;
        }

        if (IsVideoOrTextIncluded(m_objContext.GetSession()->GetCallType()) &&
                !IsLocalResourceReservedForVideoOrText(piSession))
        {
            StartQosTimer(piSession, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);
        }
    }

    if (nPolicy != ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_NOT_AVAILABLE)
    {
        // To change Local status from QosStatus::LOST to QosStatus::IDLE for removed medias
        InitializeStatusForUnusedLostQos(piSession);

        // To change remote status to QosStatus::IDLE for removed medias
        QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
        if (pStatusTable != IMS_NULL)
        {
            pStatusTable->RemoveUnusedRecords(MtcMediaUtil::GetMediaTypesFromCallType(
                    m_objContext.GetSession()->GetCallType()));
        }
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnRatChanged(IN IMS_SINT32 eRatType)
{
    IMS_TRACE_D("OnRatChanged RAT type[%s]", MtcCallStringUtils::ConvertRatType(eRatType), 0, 0);
    UpdateMobileRatType(m_objContext.GetService().GetMobileRatType());

    IMS_BOOL bPreviousOnWlan = m_bOnWlan;
    SetOnWlan(eRatType == INetworkWatcher::RADIOTECH_TYPE_IWLAN);
    if (bPreviousOnWlan != m_bOnWlan)  // W2L or L2W
    {
        for (IMS_UINT32 index = 0; index < m_objQosInfos.GetSize(); index++)
        {
            ISession* piSession = m_objQosInfos.GetKeyAt(index);

            if (!m_bOnWlan)  // W2L
            {
                if (GetQosInfo(piSession)->IsWaitAudioDedicatedBearerTimerStarted() &&
                        !IsLocalResourceReserved(piSession, IMS_FALSE))
                {
                    StartQosTimer(piSession, QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);
                }
            }
            else  // L2W
            {
                StopQosTimer(piSession, QosTimerType::GUARD_AFTER_LOST);
                StopQosTimer(piSession, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);

                NotifyQosStatusToListener(
                        piSession, IMS_TRUE, SetLocalResourceAvailable(piSession));
            }
        }
    }

    if (IsEpsFallback())
    {
        if (!m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_RESTART_DEDICATED_BEARER_WAIT_TIMER_BY_EPS_FALLBACK_BOOL))
        {
            return;
        }

        for (IMS_UINT32 index = 0; index < m_objQosInfos.GetSize(); index++)
        {
            ISession* piSession = m_objQosInfos.GetKeyAt(index);

            if (GetQosTimer(piSession)->IsQosTimerActivated(
                        QosTimerType::WAIT_AUDIO_DEDICATED_BEARER))
            {
                StopQosTimer(piSession, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
                StartQosTimer(piSession, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
            }
        }
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnQosStatusChanged(
        IN ISession* piSession, IN QosStatus eStatus, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("OnQosStatusChanged media type[%s][%s]",
            MtcMediaStringUtils::ConvertContentType(eMediaType),
            QosStringUtils::ConvertQosStatus(eStatus), 0);

    QosStatus eCurrStatus = GetQosStatus(piSession, eMediaType);
    if (!IsNeedToUpdateQosStatus(eCurrStatus, eStatus))
    {
        return;
    }

    if (SetQosStatus(piSession, eStatus, eMediaType) == IMS_FAILURE)
    {
        return;
    }
    InitializeStatusForUnusedLostQos(piSession);

    HandleQosTimer(piSession, eCurrStatus, eStatus, eMediaType);

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }
    pStatusTable->UpdateLocalCurrentStatus(
            GetSdpMediaType(eMediaType), IsLocalResourceReservedByMediaType(piSession, eMediaType));

    if ((eCurrStatus == QosStatus::IDLE && eStatus == QosStatus::AVAILABLE) &&
            (!GetQosTimer(piSession)->IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE)))
    {
        NotifyQosStatusToListener(piSession, IMS_TRUE, eMediaType);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnTimerExpired(
        IN QosTimer* pTimer, IN QosTimerType eType)
{
    IMS_TRACE_D("OnTimerExpired [%s]", QosStringUtils::ConvertQosTimerType(eType), 0, 0);

    switch (eType)
    {
        case QosTimerType::WAIT_AUDIO_DEDICATED_BEARER:
            return OnWaitAudioDedicatedBearerTimerExpired(pTimer);
        case QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER:
            return OnWaitAvailableAfterW2LHandoverTimerExpired(pTimer);
        case QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE:
            return OnWaitVideoTextAvailableTimerExpired(pTimer);
        case QosTimerType::GUARD_AFTER_LOST:
            return HandleReservationFailureByTimerExpiration(pTimer);
        default:  // QosTimerType::FORCE_AVAILABLE:
            return OnForceAvailableTimerExpired(pTimer);
    }
}

PROTECTED
QosInfo* MtcPreconditionManager::GetQosInfo(IN ISession* piSession) const
{
    IMS_SLONG nIndex = m_objQosInfos.GetIndexOfKey(piSession);
    return (nIndex >= 0) ? m_objQosInfos.GetValueAt(nIndex) : IMS_NULL;
}

PROTECTED
IMS_BOOL MtcPreconditionManager::IsEpsFallback() const
{
    IMS_TRACE_D("IsEpsFallback pre[%s] curr[%s]",
            MtcCallStringUtils::ConvertRatType(m_ePreviousRatType),
            MtcCallStringUtils::ConvertRatType(m_eCurrentRatType), 0);

    if (m_ePreviousRatType != INetworkWatcher::RADIOTECH_TYPE_NR)
    {
        return IMS_FALSE;
    }

    return (m_eCurrentRatType == INetworkWatcher::RADIOTECH_TYPE_LTE ||
            m_eCurrentRatType == INetworkWatcher::RADIOTECH_TYPE_LTE_CA);
}

PRIVATE
void MtcPreconditionManager::DestroyAllQosInfo()
{
    IMS_TRACE_D("DestroyAllQosInfo", 0, 0, 0);
    for (IMS_UINT32 index = static_cast<IMS_SINT32>(m_objQosInfos.GetSize()); index > 0; index--)
    {
        QosInfo* pInfo = m_objQosInfos.GetValueAt(index - 1);
        m_objQosInfos.RemoveAt(index - 1);
        if (pInfo != IMS_NULL)
        {
            delete pInfo;
        }
    }
}

PRIVATE
IMS_RESULT MtcPreconditionManager::SetQosStatus(
        IN ISession* piSession, QosStatus eStatus, IN IMS_UINT32 eMediaType) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (eMediaType == MEDIATYPE_AUDIO)
    {
        pInfo->SetAudioStatus(eStatus);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        pInfo->SetVideoStatus(eStatus);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        pInfo->SetTextStatus(eStatus);
    }

    return IMS_SUCCESS;
}

PRIVATE
QosStatus MtcPreconditionManager::GetQosStatus(
        IN ISession* piSession, IN IMS_UINT32 eMediaType) const
{
    const QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return QosStatus::IDLE;
    }

    switch (eMediaType)
    {
        case MEDIA_TYPE_AUDIO:
            return pInfo->GetAudioStatus();
        case MEDIA_TYPE_VIDEO:
            return pInfo->GetVideoStatus();
        case MEDIA_TYPE_TEXT:
            return pInfo->GetTextStatus();
        default:
            return QosStatus::IDLE;
    }
}

PRIVATE
QosTimer* MtcPreconditionManager::GetQosTimer(IN ISession* piSession) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return IMS_NULL;
    }

    return &(pInfo->GetTimer());
}

PRIVATE
QosStatusTable* MtcPreconditionManager::GetQosStatusTable(IN ISession* piSession) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return IMS_NULL;
    }

    return &(pInfo->GetStatusTable());
}

PRIVATE
void MtcPreconditionManager::StartQosTimer(IN ISession* piSession, IN QosTimerType eType) const
{
    if (!IsPreconditionSupportedInLocal() &&
            !m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
    {
        IMS_TRACE_D("StartQosTimer Don't use precondition mechanism.", 0, 0, 0);
        return;
    }

    QosTimer* pTimer = GetQosTimer(piSession);
    if (pTimer == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nDuration = GetQosTime(eType);
    if (nDuration < 0 &&
            (eType == QosTimerType::WAIT_AUDIO_DEDICATED_BEARER ||
                    eType == QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
    {
        eType = QosTimerType::FORCE_AVAILABLE;
        nDuration = GetQosTime(eType);
    }

    pTimer->StartQosTimer(eType, nDuration);

    if (eType == QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)
    {
        GetQosInfo(piSession)->SetWaitAudioDedicatedBearerTimerStarted();
    }
}

PRIVATE
void MtcPreconditionManager::StopQosTimer(IN ISession* piSession, IN QosTimerType eType) const
{
    QosTimer* pTimer = GetQosTimer(piSession);
    if (pTimer == IMS_NULL)
    {
        return;
    }

    pTimer->StopQosTimer(eType);
}

PRIVATE
void MtcPreconditionManager::StopAllQosTimer(IN ISession* piSession) const
{
    IMS_TRACE_D("StopAllQosTimer", 0, 0, 0);
    std::vector<QosTimerType> objTimerTypes{QosTimerType::WAIT_AUDIO_DEDICATED_BEARER,
            QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER,
            QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, QosTimerType::GUARD_AFTER_LOST,
            QosTimerType::FORCE_AVAILABLE};

    for (QosTimerType eType : objTimerTypes)
    {
        StopQosTimer(piSession, eType);
    }
}

PRIVATE
void MtcPreconditionManager::OnWaitAudioDedicatedBearerTimerExpired(IN QosTimer* pTimer)
{
    if (m_objContext.GetService().IsWlanIpCanType())
    {
        return;
    }

    if (pTimer->IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
    {
        return;
    }

    HandleReservationFailureByTimerExpiration(pTimer);
}

PRIVATE
void MtcPreconditionManager::OnWaitAvailableAfterW2LHandoverTimerExpired(IN QosTimer* pTimer)
{
    if (pTimer->IsQosTimerActivated(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER))
    {
        return;
    }

    HandleReservationFailureByTimerExpiration(pTimer);
}

PRIVATE
void MtcPreconditionManager::OnWaitVideoTextAvailableTimerExpired(IN const QosTimer* pTimer)
{
    ISession* piSession = GetISessionWithTimer(pTimer);
    if (piSession == IMS_NULL)
    {
        return;
    }

    if (IsConfirmedDialog(piSession))
    {
        return HandleReservationFailureByTimerExpiration(pTimer);
    }

    CallType eCallType = m_objContext.GetSession()->GetCallType();
    std::vector<IMS_UINT32> objMediaTypeList =
            MtcMediaUtil::GetMediaTypeListFromCallType(eCallType);

    IMS_UINT32 eReservedMediaTypes = std::accumulate(objMediaTypeList.begin(),
            objMediaTypeList.end(), static_cast<IMS_UINT32>(MEDIATYPE_NONE),
            [this, piSession](IMS_UINT32 eCurrentMediaTypes, IMS_UINT32 eMediaType)
            {
                if (GetQosStatus(piSession, eMediaType) == QosStatus::AVAILABLE)
                {
                    return eCurrentMediaTypes | eMediaType;
                }
                return eCurrentMediaTypes;
            });

    NotifyQosStatusToListener(piSession, IMS_TRUE, eReservedMediaTypes);
}

PRIVATE
void MtcPreconditionManager::OnForceAvailableTimerExpired(IN const QosTimer* pTimer)
{
    ISession* piSession = GetISessionWithTimer(pTimer);
    if (piSession == IMS_NULL)
    {
        return;
    }

    NotifyQosStatusToListener(piSession, IMS_TRUE, SetLocalResourceAvailable(piSession));
}

PRIVATE
void MtcPreconditionManager::HandleReservationFailureByTimerExpiration(IN const QosTimer* pTimer)
{
    ISession* piSession = GetISessionWithTimer(pTimer);
    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("HandleReservationFailureByTimerExpiration", 0, 0, 0);
    NotifyQosStatusToListener(piSession, IMS_FALSE, MEDIATYPE_NONE);
}

PRIVATE
void MtcPreconditionManager::InitializeStatusForUnusedLostQos(IN ISession* piSession) const
{
    CallType eCallType = m_objContext.GetSession()->GetCallType();

    for (IMS_UINT32 eMediaType : MtcMediaUtil::GetUnusedMediaTypeListFromCallType(eCallType))
    {
        if (GetQosStatus(piSession, eMediaType) == QosStatus::LOST)
        {
            IMS_TRACE_D("InitializeStatusForUnusedLostQos [%s]",
                    MtcMediaStringUtils::ConvertContentType(eMediaType), 0, 0);
            SetQosStatus(piSession, QosStatus::IDLE, eMediaType);
        }
    }
}

PRIVATE
void MtcPreconditionManager::CreateStatusRecordsWithActiveMediaTypes(IN ISession* piSession)
{
    IMS_TRACE_D("CreateStatusRecordsWithActiveMediaTypes", 0, 0, 0);

    CallType eCallType = m_objContext.GetSession()->GetCallType();

    for (IMS_UINT32 eMediaType : MtcMediaUtil::GetMediaTypeListFromCallType(eCallType))
    {
        CreateStatusRecords(piSession, eMediaType);
    }

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable)
    {
        pStatusTable->RemoveUnusedRecords(MtcMediaUtil::GetMediaTypesFromCallType(eCallType));
    }
}

PRIVATE
void MtcPreconditionManager::CreateStatusRecords(IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    if (eMediaType == MEDIATYPE_NONE)
    {
        return;
    }

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    IMS_SINT32 eSdpMediaType = GetSdpMediaType(eMediaType);
    if (!pStatusTable->GetRecords(eSdpMediaType).IsEmpty())
    {
        return;
    }

    IMS_TRACE_D("CreateStatusRecords", 0, 0, 0);
    pStatusTable->InitializeRecords(eSdpMediaType);
    IMS_BOOL bLocalReserved = IsLocalResourceReservedByMediaType(piSession, eMediaType) ||
            GetQosTimer(piSession)->IsQosTimerActivated(
                    QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);

    if (IsConfirmedDialog(piSession))
    {
        IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT);
        IMS_TRACE_D("CreateStatusRecords call upgrade policy[%d]", nPolicy, 0, 0);

        if (nPolicy == ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_NOT_AVAILABLE)
        {
            SetQosStatus(piSession, QosStatus::AVAILABLE, eMediaType);
            bLocalReserved = IsLocalResourceReservedByMediaType(piSession, eMediaType);
        }

        if (nPolicy == ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE)
        {
            bLocalReserved = IMS_TRUE;
        }
    }

    pStatusTable->UpdateLocalCurrentStatus(eSdpMediaType, bLocalReserved);
}

PRIVATE
void MtcPreconditionManager::HandleQosTimer(IN ISession* piSession, IN QosStatus eCurrentStatus,
        IN QosStatus eNewStatus, IN IMS_UINT32 eMediaType) const
{
    IMS_TRACE_D("HandleQosTimer [%s]->[%s]", QosStringUtils::ConvertQosStatus(eCurrentStatus),
            QosStringUtils::ConvertQosStatus(eNewStatus), 0);

    if (IsLocalResourceReserved(piSession, IMS_FALSE))
    {
        StopAllQosTimer(piSession);
        return;
    }

    if (eCurrentStatus == QosStatus::AVAILABLE && eNewStatus == QosStatus::LOST)
    {
        StartQosTimer(piSession, QosTimerType::GUARD_AFTER_LOST);
    }

    if (eCurrentStatus == QosStatus::IDLE && eNewStatus == QosStatus::AVAILABLE &&
            eMediaType == MEDIATYPE_AUDIO)
    {
        StopQosTimer(piSession, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);

        // video or text is not reserved
        StartQosTimer(piSession, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);
    }
}

PRIVATE
void MtcPreconditionManager::NotifyQosStatusToListener(
        IN ISession* piSession, IN IMS_BOOL bReserved, IN IMS_UINT32 eMediaTypes)
{
    if (bReserved && eMediaTypes != MEDIATYPE_NONE)
    {
        m_pListener->QosReserved(piSession, eMediaTypes);
    }
    else
    {
        QosLossPolicy eAction = GetActionForQosLoss(piSession);
        if (eAction != QosLossPolicy::MAINTAIN)
        {
            m_pListener->QosReserveFailed(piSession, eAction);
        }
    }
}

PRIVATE
void MtcPreconditionManager::SetOnWlan(IN IMS_BOOL bOnWlan)
{
    if (bOnWlan != m_bOnWlan)
    {
        IMS_TRACE_D("SetOnWlan [%s]->[%s]", _TRACE_B_(m_bOnWlan), _TRACE_B_(bOnWlan), 0);
        m_bOnWlan = bOnWlan;
    }
}

PRIVATE
void MtcPreconditionManager::UpdateMobileRatType(IN IMS_SINT32 eRatType)
{
    if (eRatType == m_eCurrentRatType)
    {
        return;
    }

    m_ePreviousRatType = m_eCurrentRatType;
    m_eCurrentRatType = eRatType;
}

PRIVATE
void MtcPreconditionManager::SetRemoteResourceAvailable(IN ISession* piSession) const
{
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }

    ImsList<IMedia*> lstMedias = piSession->GetMedia();
    IMS_UINT32 nSize = lstMedias.GetSize();
    IMS_TRACE_D("SetRemoteResourceAvailable media size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        const IMedia* piMedia = lstMedias.GetAt(index);
        if (piMedia == IMS_NULL)
        {
            continue;
        }

        const SdpMedia* pRemoteSdp = GetSdpMedia(piMedia, IMS_TRUE);
        if (pRemoteSdp == IMS_NULL)
        {
            continue;
        }

        pStatusTable->EnableRemoteCurrentStatus(pRemoteSdp->GetType());
    }
}

PRIVATE
void MtcPreconditionManager::UpdateSupportingPrecondition(
        IN ISession* piSession, IN IMS_BOOL bRemoteSupported) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateSupportingPrecondition Remote[%s]", _TRACE_B_(bRemoteSupported), 0, 0);
    pInfo->SetSupportingPrecondition(IsPreconditionSupportedInLocal() && bRemoteSupported);
}

PRIVATE
void MtcPreconditionManager::UpdateQosAttributesFromRemoteSdp(IN ISession* piSession)
{
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateQosAttributesFromRemoteSdp", 0, 0, 0);
    CreateStatusRecordsWithActiveMediaTypes(piSession);

    ImsList<IMedia*> lstMedias = piSession->GetMedia();
    for (IMS_UINT32 index = 0; index < lstMedias.GetSize(); index++)
    {
        const IMedia* piMedia = lstMedias.GetAt(index);
        if (piMedia == IMS_NULL)
        {
            continue;
        }

        if (m_pSdpPreconditionHelper->IsPreconditionIncludedInSdp(piSession))
        {
            pStatusTable->UpdateStatusTableWithRemoteSdp(*piMedia);
        }
    }
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsNeedToUpdateQosStatus(
        IN QosStatus eCurrentStatus, IN QosStatus eNewStatus)
{
    return ((eCurrentStatus == QosStatus::IDLE && eNewStatus == QosStatus::AVAILABLE) ||
            (eCurrentStatus == QosStatus::AVAILABLE && eNewStatus == QosStatus::LOST) ||
            (eCurrentStatus == QosStatus::LOST && eNewStatus != QosStatus::LOST));
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsDefaultBearerAllowed(IN IMS_UINT32 eMediaType) const
{
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        // IR.92 2.4.3.1: A roaming UE is disallowed from sending media over the default bearer.
        return !m_objContext.GetService().IsRoaming() &&
                m_objContext.GetConfigurationProxy().GetBoolean(
                        ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        return m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVt::KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        return m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigRtt::KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL);
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsRemoteResourceReserved(IN ISession* piSession) const
{
    std::vector<IMS_UINT32> objMediaTypeList =
            MtcMediaUtil::GetMediaTypeListFromCallType(m_objContext.GetSession()->GetCallType());
    if (objMediaTypeList.empty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 eMediaType : objMediaTypeList)
    {
        QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
        if (!pStatusTable ||
                !pStatusTable->IsCurrentStatusEnabled(
                        GetSdpMediaType(eMediaType), SdpPrecondition::STATUS_REMOTE))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsLocalResourceReserved(
        IN ISession* piSession, IN IMS_BOOL bAtLeastOneReserved) const
{
    std::vector<IMS_UINT32> objMediaTypeList =
            MtcMediaUtil::GetMediaTypeListFromCallType(m_objContext.GetSession()->GetCallType());
    if (objMediaTypeList.empty())
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetService().IsWlanIpCanType())
    {
        IMS_TRACE_D("IsLocalResourceReserved : it is on Wi-Fi", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bResult = !bAtLeastOneReserved;
    for (IMS_UINT32 eMediaType : objMediaTypeList)
    {
        IMS_BOOL bReserved = IsLocalResourceReservedByMediaType(piSession, eMediaType);
        if ((!bAtLeastOneReserved && !bReserved) || (bAtLeastOneReserved && bReserved))
        {
            bResult = bReserved;
            break;
        }
    }

    return bResult;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsLocalResourceReservedByMediaType(
        IN ISession* piSession, IN IMS_UINT32 eMediaType) const
{
    if (m_objContext.GetService().IsWlanIpCanType())
    {
        IMS_TRACE_D("IsLocalResourceReservedByMediaType : it is on Wi-Fi", 0, 0, 0);
        return IMS_TRUE;
    }

    QosStatus eStatus = GetQosStatus(piSession, eMediaType);
    IMS_BOOL bDefaultBearerAllowed = IsDefaultBearerAllowed(eMediaType);

    IMS_TRACE_D("IsLocalResourceReservedByMediaType [%s] status[%s] use default bearer[%s]",
            MtcMediaStringUtils::ConvertContentType(eMediaType),
            QosStringUtils::ConvertQosStatus(eStatus), _TRACE_B_(bDefaultBearerAllowed));

    return (bDefaultBearerAllowed || eStatus == QosStatus::AVAILABLE);
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsLocalResourceReservedForVideoOrText(IN ISession* piSession)
{
    for (IMS_UINT32 eMediaType :
            MtcMediaUtil::GetMediaTypeListFromCallType(m_objContext.GetSession()->GetCallType()))
    {
        if (eMediaType == MEDIATYPE_AUDIO)
        {
            continue;
        }

        if (!IsLocalResourceReservedByMediaType(piSession, eMediaType))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsPreconditionSupported(IN ISession* piSession) const
{
    if (m_objContext.IsEstablished() &&
            m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_DISABLE_PRECONDITION_AFTER_CALL_ESTABLISHED_BOOL))
    {
        IMS_TRACE_I("IsPreconditionSupported : Not supported after established", 0, 0, 0);
        return IMS_FALSE;
    }

    const QosInfo* pInfo = GetQosInfo(piSession);
    IMS_BOOL bResult = (pInfo != IMS_NULL) ? pInfo->IsPreconditionSupported() : IMS_FALSE;
    IMS_TRACE_D("IsPreconditionSupported [%s]", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsPreconditionSupportedInLocal(IN IMS_UINT32 eMediaType) const
{
    IMS_BOOL bSupport = IMS_FALSE;
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        bSupport = m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        bSupport = m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        bSupport = m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL);
    }

    IMS_TRACE_D("IsPreconditionSupportedInLocal MediaType[%s][%s]",
            MtcMediaStringUtils::ConvertContentType(eMediaType), _TRACE_B_(bSupport), 0);

    return bSupport;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsConfirmedDialog(IN const ISession* piSession)
{
    if (piSession && piSession->GetState() >= ISession::STATE_ESTABLISHED)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsNeedToStartWaitAudioDedicatedBearerTimer(
        IN ISession* piSession, IN IMS_BOOL bSendingInitialInvite) const
{
    const QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pInfo->IsWaitAudioDedicatedBearerTimerStarted())
    {
        return IMS_FALSE;
    }

    if (!bSendingInitialInvite ||
            !m_objContext.GetConfigurationProxy().GetBoolean(ConfigVoice::
                            KEY_TRIGGER_DEDICATED_BEARER_WAIT_TIMER_BY_SENDING_INITIAL_INVITE_BOOL))
    {
        if (m_objContext.GetMediaManager().GetNegotiationState(piSession) !=
                NEGO_STATE::STATE_NEGOTIATED)
        {
            return IMS_FALSE;
        }
    }

    if (GetQosStatus(piSession, MEDIATYPE_AUDIO) != QosStatus::IDLE)
    {
        return IMS_FALSE;
    }

    if (IsDefaultBearerAllowed(MEDIATYPE_AUDIO))
    {
        return IMS_FALSE;
    }

    if (IsNotUsingDedicatedWaitTimerByRatCondition())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_UINT32 MtcPreconditionManager::SetLocalResourceAvailable(IN ISession* piSession) const
{
    IMS_UINT32 eEnabledMediaTypes = MEDIATYPE_NONE;
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    for (IMS_UINT32 eMediaType : MtcMediaUtil::GetMediaTypeListFromCallType(eCallType))
    {
        if (GetQosStatus(piSession, eMediaType) != QosStatus::AVAILABLE)
        {
            SetQosStatus(piSession, QosStatus::AVAILABLE, eMediaType);
            GetQosStatusTable(piSession)->UpdateLocalCurrentStatus(
                    GetSdpMediaType(eMediaType), IMS_TRUE);
            eEnabledMediaTypes |= eMediaType;
        }
    }

    IMS_TRACE_D("SetLocalResourceAvailable Enabled Media Types[%s]",
            MtcCallStringUtils::ConvertMediaType(eEnabledMediaTypes), 0, 0);
    return eEnabledMediaTypes;
}

PRIVATE
IMS_SINT32 MtcPreconditionManager::GetQosTime(IN QosTimerType eType) const
{
    switch (eType)
    {
        case QosTimerType::WAIT_AUDIO_DEDICATED_BEARER:
            return m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT);
        case QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER:
            return m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_QOS_ACQUISITION_AFTER_W2L_HANDOVER_WAIT_TIMER_MILLIS_INT);
        case QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE:
            return m_objContext.GetConfigurationProxy().GetInt(ConfigVoice::
                            KEY_WAIT_VIDEO_TEXT_QOS_AFTER_AUDIO_QOS_ACQUISITION_TIMER_MILLIS_INT);
        case QosTimerType::GUARD_AFTER_LOST:
            return m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_QOS_LOST_GUARD_TIMER_MILLIS_INT);
        default:  // QosTimerType::FORCE_AVAILABLE:
            return m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_QOS_FORCED_ACQUISITION_TIMER_MILLIS_INT);
    }
}

PRIVATE
IMS_SINT32 MtcPreconditionManager::GetSdpMediaType(IN IMS_UINT32 eMediaType)
{
    IMS_SINT32 eSdpMediaType = SdpMedia::TYPE_INVALID;

    switch (eMediaType)
    {
        case MEDIATYPE_AUDIO:
            eSdpMediaType = SdpMedia::TYPE_AUDIO;
            break;
        case MEDIATYPE_VIDEO:
            eSdpMediaType = SdpMedia::TYPE_VIDEO;
            break;
        case MEDIATYPE_TEXT:
            eSdpMediaType = SdpMedia::TYPE_TEXT;
            break;
        default:
            break;
    }

    return eSdpMediaType;
}

PRIVATE
ISession* MtcPreconditionManager::GetISessionWithTimer(IN const QosTimer* pTimer) const
{
    for (IMS_UINT32 index = 0; index < m_objQosInfos.GetSize(); index++)
    {
        QosInfo* pInfo = m_objQosInfos.GetValueAt(index);
        if (pInfo == IMS_NULL)
        {
            continue;
        }

        if (&(pInfo->GetTimer()) == pTimer)
        {
            return m_objQosInfos.GetKeyAt(index);
        }
    }

    return IMS_NULL;
}

PRIVATE
IMediaDescriptor* MtcPreconditionManager::GetMediaDescriptor(IN const IMedia* piMedia)
{
    if (piMedia == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
    {
        return piMedia->GetProposal()->GetMediaDescriptor();
    }
    else
    {
        return piMedia->GetMediaDescriptor();
    }
}

PRIVATE
const SdpMedia* MtcPreconditionManager::GetSdpMedia(IN const IMedia* piMedia, IN IMS_BOOL bRemote)
{
    const IMediaDescriptor* piMediaDescriptor = GetMediaDescriptor(piMedia);
    if (piMediaDescriptor == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!bRemote)
    {
        if (piMedia->GetState() == IMedia::STATE_DELETED)
        {
            return IMS_NULL;
        }
        else
        {
            return piMediaDescriptor->GetMediaDescriptionExAsLocal();
        }
    }

    return piMediaDescriptor->GetMediaDescriptionEx();
}

PRIVATE
QosLossPolicy MtcPreconditionManager::GetQosLossPolicy(IN IMS_UINT32 eMediaType) const
{
    IMS_SINT32 nPolicy = -1;
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                ConfigVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                ConfigRtt::KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT);
    }

    switch (nPolicy)
    {
        case ConfigVoice::QOS_DEACTIVATION_POLICY_TERMINATE_CALL:
            return QosLossPolicy::RELEASE;
        case ConfigVoice::QOS_DEACTIVATION_POLICY_MAINTAIN_CALL:
            return QosLossPolicy::MAINTAIN;
        case ConfigVoice::QOS_DEACTIVATION_POLICY_MODIFY_CALL:
            return QosLossPolicy::MODIFY;
        default:
            break;
    }

    return QosLossPolicy::MAINTAIN;
}

PRIVATE
QosLossPolicy MtcPreconditionManager::GetActionForQosLoss(IN ISession* piSession) const
{
    QosLossPolicy eAction = QosLossPolicy::MAINTAIN;
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    for (IMS_UINT32 eMediaType : MtcMediaUtil::GetMediaTypeListFromCallType(eCallType))
    {
        if (!IsLocalResourceReservedByMediaType(piSession, eMediaType))
        {
            QosLossPolicy ePartialAction =
                    (!IsConfirmedDialog(piSession) && eMediaType == MEDIATYPE_AUDIO)
                    ? QosLossPolicy::RELEASE
                    : GetQosLossPolicy(eMediaType);
            if (eAction < ePartialAction)
            {
                eAction = ePartialAction;
            }
        }
    }

    IMS_TRACE_D("GetActionForQosLoss The next action is [%s]",
            QosStringUtils::ConvertQosLossPolicy(eAction), 0, 0);
    return eAction;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsConfirmationRequired(IN const ISession& objISession) const
{
    if (IsConfirmedDialog(&objISession))
    {
        // Check if it's sending a first response for re-INVITE.
        if (!m_objContext.GetUpdatingInfo().IsModifier() &&
                objISession.GetPreviousResponse(IMessage::SESSION_UPDATE) == IMS_NULL)
        {
            // TODO: check after sending 100 Tyring.
            return IMS_TRUE;
        }
        else
        {
            return IMS_FALSE;
        }
    }

    return (m_objContext.GetCallInfo().ePeerType == PeerType::MT);
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsNotUsingDedicatedWaitTimerByRatCondition() const
{
    IMS_TRACE_D("IsNotUsingDedicatedWaitTimerByRatCondition pre[%s] curr[%s]",
            MtcCallStringUtils::ConvertRatType(m_ePreviousRatType),
            MtcCallStringUtils::ConvertRatType(m_eCurrentRatType), 0);
    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_NR) &&
            m_eCurrentRatType == INetworkWatcher::RADIOTECH_TYPE_NR)
    {
        return IMS_TRUE;
    }

    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_EPS_FALLBACK) &&
            IsEpsFallback())
    {
        return IMS_TRUE;
    }

    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_EPS_ONLY_ATTACH) &&
            m_objContext.GetService().IsEpsOnlyAttach())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsVideoOrTextIncluded(IN CallType eCallType) const
{
    return (eCallType == CallType::VT || eCallType == CallType::RTT ||
            eCallType == CallType::VIDEO_RTT);
}
