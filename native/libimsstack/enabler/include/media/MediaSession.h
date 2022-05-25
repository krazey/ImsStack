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

#ifndef _MEDIA_SESSION_H_
#define _MEDIA_SESSION_H_

#include "IMediaSession.h"
#include "IMediaSessionClientListener.h"
#include "MediaNego.h"
#include "audio/AudioMediaSession.h"

class MediaSessionTypeNode
{
public:
    AString m_strIpAddr;
    IMS_SINT32 m_nPort;
    IMS_UINTP m_nNegoId;
    MEDIA_CONTENT_TYPE m_eMediaType;

public:
    MediaSessionTypeNode() :
            m_strIpAddr(AString::ConstNull()),
            m_nPort(0),
            m_nNegoId(0),
            m_eMediaType(MEDIA_TYPE_INVALID){};

    MediaSessionTypeNode(IN AString strIpAddr, IN IMS_SINT32 nPort, IN IMS_UINTP nNegoId,
            IN MEDIA_CONTENT_TYPE eMediaType) :
            m_strIpAddr(strIpAddr),
            m_nPort(nPort),
            m_nNegoId(nNegoId),
            m_eMediaType(eMediaType){};
};

class MediaSession : public IMediaSessionListener, public IMediaSession
{
public:
    enum SessionState
    {
        EARLY_SESSION = 0,
        READY_TO_CONFIRM,   // session just become confirm
        CONFIRMED_SESSION,  // in confirmed already
    };

    enum MediaSessionMsgType
    {
        RUN = 0,
        STOP,
    };

    MediaSession();
    MediaSession(IN MEDIA_SERVICE_TYPE nService, IN IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId);
    virtual ~MediaSession();
    // set Callback

    // SDP negotiation APIs ------------------------------------------------------------------------

    // operation APIs --------------------------------------------------------------------------
    IMS_BOOL ProcessStop(IN IMS_UINTP nNegoId = 0);
    // getter
    MediaEnvironment* GetEnvironment(void);
    MEDIA_NETWORK_TYPE GetNetworkType(void);
    IMS_BOOL GetDTMFEnabled(IN IMS_UINTP nNegoId);
    // DTMF

    /* Set Optional operation */

    //-- MediaService API

    // interface to ImsService
    void SendImsMediaRequest();
    virtual void OnMediaResponse();
    // nofity from session
    void OnNotify(IN IMS_UINT32 eReportType, IN MEDIA_CONTENT_TYPE eMediaType);
    IMS_BOOL IsHoldSession(IN IMS_UINTP nNegoId);
    IMS_BOOL HoldSession();
    void ReportToClient(IN RtpError eError, IN MEDIA_CONTENT_TYPE eMediaType);
    IMS_SINTP GetCallKey() { return m_nCallKey; };

public:
    /************************************************************/
    /******  IMediaSession  *************************************/
    /************************************************************/

    // -- Set Callback ----------------------------------------------------------------------------
    virtual void SetMtcListener(IN IMediaSessionClientListener* pISessionListener);

    // -- Environment Setting ---------------------------------------------------------------------
    virtual IMS_BOOL SetEnvironment(IN MediaEnvironment* pEnvironment);

    // -- Negotiation APIs ------------------------------------------------------------------------
    virtual IMS_UINTP CreateProfile(
            IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO);
    virtual IMS_BOOL DestroyProfile(IN IMS_UINTP nNegoID);
    virtual IMS_BOOL FormSDP(IN IMS_UINTP nNegoID, OUT ISession* pSession,
            IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 eAudioDir, IN IMS_SINT32 eVideoDir,
            IN IMS_SINT32 eTextDir = -1);
    virtual IMS_BOOL NegotiateSDP(IN IMS_UINTP nNegoID, IN ISession* pSession,
            OUT IMS_SINT32* eAudioDir, OUT IMS_SINT32* eVideoDir, OUT IMS_SINT32* eTextDir,
            OUT MediaNego::MediaNegoResult& errorReason);
    virtual void FinalizeSDP(IN IMS_UINTP nNegoID, IN ISession* pSession);

    //-- Additional Negotiation APIs --------------------------------------------------------------

    // -- Operation APIs --------------------------------------------------------------------------
    virtual IMS_BOOL Run(IN IMS_UINTP nNegoID);
    virtual IMS_BOOL Terminate();

    // -- Additional Operation APIs----------------------------------------------------------------

    // -- Condition checking APIs -----------------------------------------------------------------
    virtual NEGO_STATE GetNegoState(IN IMS_UINTP nNegoID);
    virtual MEDIA_CONTENT_TYPE GetNegotiatedMediaType(IN IMS_UINTP nNegoId);
    // virtual AString GetNegotiatedCodec(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    virtual IMS_SINT32 GetNegotiatedQuality(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    virtual IMS_SINT32 GetNegotiatedCodecBitrate(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    // virtual IMS_SINT32 GetNegotiatedCodecBandwidth(IN IMS_UINTP nNegoId,
    //         IN MEDIA_CONTENT_TYPE type);
    virtual MEDIA_DIRECTION GetNegotiatedDirection(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType);

    // -- Additional function APIs ----------------------------------------------------------------
    virtual void SetOptions(
            IN IMS_UINTP nNegoId, IN OptionType type, IN IMS_SINT32 param1, IN IMS_SINT32 param2);
    virtual void SetNetworkToneRTPTimer(IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer);
    virtual IMS_BOOL SendMessage(IN IMSMSG& objMsg);
    virtual IMS_BOOL SendDtmf(IN IMS_UINTP nNegoId, IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration);

private:
    IMS_BOOL CreateMediaConfig(IN MEDIA_SERVICE_TYPE eServiceType);
    void UpdateRtpConfig(IN IMS_UINTP nNegoId);
    void UpdateAudioRtpConfig(IN IMS_UINTP nNegoId);
    void HoldRtpConfig();
    void HoldAudioRtpConfig();
    void UpdateMediaQualityThreshold(IN IMS_UINTP nNegoId);
    void UpdateAudioQualityThreshold(IN IMS_UINTP nNegoId);
    void UpdateMediaQualityThresholdForHold();
    void UpdateAudioQualityThresholdForHold();

protected:
    IMS_UINTP CreateMediaNego(IN IMS_UINTP nNegoId);
    IMS_UINTP CreateAudioMediaSession(IN IMS_UINTP nNegoId);
    MEDIA_CONTENT_TYPE GetSessionTypeFromMsg(IN IMS_SINT32 nMsg);
    MediaNego* FindMediaNego(IN IMS_UINTP nNegoId);
    AudioMediaSession* FindAudioSession(IN IMS_UINTP nNegoId);
    void ConfirmAudioSession(IN IMS_UINTP nNegoId);
    void ConfirmMediaNego(IN IMS_UINTP nNegoId);
    void ConfirmAudioMediaSession(IN IMS_UINTP nNegoId);
    void ConfirmMediaSessionTypeNode(IN IMS_UINTP nNegoId);
    IMS_BOOL DeleteMediaNego(IN IMS_UINTP nNegoId);
    IMS_BOOL DeleteAudioMediaSession(IN IMS_UINTP nNegoId);
    IMS_BOOL DeleteMediaSessionTypeNode(IN IMS_UINTP nNegoId);
    void ClearMediaNego();
    void ClearAudioMediaSession();
    void ClearMediaSessionTypeNode();
    void SetAllMediaNegoActiveProfile(IN IMS_BOOL bIsActive);
    IMS_BOOL SetActiveProfile(IMS_UINTP nNegoId);
    void ProcessOfferSdp(IN IMS_UINTP nNegoId, IN IMS_UINT32 nReUsed);
    void ProcessAnswerSdp(IN IMS_UINTP nNegoId);
    IMS_BOOL ProcessRun(IN IMS_UINTP nNegoId);
    void UpdateLocalAddress(IN IMS_UINTP nNegoId);

    // IMediaSessionListener
    virtual void MediaSession_SendEventToUi(IMS_SINT32 nEvent, IMS_SINT32 nResult);
    virtual IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* param);
    void CreateMediaSessionTypeNode(IN IMS_UINTP nNegoId, IN ISession* pSession);
    IMS_BOOL IsExistingTypeNode(IN AString strIpAddr, IN IMS_UINT32 nPort);
    IMS_BOOL OnResponseOpenSession(IN IMSMSG& objMsg);
    IMS_BOOL OnResponseModifySession(IN IMSMSG& objMsg);
    IMS_BOOL OnResponseAddConfig(IN IMSMSG& objMsg);
    IMS_BOOL OnResponseConfirmConfig(IN IMSMSG& objMsg);
    IMS_BOOL OnNotifyFirstPacket(IN IMSMSG& objMsg);
    IMS_BOOL OnNotifyMediaInactivity(IN IMSMSG& objMsg);
    IMS_BOOL OnNofityPacketLoss(IN IMSMSG& objMsg);
    IMS_BOOL OnNofityJitter(IN IMSMSG& objMsg);
    IMS_BOOL OnNofityMediaQualityChange(IN IMSMSG& objMsg);
    IMS_BOOL OnResponseSessionChanged(IN IMSMSG& objMsg);
    IMS_BOOL OnNofityHeaderExtension(IN IMSMSG& objMsg);
    IMS_BOOL OnNotifyQosInfo(IN IMSMSG& objMsg);
    ImsMediaBasicSessionInfoParam* GetBasicSessionInfofromRemoteArress(
            IN AString strIpAddr, IN IMS_SINT32 nPort);
    IMS_UINTP GetNegoIdfromRemoteAddress(IN AString strIpAddr, IN IMS_SINT32 nPort);

protected:
    IMS_UINT32 m_nSlotId;
    IMS_SINTP m_nCallKey;
    IMediaSessionClientListener* m_pClientListener;
    MediaEnvironment* m_pEnvironment;
    IMS_BOOL m_bSessionOpened;
    IMS_SINT32 m_nCommandBuffer;
    SessionState m_eSessionState;
    IMSMap<IMS_UINTP, MediaNego*> m_objMapMediaNego;
    IMSList<AudioMediaSession*> m_listAudioSession;
    IMS_UINT32 m_nRtpTimer;
    IMSList<MediaSessionTypeNode*> m_listMediaSessionTypeNode;
};
#endif
