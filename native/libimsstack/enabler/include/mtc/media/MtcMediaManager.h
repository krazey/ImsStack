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

#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "helper/ISrvccStateListener.h"
#include "media/IMediaSessionClientListener.h"
#include "media/IMtcMediaManager.h"

class IMediaSession;
class IMediaReportEventListener;
class IMessage;
class IMtcCallContext;
class MediaManager;
class MtcMediaProfileManager;
struct SdpNegotiationResult;

class SessionMedia
{
public:
    inline SessionMedia() :
            objMediaInfo(),
            objOldMediaInfo()
    {
    }
    inline explicit SessionMedia(IN const MediaInfo& objInfo) :
            objMediaInfo(objInfo),
            objOldMediaInfo()
    {
    }
    inline virtual ~SessionMedia() {}
    SessionMedia(IN const SessionMedia&) = delete;
    SessionMedia& operator=(IN const SessionMedia&) = delete;

public:
    inline virtual const MediaInfo& GetMediaInfo() const { return objMediaInfo; }
    inline virtual const MediaInfo& GetOldMediaInfo() const { return objOldMediaInfo; }
    inline virtual void SetMediaInfo(IN const MediaInfo& objInfo)
    {
        objOldMediaInfo = objMediaInfo;
        objMediaInfo = objInfo;
    }
    inline virtual void RestoreMediaInfo() { objMediaInfo = objOldMediaInfo; }

private:
    MediaInfo objMediaInfo;
    MediaInfo objOldMediaInfo;
};

class MtcMediaManager : public IMtcMediaManager, public IMediaSessionClientListener
{
public:
    explicit MtcMediaManager(IN IMtcCallContext& objContext, IN MediaManager& objMediaManager);
    virtual ~MtcMediaManager() override;
    MtcMediaManager(IN const MtcMediaManager&) = delete;
    MtcMediaManager& operator=(IN const MtcMediaManager&) = delete;

public: /* IMediaSessionClientListener */
    void MediaSession_Notify(IN IMS_UINT32 eReportType,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID,
            IN MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = MEDIA_PROTOCOL_ANY) override;

    void MediaSession_NotifyFailures(IN IMS_UINT32 eReportType, IN IMS_SINT32 eError,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

    void MediaSession_NotifyQos(IN IMS_UINTP nNegoId, IN IMS_BOOL bSuccess,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) override;

public:
    void SetMediaReportEventListener(IN IMediaReportEventListener* pListener) override;
    void SetQosListener(IN IMediaQosEventListener* pListener) override;

    /* Media Info */
    void SetMediaInfo(IN const ISession& objISession, IN const MediaInfo& objInfo) override;
    void UpdateMediaDirection(
            IN const ISession& objISession, IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir) override;
    // using enum values defined in MtcDef.h
    const MediaInfo& GetMediaInfo(IN const ISession& objISession) const override;
    void RestoreMediaInfo(IN const ISession& objISession) override;

    /* MediaSession */
    void CreateMediaSession() override;
    void DestroyMediaSession() override;

    /* Media Profile */
    void CreateMediaProfile(
            IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOrigin) override;
    void DestroyMediaForSession(IN ISession* piSession) override;
    void DestroyAllMediaProfiles();  // called when terminate media

    /* Local Tone - public or private */
    void SetLocalTone(IN IMS_BOOL bLocalTone);  // private?
    IMS_BOOL IsLocalTone() override;

    /* Handling SDP */
    IMS_RESULT FormSdp(IN ISession* piSession, IN CallType eCallType,
            IN IMS_BOOL bAnswerForOfferlessReInvite = IMS_FALSE) override;
    SdpNegotiationResult NegotiateSdp(IN ISession* piSession) override;
    void RestoreSdp(IN ISession* piSession) override;
    void FinalizeSdp(IN ISession* piSession) override;

    /* P-Early-Media */
    void UpdatePemType(IN ISession* piSession, IN IMessage* piMessage) override;

    /* Media Operations */
    void Run(IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly) override;

    void SetRtpPort(IN ISession* piSession, IN IMS_UINT32 eMediaType, IN IMS_UINT32 nPort) override;
    IMS_SINT32 GetRemoteRtpPort(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    void SetConferenceCall() override;
    void SetConfirmedSession(IN ISession* piSession) override;

    NegotiationState GetNegotiationState(IN ISession* piSession) override;
    IMS_SINT32 GetNegotiatedDirection(
            IN const ISession* piSession, IN IMS_UINT32 eMediaType) override;
    IMS_SINT32 GetNegotiatedQuality(
            IN const ISession* piSession, IN IMS_UINT32 eMediaType) override;
    CallType GetNegotiatedCallType(IN ISession* piSession) override;

    PemType GetPemType(IN ISession* piSession) override;  // remove..?

    IMS_BOOL IsAudioInactive() override;
    void AdjustDirectionForAutoOffer(
            IN const ISession& objISession, IN CallType eCallType) override;
    void AdjustDirectionForAutoAnswer(IN const ISession& objISession) override;
    void AdjustDirectionForLocalResourceConfirmation(
            IN const ISession& objISession, IN CallType eCallType) override;
    void SetSrvccState(IN SrvccState eState) override;
    IMS_BOOL IsOnHold(IN const ISession& objISession) override;
    IMS_UINT32 GetSupportedMediaTypesFromSdp(IN ISession* piSession) override;
    IMS_BOOL IsPreviewMode(IN ISession* piSession) const override;
    IMS_BOOL IsForkedSession(IN const ISession* piSession) const override;
    void Set180Received() override;

private:
    void UpdateLocalTone(IN const ISession* piSession, IN const IMessage* piMessage,
            IN NegotiationState eNegoState);
    void UpdateLocalTone(IN IMS_BOOL bNetworkToneReceived);
    void SetNetworkToneRtpTimer(IN IMS_UINTP nNegoId, IN IMS_UINT32 nDuration);

    IMS_BOOL IsNecessaryToRunMedia(IN const ISession* piSession, IN const IMessage* piMessage);
    IMS_UINTP GetMediaNegoId(IN const ISession* piSession) const;
    IMS_UINT32 GetWaitingNetworkToneDuration(
            IN const ISession* piSession, IN const IMessage* piMessage);

    static void HandleReceivingMediaDataStarted(IN IMS_UINT32 eMediaType);
    void HandleReceivingNetworkTone(IN IMS_BOOL bNetworkToneReceived);
    IMS_BOOL IsDynamicRbtRequired();
    void SetDirectionToActiveFromInactive(
            IN const ISession& objISession, IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir);

    SessionMedia* GetSessionMedia(IN const ISession& objISession) const;
    void DestroyAllSessionMedia();
    void DestroySessionMedia(IN const ISession& objISession);

    void SetMediaPemType(IN IMS_UINTP nNegoId, IN PemType ePemType);
    AudioCodecAttributes GetNegotiatedAudioCodecAttributes(IN const ISession& objISession) const;
    IMS_BOOL ContainsSendInPem(IN const ISession* piSession) const;

protected:
    MediaManager& m_objMediaManager;
    IMediaReportEventListener* m_pMediaReportListener;
    IMediaQosEventListener* m_pQosListener;
    MtcMediaProfileManager* m_pProfileManager;
    IMtcCallContext& m_objContext;
    ImsMap<const ISession*, SessionMedia*> m_objSessionMedias;
    IMS_BOOL m_bLocalTone;
    IMS_BOOL m_bAudioInactive;
    IMediaSession* m_piMediaSession;
    IMS_BOOL m_b180Received;

    static const IMS_UINT32 TIME_WAIT_NW_TONE_RTP = 1000;
    static const IMS_UINT32 TIME_NO_WAIT_NW_TONE_RTP = 0;
};

#endif
