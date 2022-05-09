/**
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

#ifndef _INTERFACE_MEDIA_SESSION_H_
#define _INTERFACE_MEDIA_SESSION_H_

#include "ImsMessage.h"
#include "AString.h"
#include "IMediaSessionClientListener.h"
#include "IUMedia.h"
#include "ISession.h"
#include "IMSTypeDef.h"
#include "MediaEnvironment.h"
#include "MediaDef.h"
#include "MediaNego.h"
#include "audio/AudioDef.h"
#include "video/VideoDef.h"
// #include "text/TextDef.h"

class IMediaSession
{
public:
    enum OptionType
    {
        SET_RTP_PORT = 0,
        SET_DIRECTION,
        SET_CONFIRMED_SESSION,
        SET_DRA_REPORT_OPTION,
        SET_CONFERENCE_ENABLE,
        SET_CVO_SUPPORT,
        SEND_FAST_VIDEO_UPDATE,
    };

    // -- Set Callback ----------------------------------------------------------------------------
    virtual void SetMtcListener(IN IMediaSessionClientListener* pISessionListener);

    // -- Environment Setting ---------------------------------------------------------------------
    virtual IMS_BOOL SetEnvironment(IN MediaEnvironment* pEnvironment);

    // -- Negotiation APIs ------------------------------------------------------------------------
    virtual IMS_UINTP CreateProfile(
            IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO) = 0;
    virtual IMS_BOOL DestroyProfile(IN IMS_UINTP nNegoID) = 0;
    virtual IMS_BOOL FormSDP(IN IMS_UINTP nNegoID, OUT ISession* pSession,
            IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 eAudioDir, IN IMS_SINT32 eVideoDir,
            IN IMS_SINT32 eTextDir = -1) = 0;
    virtual IMS_BOOL NegotiateSDP(IN IMS_UINTP nNegoID, IN ISession* pSession,
            OUT IMS_SINT32* eAudioDir, OUT IMS_SINT32* eVideoDir, OUT IMS_SINT32* eTextDir,
            OUT MediaNego::MediaNegoResult& errorReason) = 0;
    virtual void FinalizeSDP(IN IMS_UINTP nNegoID, IN ISession* pSession) = 0;

    //-- Additional Negotiation APIs --------------------------------------------------------------

    // -- Operation APIs --------------------------------------------------------------------------
    virtual IMS_BOOL Run(IN IMS_UINTP nNegoID) = 0;
    virtual IMS_BOOL Terminate() = 0;

    //-- Additional Operation APIs-----------------------------------------------------------------

    // -- Condition checking APIs
    // -------------------------------------------------------------------------
    virtual NEGO_STATE GetNegoState(IN IMS_UINTP nNegoID) = 0;
    virtual MEDIA_CONTENT_TYPE GetNegotiatedMediaType(IN IMS_UINTP nNegoId);
    // virtual AString GetNegotiatedCodec(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    virtual IMS_SINT32 GetNegotiatedQuality(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    virtual IMS_SINT32 GetNegotiatedCodecBitrate(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    // virtual IMS_SINT32 GetNegotiatedCodecBandwidth(IN IMS_UINTP nNegoId,
    //         IN MEDIA_CONTENT_TYPE type);
    virtual MEDIA_DIRECTION GetNegotiatedDirection(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType);

    // -- Additional function APIs
    // -------------------------------------------------------------------------
    virtual void SetOptions(
            IN IMS_UINTP nNegoId, IN OptionType type, IN IMS_SINT32 param1, IN IMS_SINT32 param2);
    virtual void SetNetworkToneRTPTimer(
            IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer) = 0;
    virtual IMS_BOOL SendMessage(IN IMSMSG& objMSG) = 0;
    virtual IMS_BOOL SendDtmf(
            IN IMS_UINTP nNegoId, IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration) = 0;

    // -- WILL BE DELETED -------------------------------------------------------------------------
    // virtual MEDIA_SERVICE_TYPE GetMediaServiceType() = 0;
    // virtual void SetMediaServiceType(IN MEDIA_SERVICE_TYPE nService) = 0;
    // virtual void SetSessionId(IMS_UINTP nSessionID) = 0;
    // virtual IMS_UINTP GetSessionID() = 0;
    // virtual void SetHandlerListener(IN IMediaSessionHandlerListener*
    // pIMediaSessionHandlerListener) = 0; virtual IMS_BOOL    UpdateServiceType(IN
    // MEDIA_SERVICE_TYPE eServiceType) = 0; virtual void        SetAnalyzer(IN
    // IAnalyzerCallSession* pAnalyzer) = 0; virtual IMS_BOOL SetCvoSupportValue(IN IMS_UINTP
    // nNegoID, IN IMS_BOOL bIsSupportCvoMode) = 0; virtual IMS_BOOL    PauseVT() = 0; virtual
    // IMS_BOOL    PreviewStart(IN IMS_UINTP nNegoID, IN IMS_UINTP nSurface, IN IMS_UINT32
    // nCameraId) = 0; virtual IMS_BOOL    PreviewStop(IN IMS_UINTP nNegoID) = 0; virtual IMS_BOOL
    // IsRunning(IMS_UINTP nNegoID) = 0;

    // virtual IMS_BOOL    IsTerminated(IMS_UINTP nNegoID) = 0;
    // virtual IMS_BOOL                HasNegotiatedDTMF(IN IMS_UINTP nNegoID) = 0;
    // virtual IMS_BOOL                CheckOneWayVideoCall(IN IMS_UINTP nNegoID) = 0;
    // virtual AUDIO_CODEC             GetNegotiatedAudioQuaility(IN IMS_UINTP nNegoID) = 0;
    // virtual VIDEO_RESOLUTION        GetNegotiatedVideoQuaility(IN IMS_UINTP nNegoID) = 0;
    // virtual TEXT_CODEC              GetNegotiatedTextQuaility(IN IMS_UINTP nNegoID) = 0;
    // virtual MEDIA_DIRECTION         GetNegotiatedAudioDirection(IN IMS_UINTP nNegoID) = 0;
    // virtual MEDIA_DIRECTION         GetNegotiatedVideoDirection(IN IMS_UINTP nNegoID) = 0;
    // virtual MEDIA_DIRECTION         GetNegotiatedTextDirection(IN IMS_UINTP nNegoID) = 0;
    // virtual AUDIO_CODEC_BITRATE     GetNegotiatedAudioCodecRate(IN IMS_UINTP nNegoID) = 0;
    // virtual IMS_BOOL    SetRTPPort(IN IMS_UINTP nNegoID, IN IMS_UINT32 nAudioPort,
    //         IN IMS_UINT32 nVideoPort,  IN IMS_UINT32 nTextPort = -1) = 0;
    // virtual void        SetEnforcedDirection(IN MEDIA_DIRECTION eDir) = 0;
    // virtual IMS_BOOL    Start(IN IMS_UINTP nNegoID, IN IMS_SINT32 eType = START_LIVE, IN IMS_BOOL
    // bRTPMonitor = IMS_TRUE) = 0; virtual IMS_BOOL    Start(IN IMS_UINTP nNegoID, IN IMS_SINT32
    // eType = START_LIVE, IN MEDIA_TRANSPORT_PROTOCOL eTimeoutCheckType = MEDIA_PROTOCOL_ANY,
    //     IN IMS_SINT32 nMonitoringTimer = -1, IN MEDIA_CONTENT_TYPE eTimeoutCheckMedia =
    //     MEDIA_TYPE_AUDIOVIDEOTEXT) = 0;
    // virtual IMS_BOOL    Update(IN IMS_UINTP nNegoID, IN IMS_SINT32 eType = START_LIVE, IN
    // IMS_BOOL bRTPMonitor = IMS_TRUE) = 0; virtual IMS_BOOL    Update(IN IMS_UINTP nNegoID, IN
    // IMS_SINT32 eType = START_LIVE, IN MEDIA_TRANSPORT_PROTOCOL eTimeoutCheckType =
    // MEDIA_PROTOCOL_ANY,
    //    IN IMS_SINT32 nMonitoringTimer = -1, IN MEDIA_CONTENT_TYPE eTimeoutCheckMedia =
    //    MEDIA_TYPE_AUDIOVIDEOTEXT) = 0;
    // virtual IMS_BOOL    ResumeVT() = 0;
    // virtual IMS_BOOL    SendFastVideoUpdate(void) = 0; //it need to only active video session.
    // virtual IMS_BOOL    SendDTMF(IN IMS_CHAR* strSignal, IN IMS_SINT32 nDuration) = 0; //it need
    // to only active audio session. virtual void        ResetRTPTimer(void) = 0; virtual void
    // SetDRAStatsReportOption(IN IMS_UINT32 nAction) = 0; virtual void SetConfirmedSession(IN
    // IMS_BOOL bIsConfirmed) = 0; // for RTCP-Bye packet at only Confirmed Session Stop virtual
    // void SetIsConf(IN IMS_BOOL bIsConf = IMS_FALSE) = 0; // for LGU Video Confrence Call Surface
    // Destroy Indication

    // -- WOULD BE DELETED ------------------------------------------------------------------------
    // virtual IMS_BOOL    IsPreview(void) = 0;
    // virtual IMS_BOOL    SetGTTMode(IN IMS_SINT32 nGTTMode = -1) = 0;

    // -- WILL Be Enabled on Phase2 ---------------------------------------------------------------

    // -- WILL Be Enabled on Phase3 ---------------------------------------------------------------
    // virtual IMS_BOOL GetNegotiatedCvoResult(IN IMS_UINTP nNegoID) = 0;

    // -- WILL Be Enabled on Q3 -------------------------------------------------------------------
};

#endif
