#ifndef MTC_MEDIA_MANAGER_H_
#define MTC_MEDIA_MANAGER_H_

#include "IMediaSession.h"
#include "IMSTypeDef.h"
#include "IUMedia.h"
#include "MediaDef.h"
#include "MtcDef.h"

#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"

#include "media/IMediaSessionClientListener.h"
#include "media/IMtcMediaListener.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaProfileManager.h"

class MtcMediaManager : public IMtcMediaManager, public IMediaSessionClientListener
{
public:
    MtcMediaManager(IN IMtcCallContext& objContext);
    virtual ~MtcMediaManager();

public: /* IMediaSessionClientListener */
    virtual void MediaSession_Notify(IN IMS_UINT32 eReportType,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

    virtual void MediaSession_NotifyFailures(IN IMS_UINT32 eReportType, IN RtpError eError,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

    virtual void MediaSession_NotifyQos(IN IMS_UINTP nNegoId, IN IMS_BOOL bSuccess,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

    virtual void MediaSession_RTPInfo(IN IMediaRTPInfoMsgParam* pMsg) override;
    virtual void MediaSession_DRAInfo(IN IMediaDRAMsgParam* pMsg) override;

public:
    virtual void SetCallListener(IN IUCMediaListener* pListener) override;
    virtual void SetQosListener(IN IMediaQosEventListener* pListener) override;

    /* Media Info */
    virtual void SetMediaInfo(IN const MediaInfo& objInfo) override;
    virtual void UpdateMediaDirection(IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir) override;
    virtual void UpdateMediaQuality(IN IMS_UINT32 eMediaType, IN IMS_SINT32 eQuality) override;
    // using enum values defined in MtcDef.h
    virtual void GetMediaInfo(OUT MediaInfo& objInfo) override;
    virtual void GetOldMediaInfo(OUT MediaInfo& objInfo) override;
    virtual void RestoreMediaInfo() override;

    /* MediaSession */
    virtual void CreateMediaSession(IN JniMediaSessionThread* pJniMediaThread) override;
    virtual void DestroyMediaSession() override;

    /* Media Profile */
    virtual void CreateMediaProfile(
            IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOriginalProfile) override;
    virtual void DestroyMediaProfile(IN ISession* piSession) override;
    void DestroyAllMediaProfiles();  // called when terminate media

    /* Local Tone - public or private */
    void SetLocalTone(IN IMS_BOOL bLocalTone);  // private?
    virtual IMS_BOOL IsLocalTone() override;

    /* Media State */
    virtual MediaState GetState() override;
    virtual MediaState GetOldState() override;

    /* Handling SDP */
    virtual IMS_RESULT FormSdp(IN ISession* piSession, IN CallType eCallType) override;
    virtual NegotiationResult NegotiateSdp(IN ISession* piSession) override;
    virtual void RestoreSdp(IN ISession* piSession) override;
    void FinalizeSdp(IN ISession* piSession);

    /* P-Early-Media */
    virtual void UpdatePemType(IN ISession* piSession, IN IMessage* piMessage) override;

    /* Media Operations */
    virtual void Run(IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly,
            IN IMS_BOOL bNegoUpdated = IMS_TRUE) override;

    virtual void Terminate() override;

    // tMediaValueInfo GetNegotiatedMediaValue(IN ISession* piSession, IN IMS_UINT32 eMediaTypes);
    /* won't use MediaSession::GetNegotiatedMediaValue() anymore.
       We need to use the new following APIs.
       MediaSession::GetNegotiatedQuality(), MediaSession::GetNegotiatedCodecBitrate()
       MediaSession::GetNegotiatedCodecBandWidth() */

    virtual void SetRtpPort(
            IN ISession* piSession, IN IMS_UINT32 eMediaTypes, IN IMS_UINT32 nPort) override;
    virtual void RequestVideoDataUsage() override;
    virtual void SetEnforcedDirection(IN IMS_UINT32 eMediaTypes, IN IMS_SINT32 eDir) override;
    virtual IMS_BOOL GetCvoResult(IN ISession* piSession) override;
    virtual void SendFastVideoUpdate() override;
    virtual void SetConferenceCall(IN IMS_BOOL bConference) override;
    virtual void SetConfirmedSession(IN ISession* piSession) override;
    virtual void UpdateStatsReportOption(IN IMS_UINT32 eAction) override;

    virtual NegotiationState GetNegotiationState(IN ISession* piSession) override;
    virtual IMS_SINT32 GetNegotiatedDirection(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    virtual IMS_SINT32 GetNegotiatedQuality(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    virtual CallType GetNegotiatedCallType(IN ISession* piSession) override;

    /* Provide information of MtcMediaManager */
    virtual IMS_BOOL IsAudioQualityHd() override;
    virtual PemType GetPemType(IN ISession* piSession) override;  // remove..?
    virtual IMS_BOOL IsAudioMediaActivated() override;

private:
    void SetState(IN MediaState eState);
    void FinalizeMediaInfo(IN IMS_UINTP nNegoId);
    void UpdateLocalTone(IN ISession* piSession);
    void SetNetworkToneRtpTimer(IN IMS_UINT32 eMediaTypes, IN IMS_UINT32 nDuration);  // TBD
    void SendAudioInfoToJava(IN ISession* piSession);

    IMS_BOOL IsNecessaryToRunMedia(IN ISession* piSession, IN IMessage* piMessage);
    IMS_UINTP GetMediaNegoId(IN ISession* piSession);
    IMS_UINT32 GetDurationWaitingNetworkTone(IN ISession* piSession, IN IMessage* piMessage);

    void HandleMediaSuccess();
    void HandleReceivingRtpDataStarted();
    void HandleReceivingRtpDataFailed();
    void HandleMediaError();
    void HandleReceivingNetworkToneStarted();
    void HandleReceivingNetworkToneFailed();
    void HandleReceivedDtmfEvent();

private:
    IUCMediaListener* m_pCallListener;
    IMediaQosEventListener* m_pQosListener;
    MtcMediaProfileManager m_objProfileManager;
    IMtcCallContext& m_objContext;
    MediaInfo* m_pMediaInfo;
    MediaInfo* m_pOldMediaInfo;
    IMS_BOOL m_bLocalTone;
    IMS_UINT32 m_eRtpBlockedMediaTypes;  // m_bAudioRTPBlock, m_bVideoRTPBlock, m_bTextRTPBlock
    IMediaSession* m_piMediaSession;
    MediaState m_eState;
    MediaState m_eOldState;

    static const IMS_UINT32 TIME_WAIT_NW_TONE_RTP = 1000;
    static const IMS_UINT32 TIME_NO_WAIT_NW_TONE_RTP = 0;

    enum
    {
        USE_DYNAMIC_NW_TONE_TIMER = 0,
        NOT_USE_DYNAMIC_NW_TONE_TIMER,
        LOCAL_TONE_WITH_180_BY_FORCE
    };
};

#endif
