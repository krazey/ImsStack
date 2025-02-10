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
#include "ISipHeader.h"
#include "ISubscriberConfig.h"
#include "ImsEventDef.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcSession.h"
#include "call/UpdatingInfo.h"
#include "call/extension/MtcExtensionSet.h"
#include "configuration/MtcConfigurationProxy.h"
#include "media/IMedia.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "precondition/MtcPreconditionManager.h"
#include "precondition/QosStringDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"
#include <vector>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcPreconditionManager::MtcPreconditionManager(IN IMtcCallContext& objContext) :
        m_objQosInfos(ImsMap<ISession*, QosInfo*>()),
        m_pListener(IMS_NULL),
        m_objContext(objContext),
        m_pSdpPreconditionHelper(new SdpPreconditionHelper()),
        m_bOnWlan(objContext.GetService().IsWlanIpCanType())
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

    IMS_TRACE_D(
            "IsPreconditionSupportedInLocal CallType[%d][%s]", eCallType, _TRACE_B_(bSupport), 0);
    return bSupport;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsDedicatedBearerAllocated(
        IN ISession* piSession, IN IMS_UINT32 eMediaType) const
{
    QosStatus eStatus = GetQosStatus(piSession, eMediaType);
    IMS_TRACE_D("IsDedicatedBearerAllocated [%d][%s]", eMediaType, PS_QosStatus(eStatus), 0);

    return eStatus == QosStatus::AVAILABLE;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsCheckingResourcesRequiredToAlertUser() const
{
    IMS_TRACE_D("IsCheckingResourcesRequiredToAlertUser", 0, 0, 0);
    return !IsDefaultBearerAllowed(MEDIATYPE_AUDIO);
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsAvailableToAlertUser(IN ISession* piSession) const
{
    IMS_TRACE_D("IsAvailableToAlertUser", 0, 0, 0);
    if (piSession == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (GetQosTimer(piSession)->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE))
    {
        return IMS_FALSE;
    }

    IMS_BOOL bLocalReserved = IsLocalResourceReserved(piSession, !IsConfirmedDialog(piSession));
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

    IMS_BOOL bResult = IMS_FALSE;
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    for (IMS_UINT32 eMediaType : MtcMediaUtil::GetMediaTypeListFromCallType(eCallType))
    {
        if (!pStatusTable->IsLocalResourceConfirmed(GetSdpMediaType(eMediaType)) &&
                IsLocalResourceReservedByMediaType(piSession, eMediaType))
        {
            bResult = IMS_TRUE;
            break;
        }
    }

    IMS_TRACE_D("IsLocalResourceConfirmationRequired (%s)", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsAvailableToSendLocalResourceConfirmation(
        IN ISession* piSession) const
{
    IMS_BOOL bResult;
    if (GetQosTimer(piSession)->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE))
    {
        bResult = IMS_FALSE;
    }
    else
    {
        bResult = IsLocalResourceReserved(piSession, !IsConfirmedDialog(piSession));
    }

    // TODO: This log trace will be removed after verification.
    IMS_TRACE_D("IsAvailableToSendLocalResourceConfirmation (%s)", _TRACE_B_(bResult), 0, 0);
    return bResult;
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
        IMedia* piMedia = lstMedias.GetAt(index);
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

PUBLIC VIRTUAL void MtcPreconditionManager::HandleQosOnIpcanChanged()
{
    IMS_BOOL bPreviousOnWlan = m_bOnWlan;
    SetOnWlan(m_objContext.GetService().IsWlanIpCanType());
    if (bPreviousOnWlan == m_bOnWlan)  // cover NR to LTE case.
    {
        return;
    }

    IMS_TRACE_D("HandleQosOnIpcanChanged on WLAN [%s]", _TRACE_B_(m_bOnWlan), 0, 0);
    for (IMS_UINT32 index = 0; index < m_objQosInfos.GetSize(); index++)
    {
        ISession* piSession = m_objQosInfos.GetKeyAt(index);

        if (!m_bOnWlan)
        {
            if (!IsLocalResourceReserved(piSession, IMS_FALSE))
            {
                StopQosTimer(piSession, QosTimerType::WAIT_AUDIO_AVAILABLE);
                StartQosTimer(piSession, QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER);
            }
        }
        else
        {
            CallType eCallType = m_objContext.GetSession()->GetCallType();
            NotifyQosStatusToListener(
                    piSession, IMS_TRUE, MtcMediaUtil::GetMediaTypesFromCallType(eCallType));
        }
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnSdpReceived(
        IN ISession* piSession, IN IMessage* piMessage)
{
    IMS_TRACE_D("OnSdpReceived", 0, 0, 0);
    UpdateQosAttributesFromRemoteSdp(piSession);

    if (IsNeedToStartWaitAudioAvailableTimer(piSession, piMessage))
    {
        StartQosTimer(piSession, QosTimerType::WAIT_AUDIO_AVAILABLE);
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
    if (bCheckSdp && !m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        return;
    }

    IMS_BOOL bHasSupportedHeader = m_objContext.GetMessageUtils().HasValue(
            piMessage, MessageUtil::STR_PRECONDITION, ISipHeader::SUPPORTED);
    IMS_BOOL bHasRequireHeader = m_objContext.GetMessageUtils().HasValue(
            piMessage, MessageUtil::STR_PRECONDITION, ISipHeader::REQUIRE);

    IMS_BOOL bRemoteSupported = (bHasSupportedHeader || bHasRequireHeader) &&
            (!bCheckSdp || m_pSdpPreconditionHelper->IsPreconditionIncludedInSdp(piSession));

    UpdateSupportingPrecondition(piSession, bRemoteSupported);
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnCallEstablished(IN ISession* piSession)
{
    IMS_TRACE_D("OnCallEstablished", 0, 0, 0);

    if (!IsLocalResourceReserved(piSession, IMS_FALSE))
    {
        StartQosTimer(piSession, QosTimerType::GUARD_AVAILABLE);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnCallModified(IN ISession* piSession)
{
    IMS_TRACE_D("OnCallModified", 0, 0, 0);

    IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT);
    if (nPolicy == ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE)
    {
        if (!IsLocalResourceReserved(piSession, IMS_FALSE))
        {
            StartQosTimer(piSession, QosTimerType::GUARD_AVAILABLE);
        }
    }
    else if (nPolicy == ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING)
    {
        // To change Local status from QosStatus::LOST to QosStatus::IDLE for removed medias
        InitializeStatusForLostQos(piSession, IMS_TRUE);

        // To change remote status to QosStatus::IDLE for removed medias
        QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
        if (pStatusTable != IMS_NULL)
        {
            pStatusTable->RemoveUnusedRecords(MtcMediaUtil::GetMediaTypesFromCallType(
                    m_objContext.GetSession()->GetCallType()));
        }
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnQosStatusChanged(
        IN ISession* piSession, IN QosStatus eStatus, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("OnQosStatusChanged media type[%d][%s]", eMediaType, PS_QosStatus(eStatus), 0);

    SetOnWlan(m_objContext.GetService().IsWlanIpCanType());
    QosStatus eCurrStatus = GetQosStatus(piSession, eMediaType);
    if (eCurrStatus == eStatus)
    {
        IMS_TRACE_D("OnQosStatusChanged no update for status.", 0, 0, 0);
        return;
    }

    SetQosStatus(piSession, eStatus, eMediaType);
    HandleQosTimer(piSession, eCurrStatus, eStatus, eMediaType);

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }
    pStatusTable->UpdateLocalCurrentStatus(
            GetSdpMediaType(eMediaType), IsLocalResourceReservedByMediaType(piSession, eMediaType));

    if ((eCurrStatus == QosStatus::IDLE && eStatus == QosStatus::AVAILABLE) &&
            (!GetQosTimer(piSession)->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE)))
    {
        NotifyQosStatusToListener(piSession, IMS_TRUE, eMediaType);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnTimerExpired(
        IN QosTimer* pTimer, IN QosTimerType eType)
{
    IMS_TRACE_D("OnTimerExpired [%s]", PS_QosTimerType(eType), 0, 0);
    if (eType == QosTimerType::FORCE_AVAILABLE)
    {
        return OnForceAvailableTimerExpired(pTimer);
    }
    else if (eType == QosTimerType::GUARD_AVAILABLE)
    {
        return OnGuardAvailableTimerExpired(pTimer);
    }

    HandleReservationFailureByTimerExpiration(pTimer);
}

PROTECTED
QosInfo* MtcPreconditionManager::GetQosInfo(IN ISession* piSession) const
{
    IMS_SLONG nIndex = m_objQosInfos.GetIndexOfKey(piSession);
    return (nIndex >= 0) ? m_objQosInfos.GetValueAt(nIndex) : IMS_NULL;
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
void MtcPreconditionManager::SetQosStatus(
        IN ISession* piSession, QosStatus eStatus, IN IMS_UINT32 eMediaType) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
    if (pInfo == IMS_NULL)
    {
        return;
    }

    if (!IsNeedToUpdateQosStatus(GetQosStatus(piSession, eMediaType), eStatus))
    {
        IMS_TRACE_D("SetQosStatus nothing to update", 0, 0, 0);
        return;
    }

    // To change Local status from QosStatus::LOST to QosStatus::IDLE for removed medias
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    if (eStatus == QosStatus::LOST &&
            !(MtcMediaUtil::GetMediaTypesFromCallType(eCallType) & eMediaType))
    {
        eStatus = QosStatus::IDLE;
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
}

PRIVATE
QosStatus MtcPreconditionManager::GetQosStatus(
        IN ISession* piSession, IN IMS_UINT32 eMediaType) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
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
    if (!IsPreconditionSupportedInLocal())
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
            (eType == QosTimerType::WAIT_AUDIO_AVAILABLE ||
                    eType == QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER))
    {
        eType = QosTimerType::FORCE_AVAILABLE;
        nDuration = GetQosTime(eType);
    }

    pTimer->StartQosTimer(eType, nDuration);
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
    std::vector<QosTimerType> objTimerTypes{QosTimerType::WAIT_AUDIO_AVAILABLE,
            QosTimerType::GUARD_AVAILABLE, QosTimerType::GUARD_AFTER_LOST,
            QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER, QosTimerType::FORCE_AVAILABLE};

    for (QosTimerType eType : objTimerTypes)
    {
        StopQosTimer(piSession, eType);
    }
}

PRIVATE
void MtcPreconditionManager::OnForceAvailableTimerExpired(IN QosTimer* pTimer)
{
    ISession* piSession = GetISessionWithTimer(pTimer);
    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("OnForceAvailableTimerExpired", 0, 0, 0);
    NotifyQosStatusToListener(piSession, IMS_TRUE, SetLocalResourceAvailable(piSession));
}

PRIVATE
void MtcPreconditionManager::OnGuardAvailableTimerExpired(IN QosTimer* pTimer)
{
    ISession* piSession = GetISessionWithTimer(pTimer);
    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("OnGuardAvailableTimerExpired", 0, 0, 0);
    if (IsConfirmedDialog(piSession))
    {
        return HandleReservationFailureByTimerExpiration(pTimer);
    }

    IMS_UINT32 eReservedMediaTypes = MEDIATYPE_NONE;
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    for (IMS_UINT32 eMediaType : MtcMediaUtil::GetMediaTypeListFromCallType(eCallType))
    {
        if (GetQosStatus(piSession, eMediaType) == QosStatus::AVAILABLE)
        {
            eReservedMediaTypes |= eMediaType;
        }
    }

    NotifyQosStatusToListener(piSession, IMS_TRUE, eReservedMediaTypes);
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
    InitializeStatusForLostQos(piSession, IMS_FALSE);
}

PRIVATE
void MtcPreconditionManager::InitializeStatusForLostQos(
        IN ISession* piSession, IN IMS_BOOL bRemovedMedia) const
{
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    std::vector<IMS_UINT32> objMediaTypeList = bRemovedMedia
            ? MtcMediaUtil::GetUnusedMediaTypeListFromCallType(eCallType)
            : MtcMediaUtil::GetMediaTypeListFromCallType(eCallType);
    for (IMS_UINT32 eMediaType : objMediaTypeList)
    {
        if (GetQosStatus(piSession, eMediaType) == QosStatus::LOST)
        {
            IMS_TRACE_D("InitializeStatusForLostQos [%d]", eMediaType, 0, 0);
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
    IMS_BOOL bLocalReserved = IsLocalResourceReservedByMediaType(piSession, eMediaType);

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
    IMS_TRACE_D(
            "HandleQosTimer [%s]->[%s]", PS_QosStatus(eCurrentStatus), PS_QosStatus(eNewStatus), 0);

    if (IsLocalResourceReserved(piSession, IMS_FALSE))
    {
        StopAllQosTimer(piSession);
    }
    else
    {
        if (eCurrentStatus == QosStatus::AVAILABLE && eNewStatus == QosStatus::LOST)
        {
            StartQosTimer(piSession, QosTimerType::GUARD_AFTER_LOST);
        }

        if (eNewStatus == QosStatus::AVAILABLE && eMediaType == MEDIATYPE_AUDIO)
        {
            StopQosTimer(piSession, QosTimerType::WAIT_AUDIO_AVAILABLE);
        }

        if (IsLocalResourceReserved(piSession, IMS_TRUE))
        {
            QosTimer* pTimer = GetQosTimer(piSession);
            if (pTimer && !pTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST))
            {
                StartQosTimer(piSession, QosTimerType::GUARD_AVAILABLE);
            }
        }
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
        IMedia* piMedia = lstMedias.GetAt(index);
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
        IMedia* piMedia = lstMedias.GetAt(index);
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
        return !IsRoaming() &&
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

    QosTimer* pTimer = GetQosTimer(piSession);
    if (m_objContext.GetService().IsWlanIpCanType() ||
            (pTimer && pTimer->IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER)))
    {
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

    IMS_TRACE_D("IsLocalResourceReserved [%s]", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsLocalResourceReservedByMediaType(
        IN ISession* piSession, IN IMS_UINT32 eMediaType) const
{
    QosTimer* pTimer = GetQosTimer(piSession);
    if (m_objContext.GetService().IsWlanIpCanType() ||
            (pTimer && pTimer->IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER)))
    {
        return IMS_TRUE;
    }

    QosStatus eStatus = GetQosStatus(piSession, eMediaType);
    IMS_BOOL bDefaultBearerAllowed = IsDefaultBearerAllowed(eMediaType);

    IMS_TRACE_D("IsLocalResourceReservedByMediaType [%d] status[%s] use default bearer[%s]",
            eMediaType, PS_QosStatus(eStatus), _TRACE_B_(bDefaultBearerAllowed));

    return (bDefaultBearerAllowed || eStatus == QosStatus::AVAILABLE);
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsPreconditionSupported(IN ISession* piSession) const
{
    QosInfo* pInfo = GetQosInfo(piSession);
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

    IMS_TRACE_D(
            "IsPreconditionSupportedInLocal MediaType[%d][%s]", eMediaType, _TRACE_B_(bSupport), 0);

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
IMS_BOOL MtcPreconditionManager::IsNeedToStartWaitAudioAvailableTimer(
        IN ISession* piSession, IN IMessage* piMessage) const
{
    if (IsConfirmedDialog(piSession))
    {
        return IMS_FALSE;
    }

    if (IsLocalResourceReservedByMediaType(piSession, MEDIATYPE_AUDIO))
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piMessage->GetMethod();
    IMS_SINT32 nType = piMessage->GetMessage()->GetType();
    if (nType == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::PRACK))
        {
            return IMS_TRUE;
        }
    }
    else if (nType == ISipMessage::TYPE_RESPONSE)
    {
        if (objMethod.Equals(SipMethod::INVITE) && piMessage->GetMessage()->IsMessageRpr())
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
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

    IMS_TRACE_D("SetLocalResourceAvailable Enabled Media Types[%d]", eEnabledMediaTypes, 0, 0);
    return eEnabledMediaTypes;
}

PRIVATE
IMS_SINT32 MtcPreconditionManager::GetQosTime(IN QosTimerType eType) const
{
    LOCAL const IMS_SINT32 TIME_GUARD_AVAILABLE = 1000;
    LOCAL const IMS_SINT32 TIME_GUARD_AFTER_LOST = 1000;
    LOCAL const IMS_SINT32 TIME_FORCE_AVAILABLE = 500;

    switch (eType)
    {
        case QosTimerType::GUARD_AVAILABLE:
            return TIME_GUARD_AVAILABLE;
        case QosTimerType::GUARD_AFTER_LOST:
            return TIME_GUARD_AFTER_LOST;
        case QosTimerType::FORCE_AVAILABLE:
            return TIME_FORCE_AVAILABLE;
        default:  // WAIT_AUDIO_AVAILABLE, WAIT_AVAILABLE_AFTER_HANDOVER
            return m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT);
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
IMediaDescriptor* MtcPreconditionManager::GetMediaDescriptor(IN IMedia* piMedia)
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
const SdpMedia* MtcPreconditionManager::GetSdpMedia(IN IMedia* piMedia, IN IMS_BOOL bRemote)
{
    IMediaDescriptor* piMediaDescriptor = GetMediaDescriptor(piMedia);
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
            QosLossPolicy ePartialAction = GetQosLossPolicy(eMediaType);
            if (eAction < ePartialAction)
            {
                eAction = ePartialAction;
            }
        }
    }

    IMS_TRACE_D("GetActionForQosLoss The next action is %s", PS_QosLossPolicy(eAction), 0, 0);
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
IMS_BOOL MtcPreconditionManager::IsRoaming() const
{
    return m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_ROAMING_STATE) ==
            IMS_ROAMING_STATE_ON;
}
