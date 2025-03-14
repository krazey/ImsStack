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

#include "CallReasonInfo.h"
#include "ICoreService.h"
#include "ISipHeader.h"
#include "IMediaManager.h"
#include "MediaDef.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/MtcCallStringUtils.h"
#include "call/IMtcSession.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MtcSupplementaryService.h"
#include "media/IMediaQosEventListener.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaManager.h"
#include "media/MtcMediaStringUtils.h"
#include "media/MtcMediaUtil.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "utility/IMessageUtils.h"
#include <vector>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcMediaManager::MtcMediaManager(
        IN IMtcCallContext& objContext, IN IMediaManager& objMediaManager) :
        m_objMediaManager(objMediaManager),
        m_pMediaReportListener(IMS_NULL),
        m_pQosListener(IMS_NULL),
        m_pProfileManager(new MtcMediaProfileManager()),
        m_objContext(objContext),
        m_pMediaInfo(new MediaInfo()),
        m_pOldMediaInfo(new MediaInfo()),
        m_bLocalTone(IMS_FALSE),
        m_bAudioInactive(IMS_FALSE),
        m_piMediaSession(IMS_NULL)
{
    IMS_TRACE_D("+MtcMediaManager Callkey[%d]", m_objContext.GetCallKey(), 0, 0);
}

PUBLIC VIRTUAL MtcMediaManager::~MtcMediaManager()
{
    IMS_TRACE_D("~MtcMediaManager Callkey[%d]", m_objContext.GetCallKey(), 0, 0);

    m_pMediaReportListener = IMS_NULL;
    m_pQosListener = IMS_NULL;

    delete m_pProfileManager;

    if (m_pMediaInfo)
    {
        delete m_pMediaInfo;
    }

    if (m_pOldMediaInfo)
    {
        delete m_pOldMediaInfo;
    }

    m_objMediaManager.DestroySession(m_piMediaSession);
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_Notify(IN IMS_UINT32 eReportType,
        IN MEDIA_CONTENT_TYPE eMediaType /*= MEDIA_TYPE_INVALID*/,
        IN MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType /*= MEDIA_PROTOCOL_ANY*/)
{
    IMS_TRACE_D("MediaSession_Notify : Report[%s] Media[%s]",
            MtcMediaStringUtils::ConvertReportType(eReportType),
            MtcMediaStringUtils::ConvertContentType(eMediaType), 0);
    IMS_UINT32 eReportedMediaType = MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaType);

    switch (eReportType)
    {
        case REPORT_DATA_RECEIVE_FAILED:
            if (eReportedMediaType == MEDIATYPE_AUDIO)
            {
                IMS_TRACE_D("MediaSession_Notify : audio blocked", 0, 0, 0);
                m_bAudioInactive = IMS_TRUE;
            }
            m_pMediaReportListener->OnReceivingMediaDataFailed(
                    eReportedMediaType, eMediaProtocolType);
            break;
        case REPORT_DATA_RECEIVE_STARTED:
            HandleReceivingMediaDataStarted(eReportedMediaType);
            m_pMediaReportListener->OnReceivingMediaDataStarted(
                    eReportedMediaType, eMediaProtocolType);
            break;
        case REPORT_VIDEO_LOWEST_BITRATE:
            m_pMediaReportListener->OnVideoLowestBitRate();
            break;
        case REPORT_NW_TONE_RTP_RECEIVE_STARTED:
            HandleReceivingNetworkTone(IMS_TRUE);
            break;
        case REPORT_NW_TONE_RTP_RECEIVE_FAILED:
            HandleReceivingNetworkTone(IMS_FALSE);
            break;
        case REPORT_MEDIA_DETACH:
            m_pMediaReportListener->OnMediaFailed(CallReasonInfo(CODE_MEDIA_UNSPECIFIED));
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_NotifyFailures(IN IMS_UINT32 eReportType,
        IN IMS_SINT32 eError, IN MEDIA_CONTENT_TYPE eMediaType /*= MEDIA_TYPE_INVALID*/)
{
    IMS_TRACE_D("MediaSession_NotifyFailures : Report[%s] Error[%s] Media[%s]",
            MtcMediaStringUtils::ConvertReportType(eReportType),
            MtcMediaStringUtils::ConvertErrorType(eError),
            MtcMediaStringUtils::ConvertContentType(eMediaType));

    if (eError != RtpError::NO_ERROR)
    {
        m_pMediaReportListener->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
    }
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_NotifyQos(IN IMS_UINTP nNegoId,
        IN IMS_BOOL bSuccess, IN MEDIA_CONTENT_TYPE eMediaType /*= MEDIA_TYPE_INVALID*/)
{
    IMS_TRACE_D("MediaSession_NotifyQos : NegoId[%" PFLS_x "] Result[%d] Media[%s]", nNegoId,
            bSuccess, MtcMediaStringUtils::ConvertContentType(eMediaType));

    if (m_pQosListener != IMS_NULL)
    {
        m_pQosListener->OnQosStatusChanged(m_pProfileManager->GetSessionWithNegoId(nNegoId),
                (bSuccess) ? QosStatus::AVAILABLE : QosStatus::LOST,
                MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaType));
    }
}

PUBLIC VIRTUAL void MtcMediaManager::SetMediaReportEventListener(
        IN IMediaReportEventListener* pListener)
{
    m_pMediaReportListener = pListener;
}

PUBLIC VIRTUAL void MtcMediaManager::SetQosListener(IN IMediaQosEventListener* pListener)
{
    m_pQosListener = pListener;
}

PUBLIC VIRTUAL void MtcMediaManager::SetMediaInfo(IN const MediaInfo& objInfo)
{
    *m_pOldMediaInfo = *m_pMediaInfo;
    *m_pMediaInfo = objInfo;
}

PUBLIC VIRTUAL void MtcMediaManager::UpdateMediaDirection(
        IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir)
{
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        m_pOldMediaInfo->eAudioDirection = m_pMediaInfo->eAudioDirection;
        m_pMediaInfo->eAudioDirection = eDir;
        IMS_TRACE_D("UpdateMediaDirection : audio [%s]->[%s]",
                MtcMediaStringUtils::ConvertDirection(m_pOldMediaInfo->eAudioDirection),
                MtcMediaStringUtils::ConvertDirection(m_pMediaInfo->eAudioDirection), 0);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        m_pOldMediaInfo->eVideoDirection = m_pMediaInfo->eVideoDirection;
        m_pMediaInfo->eVideoDirection = eDir;
        IMS_TRACE_D("UpdateMediaDirection : video [%s]->[%s]",
                MtcMediaStringUtils::ConvertDirection(m_pOldMediaInfo->eVideoDirection),
                MtcMediaStringUtils::ConvertDirection(m_pMediaInfo->eVideoDirection), 0);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        m_pOldMediaInfo->eTextDirection = m_pMediaInfo->eTextDirection;
        m_pMediaInfo->eTextDirection = eDir;
        IMS_TRACE_D("UpdateMediaDirection : text [%s]->[%s]",
                MtcMediaStringUtils::ConvertDirection(m_pOldMediaInfo->eTextDirection),
                MtcMediaStringUtils::ConvertDirection(m_pMediaInfo->eTextDirection), 0);
    }
}

PUBLIC VIRTUAL const MediaInfo& MtcMediaManager::GetMediaInfo() const
{
    IMS_TRACE_D("GetMediaInfo", 0, 0, 0);
    return *m_pMediaInfo;
}

PUBLIC VIRTUAL void MtcMediaManager::RestoreMediaInfo()
{
    IMS_TRACE_D("RestoreMediaInfo", 0, 0, 0);
    MediaInfo objOldMediaInfo(*m_pOldMediaInfo);
    SetMediaInfo(objOldMediaInfo);
}

PUBLIC VIRTUAL void MtcMediaManager::CreateMediaSession()
{
    IMS_TRACE_D("CreateMediaSession", 0, 0, 0);

    ServiceType eServiceType = m_objContext.GetService().GetServiceType();
    IMS_SINT32 eRadioType = PhoneInfoService::GetPhoneInfoService()
                                    ->GetNetworkWatcher(m_objContext.GetSlotId())
                                    ->GetNetworkType();

    m_piMediaSession = m_objMediaManager.CreateSession(
            MtcMediaUtil::GetMediaNetworkType(&m_objContext.GetService(), eRadioType),
            MtcMediaUtil::GetMediaServiceType(eServiceType),
            m_objContext.GetService().GetICoreService(), m_objContext.GetCallKey());

    if (m_piMediaSession == IMS_NULL)
    {
        IMS_TRACE_D("CreateMediaSession : Failed", 0, 0, 0);
        return;
    }

    m_piMediaSession->SetMtcListener(this);
}

PUBLIC VIRTUAL void MtcMediaManager::DestroyMediaSession()
{
    IMS_TRACE_D("DestroyMediaSession", 0, 0, 0);

    m_objMediaManager.DestroySession(m_piMediaSession);
    m_piMediaSession = IMS_NULL;
}

PUBLIC VIRTUAL void MtcMediaManager::CreateMediaProfile(
        IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOrigin)
{
    IMS_TRACE_D("CreateMediaProfile", 0, 0, 0);
    MEDIA_CONTENT_TYPE eMediaContents =
            MtcMediaUtil::GetMediaContentsFromCallType(m_objContext.GetSession()->GetCallType());
    m_pProfileManager->CreateMediaProfile(
            piSession, bForked, bOrigin, eMediaContents, m_piMediaSession);
}

PUBLIC VIRTUAL void MtcMediaManager::DestroyMediaProfile(IN ISession* piSession)
{
    IMS_TRACE_D("DestroyMediaProfile", 0, 0, 0);
    m_pProfileManager->DestroyMediaProfile(piSession, m_piMediaSession);
}

PUBLIC
void MtcMediaManager::DestroyAllMediaProfiles()
{
    IMS_TRACE_D("DestroyAllMediaProfiles", 0, 0, 0);
    m_pProfileManager->DestroyAllMediaProfiles(m_piMediaSession);
}

PUBLIC
void MtcMediaManager::SetLocalTone(IN IMS_BOOL bLocalTone)
{
    m_bLocalTone = bLocalTone;
}

PUBLIC VIRTUAL IMS_BOOL MtcMediaManager::IsLocalTone()
{
    return m_bLocalTone;
}

PUBLIC VIRTUAL IMS_RESULT MtcMediaManager::FormSdp(IN ISession* piSession, IN CallType eCallType,
        IN IMS_BOOL bAnswerForOfferlessReInvite /* = IMS_FALSE*/)
{
    if (GetNegotiationState(piSession) == NegotiationState::STATE_OFFER_SENT)
    {
        IMS_TRACE_D("FormSdp : Failed to form SDP because nego state is offer-sent.", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_D("FormSdp : CallType[%s]", MtcCallStringUtils::ConvertCallType(eCallType), 0, 0);
    MEDIA_CONTENT_TYPE eContents = MtcMediaUtil::GetMediaContentsFromCallType(eCallType);
    IMS_UINTP nNegoId = GetMediaNegoId(piSession);

    IMS_BOOL bResult = m_piMediaSession->FormSdp(nNegoId, piSession, eContents,
            m_pMediaInfo->eAudioDirection, m_pMediaInfo->eVideoDirection,
            m_pMediaInfo->eTextDirection, bAnswerForOfferlessReInvite);

    if (!bResult)
    {
        RestoreMediaInfo();
    }
    else if (GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED)
    {
        m_piMediaSession->RequestQos(nNegoId, eContents);
    }

    return (bResult) ? IMS_SUCCESS : IMS_FAILURE;
}

PUBLIC VIRTUAL NegotiationResult MtcMediaManager::NegotiateSdp(IN ISession* piSession)
{
    MediaNego::MediaNegoResult eErrorReason = MediaNego::MediaNegoResult::NO_ERROR;
    IMS_SINT32 eAudioDirection = DIRECTION_INVALID;
    IMS_SINT32 eVideoDirection = DIRECTION_INVALID;
    IMS_SINT32 eTextDirection = DIRECTION_INVALID;

    IMS_UINTP nNegoId = GetMediaNegoId(piSession);

    m_piMediaSession->NegotiateSdp(
            nNegoId, piSession, &eAudioDirection, &eVideoDirection, &eTextDirection, eErrorReason);

    IMS_TRACE_D("NegotiateSdp : [%s]", MtcMediaStringUtils::ConvertNegoType(eErrorReason), 0, 0);

    if (eErrorReason != NegotiationResult::NO_ERROR)
    {
        return eErrorReason;
    }

    MediaInfo objInfo(eAudioDirection, eVideoDirection, eTextDirection,
            GetNegotiatedQuality(piSession, MEDIATYPE_AUDIO),
            GetNegotiatedQuality(piSession, MEDIATYPE_VIDEO),
            MtcMediaUtil::GetGttModeFromTextQuality(
                    GetNegotiatedQuality(piSession, MEDIATYPE_TEXT)));
    SetMediaInfo(objInfo);

    if (GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED)
    {
        m_piMediaSession->RequestQos(nNegoId, m_piMediaSession->GetNegotiatedMediaType(nNegoId));
    }

    return eErrorReason;
}

PUBLIC VIRTUAL void MtcMediaManager::RestoreSdp(IN ISession* piSession)
{
    IMS_TRACE_D("RestoreSdp", 0, 0, 0);
    RestoreMediaInfo();
    FinalizeSdp(piSession);
    if (piSession->GetState() == ISession::STATE_ESTABLISHED)
    {
        piSession->Restore();
    }
}

PUBLIC
void MtcMediaManager::FinalizeSdp(IN ISession* piSession)
{
    IMS_TRACE_D("FinalizeSdp", 0, 0, 0);
    m_piMediaSession->FinalizeSdp(GetMediaNegoId(piSession), piSession);
}

PUBLIC VIRTUAL void MtcMediaManager::UpdatePemType(IN ISession* piSession, IN IMessage* piMessage)
{
    PemType ePemType = PemType::NONE;

    AString strPemHeader =
            m_objContext.GetMessageUtils().GetHeader(piMessage, ISipHeader::P_EARLY_MEDIA);
    if (strPemHeader.Contains("sendrecv"))
    {
        ePemType = PemType::SENDRECV;
    }
    else if (strPemHeader.Contains("sendonly"))
    {
        ePemType = PemType::SENDONLY;
    }
    else if (strPemHeader.Contains("recvonly"))
    {
        ePemType = PemType::RECVONLY;
    }
    else if (strPemHeader.Contains("inactive"))
    {
        ePemType = PemType::INACTIVE;
    }
    // When UE receives P-Early-Media header with "gated", PemType will be keep the value.

    if (ePemType != PemType::NONE ||
            m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL))
    {
        m_pProfileManager->SetPemType(piSession, ePemType);
    }
    else
    {
        IMS_TRACE_D("UpdatePemType : no update for P-Early-Media value.", 0, 0, 0);
    }

    IMS_TRACE_D("UpdatePemType : [%s]", MtcMediaStringUtils::ConvertPemType(GetPemType(piSession)),
            0, 0);
}

PUBLIC VIRTUAL void MtcMediaManager::Run(
        IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly)
{
    if (bEarly && m_objContext.GetCallInfo().ePeerType == PeerType::MO)
    {
        UpdateLocalTone(piSession, piMessage);
    }

    if (GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
    {
        IMS_TRACE_D("Run : SDP is not negotiated, Don't run the media.", 0, 0, 0);
        return;
    }

    if (!bEarly)
    {
        SetConfirmedSession(piSession);
        FinalizeSdp(piSession);
    }

    if (!IsNecessaryToRunMedia(piSession, piMessage))
    {
        return;
    }

    IMS_TRACE_D("Run : EarlyDialog[%s]", _TRACE_B_(bEarly), 0, 0);

    IMS_UINTP nNegoId = GetMediaNegoId(piSession);
    IMS_UINT32 nNetworkToneRtpTime = GetWaitingNetworkToneDuration(piSession, piMessage);
    SetNetworkToneRtpTimer(nNegoId, nNetworkToneRtpTime);
    if (!bEarly && nNetworkToneRtpTime == TIME_NO_WAIT_NW_TONE_RTP)
    {
        m_objContext.GetSupplementaryService().Delete(SuppType::ENFORCE_LT);
    }

    if (!m_piMediaSession->Run(nNegoId))
    {
        return;
    }

    m_pProfileManager->UpdateProfileForMediaActivation(piSession);
}

PUBLIC VIRTUAL void MtcMediaManager::SetRtpPort(
        IN ISession* piSession, IN IMS_UINT32 eMediaType, IN IMS_UINT32 nPort)
{
    IMS_TRACE_D("SetRtpPort : MediaType[%s] Port[%d]",
            MtcMediaStringUtils::ConvertContentType(eMediaType), nPort, 0);

    MEDIA_CONTENT_TYPE eContents = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaType);
    m_piMediaSession->SetOptions(
            GetMediaNegoId(piSession), IMediaSession::OptionType::SET_RTP_PORT, eContents, nPort);
}

PUBLIC VIRTUAL IMS_SINT32 MtcMediaManager::GetRemoteRtpPort(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    return m_piMediaSession->GetRemotePort(
            GetMediaNegoId(piSession), MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaType));
}

PUBLIC VIRTUAL void MtcMediaManager::SetConferenceCall(IN IMS_BOOL bConference)
{
    IMS_TRACE_D("SetConferenceCall : %d", bConference, 0, 0);

    // check the params for SetOptions()
    // check if negoId is necessary or not by the Media side.
    m_piMediaSession->SetOptions(
            UNDEFINED_NEGO_ID, IMediaSession::OptionType::SET_CONFERENCE_ENABLE, 0, 0);
}

PUBLIC VIRTUAL void MtcMediaManager::SetConfirmedSession(IN ISession* piSession)
{
    IMS_TRACE_D("SetConfirmedSession", 0, 0, 0);

    m_piMediaSession->SetOptions(GetMediaNegoId(piSession),
            IMediaSession::OptionType::SET_CONFIRMED_SESSION, IMS_TRUE, 0);

    m_pProfileManager->SetConfirmed(piSession, IMS_TRUE);
}

PUBLIC VIRTUAL NegotiationState MtcMediaManager::GetNegotiationState(IN ISession* piSession)
{
    if (!m_piMediaSession)
    {
        IMS_TRACE_D("GetNegotiationState : Fail to get the negotiation state.", 0, 0, 0);
        return NegotiationState::STATE_IDLE;
    }

    NegotiationState eState = m_piMediaSession->GetNegoState(GetMediaNegoId(piSession));
    IMS_TRACE_D("GetNegotiationState : [%s]", MtcMediaStringUtils::ConvertNegoState(eState), 0, 0);

    return eState;
}

PUBLIC VIRTUAL IMS_SINT32 MtcMediaManager::GetNegotiatedDirection(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    if (!m_piMediaSession)
    {
        IMS_TRACE_D("GetNegotiatedDirection : Fail to get the negotiated direction.", 0, 0, 0);
        return MEDIA_DIRECTION::MEDIA_DIRECTION_INVALID;
    }

    MEDIA_CONTENT_TYPE eContent = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaType);

    MEDIA_DIRECTION eDir =
            m_piMediaSession->GetNegotiatedDirection(GetMediaNegoId(piSession), eContent);
    IMS_TRACE_D("GetNegotiatedDirection : media type[%s], direction[%s]",
            MtcMediaStringUtils::ConvertContentType(eMediaType),
            MtcMediaStringUtils::ConvertDirection(eDir), 0);

    return eDir;
}

PUBLIC VIRTUAL IMS_SINT32 MtcMediaManager::GetNegotiatedQuality(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    MEDIA_CONTENT_TYPE eContent = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaType);
    IMS_SINT32 eQuality =
            m_piMediaSession->GetNegotiatedQuality(GetMediaNegoId(piSession), eContent);

    IMS_TRACE_D("GetNegotiatedQuality : media type[%s], quality[%s]",
            MtcMediaStringUtils::ConvertContentType(eMediaType),
            MtcMediaStringUtils::ConvertQuality(eQuality), 0);

    return eQuality;
}

PUBLIC VIRTUAL CallType MtcMediaManager::GetNegotiatedCallType(IN ISession* piSession)
{
    if (!m_piMediaSession)
    {
        IMS_TRACE_D("GetNegotiatedCallType : Fail to get the negotiated call type.", 0, 0, 0);
        return CallType::UNKNOWN;
    }

    MEDIA_CONTENT_TYPE eContents =
            m_piMediaSession->GetNegotiatedMediaType(GetMediaNegoId(piSession));

    IMS_TRACE_D("GetNegotiatedCallType : [%s]", MtcMediaStringUtils::ConvertContentType(eContents),
            0, 0);

    return MtcMediaUtil::GetCallTypeFromMediaContents(eContents);
}

PUBLIC VIRTUAL PemType MtcMediaManager::GetPemType(IN ISession* piSession)
{
    return m_pProfileManager->GetPemType(piSession);
}

PUBLIC VIRTUAL IMS_BOOL MtcMediaManager::IsAudioInactive()
{
    return m_bAudioInactive;
}

PUBLIC VIRTUAL void MtcMediaManager::AdjustDirectionForAutoOffer(IN CallType eCallType)
{
    const IMS_SINT32 eNewDirection =
            m_objContext.IsHeldByMe() ? DIRECTION_SEND : DIRECTION_SEND_RECEIVE;

    m_pMediaInfo->eAudioDirection = eNewDirection;

    if (eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT)
    {
        m_pMediaInfo->eVideoDirection = eNewDirection;
    }

    if (eCallType == CallType::RTT || eCallType == CallType::VIDEO_RTT)
    {
        m_pMediaInfo->eTextDirection = eNewDirection;
    }
}

PUBLIC VIRTUAL void MtcMediaManager::AdjustDirectionForAutoAnswer()
{
    if (!m_objContext.IsHeldByMe())
    {
        return;
    }

    std::vector<IMS_SINT32*> objAllMediaDirections{&m_pMediaInfo->eAudioDirection,
            &m_pMediaInfo->eVideoDirection, &m_pMediaInfo->eTextDirection};

    for (IMS_SINT32* pDirection : objAllMediaDirections)
    {
        if (*pDirection == DIRECTION_SEND_RECEIVE)
        {
            *pDirection = DIRECTION_SEND;
        }
        else if (*pDirection == DIRECTION_RECEIVE)
        {
            *pDirection = DIRECTION_INACTIVE;
        }
    }
}

PUBLIC VIRTUAL void MtcMediaManager::AdjustDirectionForLocalResourceConfirmation(
        IN CallType eCallType)
{
    SetDirectionToActiveFromInactive(MEDIATYPE_AUDIO, m_pMediaInfo->eAudioDirection);

    if (eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT)
    {
        SetDirectionToActiveFromInactive(MEDIATYPE_VIDEO, m_pMediaInfo->eVideoDirection);
    }

    if (eCallType == CallType::RTT || eCallType == CallType::VIDEO_RTT)
    {
        SetDirectionToActiveFromInactive(MEDIATYPE_TEXT, m_pMediaInfo->eTextDirection);
    }
}

PUBLIC VIRTUAL void MtcMediaManager::SetSrvccState(IN SrvccState eState)
{
    if (!m_piMediaSession)
    {
        IMS_TRACE_E(0, "SetSrvccState : No IMediaSession", 0, 0, 0);
        return;
    }

    MEDIA_SRVCC_STATUS eMediaSrvccStatus;
    switch (eState)
    {
        case SrvccState::IDLE:
            eMediaSrvccStatus = MEDIA_SRVCC_STATUS::MEDIA_SRVCC_IDLE;
            break;
        case SrvccState::STARTED:
            eMediaSrvccStatus = MEDIA_SRVCC_STATUS::MEDIA_SRVCC_STARTED;
            break;
        case SrvccState::SUCCEEDED:
            eMediaSrvccStatus = MEDIA_SRVCC_STATUS::MEDIA_SRVCC_SUCCEED;
            break;
        case SrvccState::FAILED:
            eMediaSrvccStatus = MEDIA_SRVCC_STATUS::MEDIA_SRVCC_FAILED;
            break;
        default:  // SrvccState::CANCELED:
            eMediaSrvccStatus = MEDIA_SRVCC_STATUS::MEDIA_SRVCC_CANCELED;
            break;
    }
    m_piMediaSession->NotifySrvccStatus(eMediaSrvccStatus);
}

PUBLIC VIRTUAL IMS_BOOL MtcMediaManager::IsOnHold()
{
    IMS_SINT32 nAudioDirection = m_pMediaInfo->eAudioDirection;
    return (nAudioDirection != DIRECTION_INVALID && nAudioDirection != DIRECTION_SEND_RECEIVE);
}

PUBLIC VIRTUAL IMS_UINT32 MtcMediaManager::GetSupportedMediaTypesFromSdp(IN ISession* piSession)
{
    return MtcMediaUtil::GetMediaTypesFromMediaContents(
            m_piMediaSession->GetSupportedMediaTypesFromSdp(GetMediaNegoId(piSession), piSession));
}

PRIVATE
void MtcMediaManager::UpdateLocalTone(IN ISession* piSession, IN IMessage* piMessage)
{
    IMS_TRACE_D("UpdateLocalTone", 0, 0, 0);
    IMS_BOOL bUseLocalTone = IMS_FALSE;

    /* 3GPP 24.628 - 4.7.2.1
     * NOTE 1 : In-band information received from the network overrides any locally generated
     * communication progress information also when the most recently received P-Early-Media header
     * fields of all early dialogs contain "inactive" or "recvonly".
     */
    if (!m_objContext.GetMessageUtils().IsResponseExist(piSession, SipStatusCode::SC_180))
    {
        bUseLocalTone = IMS_FALSE;
    }
    else
    {
        IMS_SINT32 nLocalRbtPolicy = m_objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT);

        IMS_TRACE_D("UpdateLocalTone : local RBT policy[%d]", nLocalRbtPolicy, 0, 0);

        switch (nLocalRbtPolicy)
        {
            case USE_DYNAMIC_NW_TONE_TIMER:
                bUseLocalTone = IMS_FALSE;
                break;
            case NOT_USE_DYNAMIC_NW_TONE_TIMER:
            {
                // AT&T, T-Mobile US
                PemType eType = GetPemType(piSession);
                bUseLocalTone = (eType != PemType::SENDONLY && eType != PemType::SENDRECV);
                break;
            }
            case LOCAL_TONE_WITH_180_BY_FORCE:
            {
                // Verizon
                IMS_SINT32 eCode =
                        (piMessage) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;

                bUseLocalTone = (eCode == SipStatusCode::SC_180);
                break;
            }
            default:
                break;
        }
    }

    SetLocalTone(bUseLocalTone);
    IMS_TRACE_D("UpdateLocalTone : use local ringback tone [%s]", _TRACE_B_(bUseLocalTone), 0, 0);
}

PRIVATE
void MtcMediaManager::UpdateLocalTone(IN ISession* piSession, IN IMS_BOOL bNetworkToneReceived)
{
    if (!IsDynamicRbtRequired(piSession))
    {
        return;
    }

    SetLocalTone(!bNetworkToneReceived);
    IMS_TRACE_D("UpdateLocalTone : use local ringback tone [%s]", _TRACE_B_(!bNetworkToneReceived),
            0, 0);
}

PRIVATE
void MtcMediaManager::SetNetworkToneRtpTimer(IN IMS_UINTP nNegoId, IN IMS_UINT32 nDuration)
{
    IMS_TRACE_D("SetNetworkToneRtpTimer : NegoId[%" PFLS_x "] Duration[%d]", nNegoId, nDuration, 0);
    m_piMediaSession->SetNetworkToneRtpTimer(nNegoId, MEDIA_TYPE_AUDIO, nDuration);
}

PRIVATE
IMS_BOOL MtcMediaManager::IsNecessaryToRunMedia(IN ISession* piSession, IN IMessage* piMessage)
{
    if (m_pProfileManager->IsConfirmed(piSession))
    {
        IMS_TRACE_D("IsNecessaryToRunMedia it is confirmed session.", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
    {
        IMS_TRACE_D("IsNecessaryToRunMedia MT case", 0, 0, 0);
        return IMS_FALSE;
    }

    /* IR.92 - 2.2.7 Early media and announcements
     * For SIP response 181 and 182 to the SIP INVITE, the UE must not locally render tones to
     * indicate diversion or queueing of calls.
     */
    IMS_SINT32 nStatusCode = (piMessage) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;
    if (nStatusCode == SipStatusCode::SC_181 || nStatusCode == SipStatusCode::SC_182)
    {
        IMS_TRACE_D("IsNecessaryToRunMedia Run the media in 181/182 response case.", 0, 0, 0);
        return IMS_TRUE;
    }

    PemType ePemType = GetPemType(piSession);
    if (ePemType == PemType::SENDONLY || ePemType == PemType::SENDRECV)
    {
        IMS_TRACE_D("IsNecessaryToRunMedia should play network RBT.", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_pProfileManager->IsPemSendInOtherEarlySession(piSession))
    {
        IMS_TRACE_D("IsNecessaryToRunMedia there's other session playing RBT.", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("IsNecessaryToRunMedia Run the media.", 0, 0, 0);
    return IMS_TRUE;
}

PRIVATE
IMS_UINTP MtcMediaManager::GetMediaNegoId(IN ISession* piSession)
{
    return m_pProfileManager->GetNegoId(piSession);
}

PRIVATE
IMS_UINT32 MtcMediaManager::GetWaitingNetworkToneDuration(
        IN ISession* piSession, IN IMessage* piMessage)
{
    if (m_pProfileManager->IsConfirmed(piSession))
    {
        if (m_pMediaInfo->eAudioDirection != DIRECTION_RECEIVE)
        {
            return TIME_NO_WAIT_NW_TONE_RTP;
        }

        return TIME_WAIT_NW_TONE_RTP;
    }

    if (!IsDynamicRbtRequired(piSession))
    {
        return TIME_NO_WAIT_NW_TONE_RTP;
    }

    IMS_SINT32 nStatusCode = (piMessage) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;
    if (nStatusCode == SipStatusCode::SC_181 || nStatusCode == SipStatusCode::SC_182)
    {
        return TIME_NO_WAIT_NW_TONE_RTP;
    }

    PemType ePemType = GetPemType(piSession);
    if (ePemType == PemType::SENDONLY || ePemType == PemType::SENDRECV)
    {
        return TIME_NO_WAIT_NW_TONE_RTP;
    }

    return TIME_WAIT_NW_TONE_RTP;
}

PRIVATE
void MtcMediaManager::HandleReceivingMediaDataStarted(IN IMS_UINT32 eMediaType)
{
    if (eMediaType == MEDIATYPE_VIDEO)
    {
        // TODO: Send CVO Result, INFO_TYPE_VIDEO_DATA_RECV for 3rd party call UI.
    }
}

PRIVATE
void MtcMediaManager::HandleReceivingNetworkTone(IN IMS_BOOL bNetworkToneReceived)
{
    IMS_TRACE_D("HandleReceivingNetworkTone", 0, 0, 0);
    ISession* piSession = m_pProfileManager->GetActiveSession();
    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bConfirmed = m_pProfileManager->IsConfirmed(piSession);
    if (bConfirmed && m_pMediaInfo->eAudioDirection != DIRECTION_RECEIVE)
    {
        return;
    }

    if (!bConfirmed)
    {
        UpdateLocalTone(piSession, bNetworkToneReceived);
    }
    else
    {
        MtcSupplementaryService& objSuppService = m_objContext.GetSupplementaryService();
        if (bNetworkToneReceived)
        {
            objSuppService.Delete(SuppType::ENFORCE_LT);
        }
        else
        {
            objSuppService.Add(SuppType::ENFORCE_LT, IMS_TRUE);
        }
    }

    if (bNetworkToneReceived)
    {
        m_pMediaReportListener->OnReceivingNetworkToneStarted();
    }
    else
    {
        m_pMediaReportListener->OnReceivingNetworkToneFailed();
    }
}

PRIVATE
IMS_BOOL MtcMediaManager::IsDynamicRbtRequired(IN ISession* piSession)
{
    if (!m_objContext.GetMessageUtils().IsResponseExist(piSession, SipStatusCode::SC_180))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nLocalRbtPolicy = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT);
    if (nLocalRbtPolicy != USE_DYNAMIC_NW_TONE_TIMER)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void MtcMediaManager::SetDirectionToActiveFromInactive(IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir)
{
    if (eDir == DIRECTION_INACTIVE)
    {
        UpdateMediaDirection(eMediaType, DIRECTION_SEND_RECEIVE);
    }
}
