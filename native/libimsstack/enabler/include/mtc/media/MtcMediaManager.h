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

#ifndef MTC_MEDIA_MANAGER_H_
#define MTC_MEDIA_MANAGER_H_

#include "IMediaSession.h"
#include "IMessage.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "helper/ISrvccStateListener.h"
#include "media/IMediaReportEventListener.h"
#include "media/IMediaSessionClientListener.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaProfileManager.h"

class IMediaManager;

class MtcMediaManager : public IMtcMediaManager, public IMediaSessionClientListener
{
public:
    explicit MtcMediaManager(IN IMtcCallContext& objContext, IN IMediaManager& objMediaManager);
    virtual ~MtcMediaManager();
    MtcMediaManager(IN const MtcMediaManager&) = delete;
    MtcMediaManager& operator=(IN const MtcMediaManager&) = delete;

public: /* IMediaSessionClientListener */
    virtual void MediaSession_Notify(IN IMS_UINT32 eReportType,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID,
            IN MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = MEDIA_PROTOCOL_ANY) override;

    virtual void MediaSession_NotifyFailures(IN IMS_UINT32 eReportType, IN IMS_SINT32 eError,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

    virtual void MediaSession_NotifyQos(IN IMS_UINTP nNegoId, IN IMS_BOOL bSuccess,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

public:
    virtual void SetMediaReportEventListener(IN IMediaReportEventListener* pListener) override;
    virtual void SetQosListener(IN IMediaQosEventListener* pListener) override;

    /* Media Info */
    virtual void SetMediaInfo(IN const MediaInfo& objInfo) override;
    virtual void UpdateMediaDirection(IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir) override;
    // using enum values defined in MtcDef.h
    virtual const MediaInfo& GetMediaInfo() const override;
    virtual void RestoreMediaInfo() override;

    /* MediaSession */
    virtual void CreateMediaSession() override;
    virtual void DestroyMediaSession() override;

    /* Media Profile */
    virtual void CreateMediaProfile(
            IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOrigin) override;
    virtual void DestroyMediaProfile(IN ISession* piSession) override;
    void DestroyAllMediaProfiles();  // called when terminate media

    /* Local Tone - public or private */
    void SetLocalTone(IN IMS_BOOL bLocalTone);  // private?
    virtual IMS_BOOL IsLocalTone() override;

    /* Handling SDP */
    virtual IMS_RESULT FormSdp(IN ISession* piSession, IN CallType eCallType,
            IN IMS_BOOL bAnswerForOfferlessReInvite = IMS_FALSE) override;
    virtual NegotiationResult NegotiateSdp(IN ISession* piSession) override;
    virtual void RestoreSdp(IN ISession* piSession) override;
    void FinalizeSdp(IN ISession* piSession) override;

    /* P-Early-Media */
    virtual void UpdatePemType(IN ISession* piSession, IN IMessage* piMessage) override;

    /* Media Operations */
    virtual void Run(IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly) override;

    virtual void SetRtpPort(
            IN ISession* piSession, IN IMS_UINT32 eMediaType, IN IMS_UINT32 nPort) override;
    IMS_SINT32 GetRemoteRtpPort(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    virtual void SetConferenceCall(IN IMS_BOOL bConference) override;
    virtual void SetConfirmedSession(IN ISession* piSession) override;

    virtual NegotiationState GetNegotiationState(IN ISession* piSession) override;
    virtual IMS_SINT32 GetNegotiatedDirection(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    virtual IMS_SINT32 GetNegotiatedQuality(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    virtual CallType GetNegotiatedCallType(IN ISession* piSession) override;

    virtual PemType GetPemType(IN ISession* piSession) override;  // remove..?

    virtual IMS_BOOL IsAudioInactive() override;
    virtual void AdjustDirectionForAutoAccept(
            IN IMS_BOOL bSendOffer, IN IMS_BOOL bHeldByMe) override;
    void SetSrvccState(IN SrvccState eState) override;
    IMS_BOOL IsOnHold() override;

private:
    void UpdateLocalTone(IN ISession* piSession, IN IMessage* piMessage);
    void UpdateLocalTone(IN ISession* piSession, IN IMS_BOOL bAudioBlocked);
    void SetNetworkToneRtpTimer(IN IMS_UINTP nNegoId, IN IMS_UINT32 nDuration);
    void AdjustDirectionForAutoOffer(IN IMS_BOOL bHeldByMe);
    static void AdjustDirectionForAutoAnswerIfHeldByMe(IN_OUT IMS_SINT32& eDirection);

    IMS_BOOL IsNecessaryToRunMedia(IN ISession* piSession, IN IMessage* piMessage);
    IMS_UINTP GetMediaNegoId(IN ISession* piSession);
    IMS_UINT32 GetWaitingNetworkToneDuration(IN ISession* piSession, IN IMessage* piMessage);

    static void HandleReceivingMediaDataStarted(IN IMS_UINT32 eMediaType);
    void HandleReceivingNetworkTone(IN IMS_BOOL bNetworkToneReceived);
    void RequestToRegisterQosCallback(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eContents);
    IMS_BOOL IsDynamicRbtRequired(IN ISession* piSession);

protected:
    IMediaManager& m_objMediaManager;
    IMediaReportEventListener* m_pMediaReportListener;
    IMediaQosEventListener* m_pQosListener;
    MtcMediaProfileManager* m_pProfileManager;
    IMtcCallContext& m_objContext;
    MediaInfo* m_pMediaInfo;
    MediaInfo* m_pOldMediaInfo;
    IMS_BOOL m_bLocalTone;
    IMS_BOOL m_bAudioInactive;
    IMediaSession* m_piMediaSession;

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
