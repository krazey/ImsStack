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

#ifndef MEDIA_SESSION_H_
#define MEDIA_SESSION_H_

#include <mutex>

#include "IMediaSession.h"
#include "IMediaSessionListener.h"
#include "IMediaNetworkConnectionListener.h"
#include "MediaDef.h"

class IService;
class AudioSession;
class AudioController;
class MediaNego;
class MediaNegoHandler;
class MediaNetworkConnectionWatcher;
class TextController;
class VideoController;

class MediaSession :
        public IMediaSessionListener,
        public IMediaSession,
        public IMediaNetworkConnectionListener
{
public:
    /**
     * @brief Constructor
     *
     * @param eNetwork The network type @link{MEDIA_NETWORK_TYPE}
     * @param eServiceType The service type of the call session defined as the
     * @link{MEDIA_SERVICE_TYPE}
     * @param pIService The interface to access the IMS services
     * @param nCallKey The identification of the call session
     * @param nSlotId The UICC slot id
     */
    explicit MediaSession(MEDIA_NETWORK_TYPE eNetwork, MEDIA_SERVICE_TYPE eServiceType,
            IService* pIService, IN IMS_SINTP nCallKey = 0, IN IMS_UINT32 nSlotId = 0);
    virtual ~MediaSession() override;

    /**
     * @brief Get the current connected network type
     *
     * @return MEDIA_NETWORK_TYPE
     */
    MEDIA_NETWORK_TYPE GetNetworkType(void);

    /**
     * @brief Get the call key to identify the call session
     *
     * @return IMS_SINTP The session key
     */
    IMS_SINTP GetCallKey() { return m_nCallKey; };

    /**
     * @brief Set the MediaNegoHandler object
     */
    void SetMediaNegoHandler(std::shared_ptr<MediaNegoHandler> pMediaNegoHandler);

    /**
     * @brief Set the AudioController object
     */
    void SetAudioController(std::shared_ptr<AudioController> pAudioController);

    /**
     * @brief Set the VideoController object
     */
    void SetVideoController(std::shared_ptr<VideoController> pVideoController);

    /**
     * @brief Set the TextController object
     */
    void SetTextController(std::shared_ptr<TextController> pTextController);

    // IMediaSession
    void SetMtcListener(IN IMediaSessionClientListener* pISessionListener) override;
    IMS_UINTP CreateProfile(
            IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIO) override;
    IMS_BOOL DestroyProfile(IN IMS_UINTP nNegoID) override;
    IMS_BOOL FormSdp(IN IMS_UINTP nNegoID, OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eAudioDirection, IN MEDIA_DIRECTION eVideoDirection,
            IN MEDIA_DIRECTION eTextDirection = MEDIA_DIRECTION_INVALID,
            IN IMS_BOOL bEnforceReofferMode = IMS_FALSE) override;
    virtual MEDIA_CONTENT_TYPE GetSupportedMediaTypesFromSdp(
            IN IMS_UINTP nNegoId, IN ISession* pSession) override;
    SdpNegotiationResult NegotiateSdp(IN IMS_UINTP nNegoId, IN ISession* pSession) override;
    IMS_BOOL RequestQos(
            IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIO) override;
    void FinalizeSdp(IN IMS_UINTP nNegoId, IN ISession* pSession) override;
    IMS_BOOL Run(IN IMS_UINTP nNegoID) override;
    IMS_BOOL Terminate() override;
    NEGO_STATE GetNegoState(IN IMS_UINTP nNegoID) override;
    MEDIA_CONTENT_TYPE GetNegotiatedMediaType(IN IMS_UINTP nNegoId) override;
    IMS_SINT32 GetNegotiatedQuality(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) override;
    IMS_SINT32 GetNegotiatedCodecBitrate(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) override;
    IMS_FLOAT GetNegotiatedCodecBitrateKbps(IN IMS_UINTP nNegoId) override;
    IMS_FLOAT GetNegotiatedCodecBandwidthKhz(IN IMS_UINTP nNegoId) override;
    void GetNegotiatedCodecBitrateRange(IN IMS_UINTP nNegoId, OUT IMS_FLOAT& nBitrateStart,
            OUT IMS_FLOAT& nBitrateEnd) override;
    void GetNegotiatedCodecBandwidthRange(IN IMS_UINTP nNegoId, OUT IMS_FLOAT& nBandwidthStart,
            OUT IMS_FLOAT& nBandwidthEnd) override;
    IMS_SINT32 GetRemotePort(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) override;
    MEDIA_DIRECTION GetNegotiatedDirection(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType) override;
    void SetOptions(IN IMS_UINTP nNegoId, IN OptionType type, IN IMS_SINT32 param1,
            IN IMS_SINT32 param2) override;
    void SetNetworkToneRtpTimer(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType, IN IMS_UINT32 nRtpTimer) override;
    IMS_BOOL NotifySrvccStatus(IN MEDIA_SRVCC_STATUS nStatus) override;
    IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam) override;
    void SetMediaPemType(IN IMS_UINTP nNegoId, IN MEDIA_PEM_TYPE ePemType) override;
    IMS_BOOL IsPreviewMode(IMS_UINTP nNegoId) override;

    /* IMediaNetworkConnectionListener Interface Impl */
    void OnNetworkConnectionChanged(IN const IMS_SINT32 nRatType) override;
    void OnMediaMtuChanged(IN const IMS_UINT32 nMtu) override;

protected:
    QosRequestParam* FindQosParam(const QosRequestParam* param);
    virtual QosRequestParam* createQosParam(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType);
    void ClearQosParam();
    // IMediaSessionListener
    IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* param) override;
    IMS_BOOL MediaSession_NotifyToClient(IMS_UINT32 eReportType,
            MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_INVALID,
            MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = MEDIA_PROTOCOL_ANY) override;
    IMS_BOOL IsExistingTypeNode(IN AString strIpAddr, IN IMS_UINT32 nPort);
    IMS_BOOL CreateMediaConfig(IN MEDIA_SERVICE_TYPE eServiceType);
    void SetMediaQuality(IN AudioSession* pAudioSession);
    IMS_BOOL OnMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);
    IMS_BOOL OnResponse(IN IMS_UINTP pParam);
    IMS_BOOL OnNotify(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);
    IMS_BOOL OnSendDtmf(IN IMS_UINTP nParam);
    void ReportToClient(IN IMS_SINT32 eError, IN MEDIA_CONTENT_TYPE eType);
    IMS_BOOL OnChangeNetworkConnection(IN IMS_UINTP pParam);
    IMS_BOOL OnMediaMtuChanged();
    IMS_BOOL OnNotifyAnbrReceived(IN IMS_UINTP nParam);
    void RequestQosParam(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType);
    void ReleaseQosParam(IN MEDIA_CONTENT_TYPE eType);

private:
    IMS_BOOL HandleNotifyMediaInactivity(IN IMS_UINTP nParam);
    IMS_BOOL IsInactivityTimerExpired(IN IMS_SINT32 nRunningTimerValue, IN IMS_SINT32 nTimerValue);
    void CreateMediaSessions(IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eType);
    void OpenMediaSessions(IN IMS_UINTP nNegoId, IN std::shared_ptr<MediaNego> pMediaNego,
            MEDIA_CONTENT_TYPE eType);
    void UpdateMediaSessions(IN IMS_UINTP nNegoId, IN std::shared_ptr<MediaNego> pMediaNego,
            MEDIA_CONTENT_TYPE eType);
    void CloseMediaSessions(MEDIA_CONTENT_TYPE eType);
    /** Get the rtp fragment size from mtu */
    IMS_SINT32 GetRtpFragmentSize();

protected:
    IMS_UINT32 m_nSlotId;
    IMS_UINT32 m_nCurrentAccessNetwork;
    IMS_SINTP m_nCallKey;
    IMediaSessionClientListener* m_pClientListener;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    std::shared_ptr<MediaNegoHandler> m_pMediaNegoHandler;
    ImsList<QosRequestParam*> m_objListQosParams;
    std::shared_ptr<AudioController> m_pAudioController;
    std::shared_ptr<VideoController> m_pVideoController;
    std::shared_ptr<TextController> m_pTextController;
    IMS_BOOL m_bSessionConfirmed;
    std::mutex m_objMutex;
    MEDIA_CONTENT_TYPE m_eCurMediaType;
    IMS_BOOL m_bIsConference;
    std::shared_ptr<MediaNetworkConnectionWatcher> m_pNetworkConnectionWatcher;
};

#endif
