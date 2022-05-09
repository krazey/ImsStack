#include "mtc/media/MtcMediaManager.h"
#include "media/MediaManager.h"
#include "utility/MessageUtil.h"
#include "ISipHeader.h"
#include "media/MtcMediaUtil.h"
#include "media/IMtcMediaManager.h"
#include "configuration/ConfigDef.h"
#include "SipStatusCode.h"
#include "helper/MtcSupplementaryService.h"
#include "media/IMediaQosEventListener.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcMediaManager::MtcMediaManager(IN IMtcCallContext& objContext) :
        m_pCallListener(IMS_NULL),
        m_pQosListener(IMS_NULL),
        m_objProfileManager(MtcMediaProfileManager()),
        m_objContext(objContext),
        m_pMediaInfo(new MediaInfo()),
        m_pOldMediaInfo(new MediaInfo()),
        m_bLocalTone(IMS_FALSE),
        m_eRtpBlockedMediaTypes(MEDIATYPE_NONE),
        m_piMediaSession(IMS_NULL),
        m_eState(MediaState::IDLE),
        m_eOldState(MediaState::IDLE)
{
    IMS_TRACE_D("+MtcMediaManager Callkey[%" PFLS_x "]", m_objContext.GetCallKey(), 0, 0);
}

PUBLIC VIRTUAL MtcMediaManager::~MtcMediaManager()
{
    IMS_TRACE_D("~MtcMediaManager Callkey[%" PFLS_x "]", m_objContext.GetCallKey(), 0, 0);

    m_pCallListener = IMS_NULL;
    m_pQosListener = IMS_NULL;

    if (m_pMediaInfo)
    {
        delete m_pMediaInfo;
    }

    if (m_pOldMediaInfo)
    {
        delete m_pOldMediaInfo;
    }

    DestroyAllMediaProfiles();
    DestroyMediaSession();
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_Notify(
        IN IMS_UINT32 eReportType, IN MEDIA_CONTENT_TYPE eMediaType /*= MEDIA_TYPE_INVALID*/)
{
    /**
     * REPORT_SUCCESS
     * REPORT_CLOSE_SESSION
     * REPORT_DATA_RECEIVE_FAILED
     * REPORT_DATA_RECEIVE_STARTED
     * REPORT_VIDEO_LOWEST_BIT_RATE
     * REPORT_CHECK_RADIO_CONNECTION
     * REPORT_NW_TONE_RTP_RECEIVE_STARTED
     * REPORT_NW_TONE_RTP_RECEIVE_FAILED
     */

    IMS_TRACE_D("MediaSession_Notify : Report[%d] Media[%d]", eReportType, eMediaType, 0);
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_NotifyFailures(IN IMS_UINT32 eReportType,
        IN RtpError eError, IN MEDIA_CONTENT_TYPE eMediaType /*= MEDIA_TYPE_INVALID*/)
{
    IMS_TRACE_D("MediaSession_NotifyFailures : Report[%d] Error[%d] Media[%d]", eReportType, eError,
            eMediaType);
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_NotifyQos(IN IMS_UINTP nNegoId,
        IN IMS_BOOL bSuccess, IN MEDIA_CONTENT_TYPE eMediaType /*= MEDIA_TYPE_INVALID*/)
{
    IMS_TRACE_D("MediaSession_NotifyQos : NegoId[%d] Result[%d] Media[%d]", nNegoId, bSuccess,
            eMediaType);

    m_pQosListener->OnQosStatusChanged(m_objProfileManager.GetSessionWithNegoId(nNegoId),
            (bSuccess) ? QosStatus::AVAILABLE : QosStatus::LOST,
            MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaType));
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_RTPInfo(IN IMediaRTPInfoMsgParam* pMsg)
{
    IMS_TRACE_D("MediaSession_RTPInfo", 0, 0, 0);
    UNUSED_PARAM(pMsg);
}

PUBLIC VIRTUAL void MtcMediaManager::MediaSession_DRAInfo(IN IMediaDRAMsgParam* pMsg)
{
    IMS_TRACE_D("MediaSession_DRAInfo", 0, 0, 0);
    UNUSED_PARAM(pMsg);
}

PUBLIC VIRTUAL void MtcMediaManager::SetCallListener(IN IUCMediaListener* pListener)
{
    m_pCallListener = pListener;
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
        m_pOldMediaInfo->eADir = m_pMediaInfo->eADir;
        m_pMediaInfo->eADir = eDir;
        IMS_TRACE_D("UpdateMediaDirection : audio [%d]->[%d]", m_pOldMediaInfo->eADir,
                m_pMediaInfo->eADir, 0);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        m_pOldMediaInfo->eVDir = m_pMediaInfo->eVDir;
        m_pMediaInfo->eVDir = eDir;
        IMS_TRACE_D("UpdateMediaDirection : video [%d]->[%d]", m_pOldMediaInfo->eVDir,
                m_pMediaInfo->eVDir, 0);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        m_pOldMediaInfo->eTDir = m_pMediaInfo->eTDir;
        m_pMediaInfo->eTDir = eDir;
        IMS_TRACE_D("UpdateMediaDirection : text [%d]->[%d]", m_pOldMediaInfo->eTDir,
                m_pMediaInfo->eTDir, 0);
    }
}

PUBLIC VIRTUAL void MtcMediaManager::UpdateMediaQuality(
        IN IMS_UINT32 eMediaType, IN IMS_SINT32 eQuality)
{
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        m_pOldMediaInfo->eAQuality = m_pMediaInfo->eAQuality;
        m_pMediaInfo->eAQuality = eQuality;
        IMS_TRACE_D("UpdateMediaQuality : audio [%d]->[%d]", m_pOldMediaInfo->eAQuality,
                m_pMediaInfo->eAQuality, 0);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        m_pOldMediaInfo->eVQuality = m_pMediaInfo->eVQuality;
        m_pMediaInfo->eVQuality = eQuality;
        IMS_TRACE_D("UpdateMediaQuality : video [%d]->[%d]", m_pOldMediaInfo->eVQuality,
                m_pMediaInfo->eVQuality, 0);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        m_pOldMediaInfo->eGTTMode = m_pMediaInfo->eGTTMode;
        m_pMediaInfo->eGTTMode = eQuality;
        IMS_TRACE_D("UpdateMediaQuality : text [%d]->[%d]", m_pOldMediaInfo->eGTTMode,
                m_pMediaInfo->eGTTMode, 0);
    }
}

PUBLIC VIRTUAL void MtcMediaManager::GetMediaInfo(OUT MediaInfo& objInfo)
{
    objInfo = *m_pMediaInfo;
}

PUBLIC VIRTUAL void MtcMediaManager::GetOldMediaInfo(OUT MediaInfo& objInfo)
{
    objInfo = *m_pOldMediaInfo;
}

PUBLIC VIRTUAL void MtcMediaManager::RestoreMediaInfo()
{
    MediaInfo objOldMediaInfo(*m_pOldMediaInfo);
    SetMediaInfo(objOldMediaInfo);
}

PUBLIC VIRTUAL void MtcMediaManager::CreateMediaSession(IN JniMediaSessionThread* pJniMediaThread)
{
    MediaManager* pMediaManager = MediaManager::GetInstance(m_objContext.GetSlotId());
    if (!pMediaManager)
    {
        return;
    }

    ServiceType eServiceType = m_objContext.GetService().GetServiceType();
    MEDIA_SERVICE_TYPE eMediaServiceType = MEDIA_SERVICE_NONE;

    if (eServiceType == ServiceType::NORMAL)
    {
        eMediaServiceType = MEDIA_SERVICE_DEFAULT;
    }
    else if (eServiceType == ServiceType::EMERGENCY)
    {
        eMediaServiceType = MEDIA_SERVICE_EMERGENCY;
    }

    IMS_TRACE_D("CreateMediaSession", 0, 0, 0);

    m_piMediaSession = pMediaManager->CreateSession(
            eMediaServiceType, m_objContext.GetCallKey(), pJniMediaThread);

    if (m_piMediaSession == IMS_NULL)
    {
        IMS_TRACE_D("CreateMediaSession : Failed", 0, 0, 0);
        return;
    }

    m_piMediaSession->SetMtcListener(this);

    MediaEnvironment* pEnvironment = new MediaEnvironment();
    pEnvironment->eNetworkType =
            MtcMediaUtil::GetMediaNetworkType(&m_objContext.GetService(), m_objContext.GetSlotId());
    pEnvironment->eServiceType = eMediaServiceType;
    pEnvironment->pIService = (IService*)m_objContext.GetService().GetICoreService();

    if (!m_piMediaSession->SetEnvironment(pEnvironment))
    {
        IMS_TRACE_D("Setting media environment is failed.", 0, 0, 0);
        delete pEnvironment;
    }
}

PUBLIC VIRTUAL void MtcMediaManager::DestroyMediaSession()
{
    MediaManager* pMediaManager = MediaManager::GetInstance(m_objContext.GetSlotId());
    if (!pMediaManager)
    {
        IMS_TRACE_D("DestroyMediaSession : Failed to destroy Media Session.", 0, 0, 0);
        return;
    }

    if (GetState() >= MediaState::TERMINATING)
    {
        return;
    }

    IMS_TRACE_D("DestroyMediaSession", 0, 0, 0);
    pMediaManager->DestroySession((MediaSession*)m_piMediaSession);
}

PUBLIC VIRTUAL void MtcMediaManager::CreateMediaProfile(
        IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOrigin)
{
    IMS_TRACE_D("CreateMediaProfile", 0, 0, 0);
    m_objProfileManager.CreateMediaProfile(piSession, bForked, bOrigin, m_piMediaSession);
}

PUBLIC VIRTUAL void MtcMediaManager::DestroyMediaProfile(IN ISession* piSession)
{
    if (GetState() >= MediaState::TERMINATING)
    {
        return;
    }

    IMS_TRACE_D("DestroyMediaProfile", 0, 0, 0);
    m_objProfileManager.DestroyMediaProfile(piSession, m_piMediaSession);
}

PUBLIC
void MtcMediaManager::DestroyAllMediaProfiles()
{
    if (GetState() >= MediaState::TERMINATING)
    {
        return;
    }

    IMS_TRACE_D("DestroyAllMediaProfiles", 0, 0, 0);
    m_objProfileManager.DestroyAllMediaProfiles(m_piMediaSession);
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

PUBLIC VIRTUAL MediaState MtcMediaManager::GetState()
{
    return m_eState;
}

PUBLIC VIRTUAL MediaState MtcMediaManager::GetOldState()
{
    return m_eOldState;
}

PUBLIC VIRTUAL IMS_RESULT MtcMediaManager::FormSdp(IN ISession* piSession, IN CallType eCallType)
{
    if (GetNegotiationState(piSession) == NegotiationState::STATE_OFFER_SENT)
    {
        IMS_TRACE_D("FormSdp : Failed to form SDP because nego state is offer-sent.", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_D("FormSdp : CallType[%d]", eCallType, 0, 0);
    MEDIA_CONTENT_TYPE eContents = MtcMediaUtil::GetMediaContentsFromCallType(eCallType);

    IMS_BOOL bResult = m_piMediaSession->FormSDP(GetMediaNegoId(piSession), piSession, eContents,
            m_pMediaInfo->eADir, m_pMediaInfo->eVDir, m_pMediaInfo->eTDir);

    if (!bResult)
    {
        RestoreMediaInfo();
    }

    return (bResult) ? IMS_SUCCESS : IMS_FAILURE;
}

PUBLIC VIRTUAL NegotiationResult MtcMediaManager::NegotiateSdp(IN ISession* piSession)
{
    MediaNego::MediaNegoResult eErrorReason = MediaNego::MediaNegoResult::NO_ERROR;
    IMS_SINT32 eADir = DIRECTION_INVALID;
    IMS_SINT32 eVDir = DIRECTION_INVALID;
    IMS_SINT32 eTDir = DIRECTION_INVALID;

    m_piMediaSession->NegotiateSDP(
            GetMediaNegoId(piSession), piSession, &eADir, &eVDir, &eTDir, eErrorReason);

    IMS_TRACE_D("NegotiateSdp : %d", eErrorReason, 0, 0);

    if (eErrorReason == NegotiationResult::NO_ERROR)
    {
        MediaInfo objInfo(eADir, eVDir, eTDir, GetNegotiatedQuality(piSession, MEDIATYPE_AUDIO),
                GetNegotiatedQuality(piSession, MEDIATYPE_VIDEO),
                MtcMediaUtil::GetGttModeFromTextQuality(
                        GetNegotiatedQuality(piSession, MEDIATYPE_TEXT)));
        SetMediaInfo(objInfo);
    }

    return eErrorReason;
}

PUBLIC VIRTUAL void MtcMediaManager::RestoreSdp(IN ISession* piSession)
{
    RestoreMediaInfo();
    FinalizeSdp(piSession);
    piSession->Restore();
}

PUBLIC
void MtcMediaManager::FinalizeSdp(IN ISession* piSession)
{
    // don't finalizeSDP when the session is in early dialog state.
    if (!m_objProfileManager.IsConfirmed(piSession))
    {
        IMS_TRACE_D("FinalizeSdp : Do not finalize SDP - it is the confirmed session.", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("FinalizeSdp", 0, 0, 0);

    m_piMediaSession->FinalizeSDP(GetMediaNegoId(piSession), piSession);
}

PUBLIC VIRTUAL void MtcMediaManager::HandleRingBackTone(
        IN ISession* piSession, IN IMessage* piMessage)
{
    IMS_TRACE_D("HandleRingBackTone", 0, 0, 0);

    UpdatePemType(piSession, piMessage);
    UpdateLocalTone(piSession);
}

PUBLIC
void MtcMediaManager::HandleDynamicNetworkTone(
        IN ISession* piSession, IN IMS_BOOL b180Received, OUT IMS_UINT32& nDuration)
{
    if (m_objProfileManager.IsConfirmed(piSession))  // Confirmed Dialog State
    {
        if (m_pMediaInfo->eADir != DIRECTION_RECEIVE)
        {
            nDuration = 0;
            // TODO: detach ENFORCE_LT from the MtcSupplementaryService
            m_objContext.GetSupplementaryService().Delete(SuppType::ENFORCE_LT);
        }
    }
    else  // Early Dialog State
    {
        // if (KEY_IGNORE_P_EARLY_MEDIA_HEADER_BOOL == TRUE && vzw case) return;
        // else ..
        PemType ePemType = GetPemType(piSession);
        if ((ePemType == PemType::SENDRECV || ePemType == PemType::SENDONLY) || !b180Received)
        {
            nDuration = 0;
        }
    }

    IMS_TRACE_D("HandleDynamicNetworkTone : Duration[%d]", nDuration, 0, 0);
}

PUBLIC
void MtcMediaManager::SetNetworkToneRtpTimer(IN IMS_UINT32 eMediaTypes, IN IMS_UINT32 nDuration)
{
    MEDIA_CONTENT_TYPE eContents = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes);

    IMS_TRACE_D("SetNetworkToneRtpTimer : MediaType[%d] Duration[%d]", eMediaTypes, nDuration, 0);

    m_piMediaSession->SetNetworkToneRTPTimer(eContents, nDuration);
}

PUBLIC VIRTUAL void MtcMediaManager::Run(IN ISession* piSession, IN IMessage* piMessage,
        IN IMS_BOOL bEarly, IN IMS_BOOL b180Received)
{
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

    IMS_UINT32 nNetworkToneTimerDuration = 2000;  // 2 sec

    // TODO: temp code? is this going to move to Mtc Enablers?
    // will modify to check 180 response from all early dialogs
    b180Received = IMS_FALSE;
    IMSList<IMessage*> objResponses = piSession->GetPreviousResponses(IMessage::SESSION_START);

    for (IMS_UINT32 index = 0; index < objResponses.GetSize(); index++)
    {
        if (objResponses.GetAt(index)->GetStatusCode() == SipStatusCode::SC_180)
        {
            b180Received = IMS_TRUE;
            break;
        }
    }

    HandleDynamicNetworkTone(piSession, b180Received, nNetworkToneTimerDuration);

    /* IR.92 - 2.2.7 Early media and announcements
     * For SIP response 181 and 182 to the SIP INVITE, the UE must not locally render tones to
     * indicate diversion or queueing of calls. */
    IMS_SINT32 nStatusCode = (!piMessage) ? SipStatusCode::SC_INVALID : piMessage->GetStatusCode();
    if (nStatusCode == SipStatusCode::SC_181 || nStatusCode == SipStatusCode::SC_182)
    {
        nNetworkToneTimerDuration = 0;
    }

    IMS_TRACE_D(
            "Run EarlyDialog[%s] 180-Received[%s]", _TRACE_B_(bEarly), _TRACE_B_(b180Received), 0);

    if (!m_piMediaSession->Run(GetMediaNegoId(piSession)))
    {
        return;
    }

    SetState(MediaState::STARTING);
    SetNetworkToneRtpTimer(MEDIATYPE_AUDIO, nNetworkToneTimerDuration);
    m_objProfileManager.UpdateProfileForMediaActivation(piSession);
}

PUBLIC VIRTUAL void MtcMediaManager::Terminate()
{
    IMS_TRACE_D("Terminate", 0, 0, 0);
    SetState(MediaState::TERMINATING);

    if (!m_piMediaSession)
    {
        IMS_TRACE_D("Terminate : nothing to terminate for media.", 0, 0, 0);
        return;

    }

    m_piMediaSession->Terminate();
}

PUBLIC VIRTUAL void MtcMediaManager::SetRtpPort(
        IN ISession* piSession, IN IMS_UINT32 eMediaTypes, IN IMS_UINT32 nPort)
{
    IMS_TRACE_D("SetRtpPort : MediaType[%d] Port[%d]", eMediaTypes, nPort, 0);

    MEDIA_CONTENT_TYPE eContents = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes);
    m_piMediaSession->SetOptions(
            GetMediaNegoId(piSession), IMediaSession::OptionType::SET_RTP_PORT, eContents, nPort);
}

PUBLIC VIRTUAL void MtcMediaManager::RequestVideoDataUsage()
{
    IMS_TRACE_D("RequestVideoDataUsage", 0, 0, 0);

    IMSMSG objMsg;
    objMsg.nMSG = IUMedia::VIDEO_DATA_USAGE_CMD;
    m_piMediaSession->SendMessage(objMsg);
}

PUBLIC VIRTUAL void MtcMediaManager::SetEnforcedDirection(
        IN IMS_UINT32 eMediaTypes, IN IMS_SINT32 eDir)
{
    IMS_TRACE_D("SetEnforcedDirection : MediaType[%d] Dir[%d]", eMediaTypes, eDir, 0);

    // check if negoId is necessary or not by the Media side.
    MEDIA_CONTENT_TYPE eContents = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes);
    m_piMediaSession->SetOptions(
            IMS_NULL, IMediaSession::OptionType::SET_DIRECTION, eContents, eDir);
}

PUBLIC VIRTUAL IMS_BOOL MtcMediaManager::GetCvoResult(IN ISession* piSession)
{
    IMS_BOOL bResult = IMS_FALSE;
    UNUSED_PARAM(piSession);
    // bResult = m_piMediaSession->GetNegotiatedCVOResult(GetMediaNegoId(piSession));

    IMS_TRACE_D("GetCvoResult : %d", bResult, 0, 0);

    return bResult;
}

PUBLIC VIRTUAL void MtcMediaManager::SendFastVideoUpdate()
{
    IMS_TRACE_D("SendFastVideoUpdate", 0, 0, 0);

    // check the params for SetOptions() - video phase
    // check if negoId is necessary or not by the Media side.
    m_piMediaSession->SetOptions(IMS_NULL, IMediaSession::OptionType::SEND_FAST_VIDEO_UPDATE, 0, 0);
}

PUBLIC VIRTUAL void MtcMediaManager::SetConferenceCall(IN IMS_BOOL bConference)
{
    IMS_TRACE_D("SetConferenceCall : %d", bConference, 0, 0);

    // check the params for SetOptions()
    // check if negoId is necessary or not by the Media side.
    m_piMediaSession->SetOptions(IMS_NULL, IMediaSession::OptionType::SET_CONFERENCE_ENABLE, 0, 0);
}

PUBLIC VIRTUAL void MtcMediaManager::SetConfirmedSession(IN ISession* piSession)
{
    IMS_TRACE_D("SetConfirmedSession", 0, 0, 0);

    m_piMediaSession->SetOptions(GetMediaNegoId(piSession),
            IMediaSession::OptionType::SET_CONFIRMED_SESSION, IMS_TRUE, 0);

    m_objProfileManager.HandleProfilesInConfirmedState(piSession, m_piMediaSession);
}

PUBLIC VIRTUAL void MtcMediaManager::UpdateStatsReportOption(IN IMS_UINT32 eAction)
{
    IMS_TRACE_D("UpdateStatsReportOption : %d", eAction, 0, 0);

    // param is not defined for this option.
    m_piMediaSession->SetOptions(
            IMS_NULL, IMediaSession::OptionType::SET_DRA_REPORT_OPTION, eAction, 0);
}

PUBLIC VIRTUAL NegotiationState MtcMediaManager::GetNegotiationState(IN ISession* piSession)
{
    if (!m_piMediaSession)
    {
        IMS_TRACE_D("GetNegotiationState : Fail to get the negotiation state.", 0, 0, 0);
        return NegotiationState::STATE_IDLE;
    }

    NegotiationState eState = m_piMediaSession->GetNegoState(GetMediaNegoId(piSession));
    IMS_TRACE_D("GetNegotiationState : %d", eState, 0, 0);

    return eState;
}

PUBLIC VIRTUAL IMS_SINT32 MtcMediaManager::GetNegotiatedDirection(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    MEDIA_CONTENT_TYPE eContent = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaType);

    MEDIA_DIRECTION eDir =
            m_piMediaSession->GetNegotiatedDirection(GetMediaNegoId(piSession), eContent);
    IMS_TRACE_D("GetNegotiatedDirection : %d", eDir, 0, 0);

    return eDir;
}

PUBLIC VIRTUAL IMS_SINT32 MtcMediaManager::GetNegotiatedQuality(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    MEDIA_CONTENT_TYPE eContent = MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaType);
    IMS_SINT32 eQuality =
            m_piMediaSession->GetNegotiatedQuality(GetMediaNegoId(piSession), eContent);

    IMS_TRACE_D("GetNegotiatedQuality : %d", eQuality, 0, 0);

    return eQuality;
}

PUBLIC VIRTUAL CallType MtcMediaManager::GetNegotiatedCallType(IN ISession* piSession)
{
    MEDIA_CONTENT_TYPE eContents =
            m_piMediaSession->GetNegotiatedMediaType(GetMediaNegoId(piSession));

    IMS_TRACE_D("GetNegotiatedCallType : %d", eContents, 0, 0);

    return MtcMediaUtil::GetCallTypeFromMediaContents(eContents);
}

PUBLIC VIRTUAL IMS_BOOL MtcMediaManager::IsAudioQualityHd()
{
    switch (m_pMediaInfo->eAQuality)
    {
        case AUDIO_QUALITY_AMR_WB:
        case AUDIO_QUALITY_EVS:
        case AUDIO_QUALITY_EVS_NB:
        case AUDIO_QUALITY_EVS_WB:
        case AUDIO_QUALITY_EVS_SWB:
        case AUDIO_QUALITY_EVS_FB:
            return IMS_TRUE;
        default:
            break;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL PemType MtcMediaManager::GetPemType(IN ISession* piSession)
{
    return m_objProfileManager.GetPemType(piSession);
}

PUBLIC VIRTUAL IMS_BOOL MtcMediaManager::IsAudioMediaActivated()
{
    return (m_pMediaInfo->eADir != DIRECTION_INACTIVE) ? IMS_TRUE : IMS_FALSE;
}

PRIVATE
void MtcMediaManager::SetState(IN MediaState eState)
{
    if (m_eState == eState)
    {
        return;
    }

    m_eOldState = m_eState;
    m_eState = eState;
}

PRIVATE
void MtcMediaManager::FinalizeMediaInfo(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("FinalizeMediaInfo", 0, 0, 0);
    // it is called by FinalizeSDP()
    MEDIA_CONTENT_TYPE eContentType = m_piMediaSession->GetNegotiatedMediaType(nNegoId);

    if (eContentType & MEDIA_TYPE_AUDIO)
    {
        MEDIA_DIRECTION eDir = m_piMediaSession->GetNegotiatedDirection(nNegoId, MEDIA_TYPE_AUDIO);
        IMS_SINT32 eQuality = m_piMediaSession->GetNegotiatedQuality(nNegoId, MEDIA_TYPE_AUDIO);

        UpdateMediaDirection(MEDIATYPE_AUDIO, eDir);
        UpdateMediaQuality(MEDIATYPE_AUDIO, eQuality);
    }

    if (eContentType & MEDIA_TYPE_VIDEO)
    {
        MEDIA_DIRECTION eDir = m_piMediaSession->GetNegotiatedDirection(nNegoId, MEDIA_TYPE_VIDEO);
        IMS_SINT32 eQuality = m_piMediaSession->GetNegotiatedQuality(nNegoId, MEDIA_TYPE_VIDEO);

        UpdateMediaDirection(MEDIATYPE_VIDEO, eDir);
        UpdateMediaQuality(MEDIATYPE_VIDEO, eQuality);
    }

    if (eContentType & MEDIA_TYPE_TEXT)
    {
        MEDIA_DIRECTION eDir = m_piMediaSession->GetNegotiatedDirection(nNegoId, MEDIA_TYPE_TEXT);
        IMS_SINT32 eQuality = m_piMediaSession->GetNegotiatedQuality(nNegoId, MEDIA_TYPE_TEXT);

        UpdateMediaDirection(MEDIATYPE_TEXT, eDir);
        UpdateMediaQuality(MEDIATYPE_TEXT, eQuality);
    }
}

PRIVATE
void MtcMediaManager::UpdatePemType(IN ISession* piSession, IN IMessage* piMessage)
{
    // if (KEY_IGNORE_P_EARLY_MEDIA_HEADER_BOOL == IMS_TRUE) return;

    AString strPemValue;
    IMS_RESULT nResult =
            MessageUtil::GetHeaderValue(piMessage, ISipHeader::P_EARLY_MEDIA, strPemValue);

    if (nResult == IMS_FAILURE)  // no P-Early-Media header or no value
    {
        // TMUS : Set PemType::NONE after UE receives 180 response
        return;
    }

    PemType ePemType = GetPemType(piSession);

    if (strPemValue.Contains("sendrecv"))
    {
        ePemType = PemType::SENDRECV;
    }
    else if (strPemValue.Contains("sendonly"))
    {
        ePemType = PemType::SENDONLY;
    }
    else if (strPemValue.Contains("recvonly"))
    {
        ePemType = PemType::RECVONLY;
    }
    else if (strPemValue.Contains("inactive"))
    {
        ePemType = PemType::INACTIVE;
    }

    // When UE receives P-Early-Media header with "gated", PemType will be keep the value.
    IMS_TRACE_D("UpdatePemType : %d", ePemType, 0, 0);

    m_objProfileManager.SetPemType(piSession, ePemType);
}

PRIVATE
void MtcMediaManager::UpdateLocalTone(IN ISession* piSession)
{
    if (GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
    {
        return;
    }

    IMS_TRACE_D("UpdateLocalTone", 0, 0, 0);

    /* 3GPP 24.628 - 4.7.2.1
     * NOTE 1 : In-band information received from the network overrides any locally generated
     * communication progress information also when the most recently received P-Early-Media header
     * fields of all early dialogs contain "inactive" or "recvonly".
     */
    SetLocalTone(IMS_FALSE);

    // Add asset to distinguish the way playing local tone with P-Early-Media value.
    // 1) UE has never received a P-Early-Media header :
    //      play the local RBT for AT&T. Standard - local policy.
    // 2) UE receives P-Ealry-Media with inactive :
    //      play local tone after receiving 180. or don't play any media stream. - TMUS

    // KEY_IGNORE_P_EARLY_MEDIA_HEADER_BOOL == true && vzw case
    // SetLocalTone(IMS_FALSE);
    // if (StatusCode == 183)
    // {
    //      if (m_pMediaInfo->eADir == DIRECTION_INVALID) SetEnforcedDirection(AUDIO, SENDRECV);
    // }
    // else if (StatusCode == 180)
    // {
    //      if (!HasSdp()) SetLocalTone(IMS_TRUE);
    //      SetEnforcedDirection(AUDIO, INVALID);
    // }
}

PRIVATE
void MtcMediaManager::SendAudioInfoToJava(IN ISession* piSession)
{
    UNUSED_PARAM(piSession);
    // IMS_EVENT_CALL_MEIDA_INFO, SlotId, MediaSession::GetNegotiatedQuality(), CodecBitrate()
}

PRIVATE
IMS_BOOL MtcMediaManager::IsNecessaryToRunMedia(IN ISession* piSession, IN IMessage* piMessage)
{
    if (m_objProfileManager.IsConfirmed(piSession))
    {
        IMS_TRACE_D("IsNecessaryToRunMedia : it is confirmed session.", 0, 0, 0);
        return IMS_TRUE;
    }

    if (!MessageUtil::HasSdp(piMessage) || IsLocalTone())
    {
        IMS_TRACE_D("IsNecessaryToRunMedia : no SDP body or local tone should be played.", 0, 0, 0);
        return IMS_FALSE;
    }

    PemType ePemType = m_objProfileManager.GetPemType(piSession);
    if (ePemType == PemType::SENDONLY || ePemType == PemType::SENDRECV)
    {
        IMS_TRACE_D("IsNecessaryToRunMedia : should play network RBT.", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_objProfileManager.IsPemSendInOtherEarlySession(piSession))
    {
        IMS_TRACE_D("IsNecessaryToRunMedia : there's other session playing RBT.", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("IsNecessaryToRunMedia : Run the media.", 0, 0, 0);
    return IMS_TRUE;
}

PRIVATE
IMS_UINTP MtcMediaManager::GetMediaNegoId(IN ISession* piSession)
{
    return m_objProfileManager.GetNegoId(piSession);
}

PRIVATE
void MtcMediaManager::HandleMediaSuccess() {}

PRIVATE
void MtcMediaManager::HandleReceivingRtpDataStarted()
{
    m_eRtpBlockedMediaTypes = MEDIATYPE_NONE;
}

PRIVATE
void MtcMediaManager::HandleReceivingRtpDataFailed() {}

PRIVATE
void MtcMediaManager::HandleMediaError() {}

PRIVATE
void MtcMediaManager::HandleReceivingNetworkToneStarted() {}

PRIVATE
void MtcMediaManager::HandleReceivingNetworkToneFailed() {}

PRIVATE
void MtcMediaManager::HandleReceivedDtmfEvent() {}
