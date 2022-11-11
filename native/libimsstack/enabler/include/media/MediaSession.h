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
#include "audio/AudioController.h"
#include "video/VideoController.h"
#include "text/TextController.h"
#include <mutex>

class MediaSession : public IMediaSessionListener, public IMediaSession
{
public:
    /**
     * @brief Construct
     *
     * @param nService The service type of the call session defined as the MEDIA_SERVICE_TYPE
     * @param nCallKey The identification of the call session
     * @param nSlotId The UICC slot id
     */
    MediaSession(IN MEDIA_SERVICE_TYPE nService = MEDIA_SERVICE_DEFAULT, IN IMS_SINTP nCallKey = 0,
            IN IMS_UINT32 nSlotId = 0);
    virtual ~MediaSession();

    /**
     * @brief Get the environment instance
     *
     * @return MediaEnvironment*
     */
    MediaEnvironment* GetEnvironment(void);

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

    virtual void SetMtcListener(IN IMediaSessionClientListener* pISessionListener);
    virtual IMS_BOOL SetEnvironment(IN MediaEnvironment* pEnvironment);
    virtual IMS_UINTP CreateProfile(
            IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO);
    virtual IMS_BOOL DestroyProfile(IN IMS_UINTP nNegoID);
    virtual IMS_BOOL FormSDP(IN IMS_UINTP nNegoID, OUT ISession* pSession,
            IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 nAudioDirection,
            IN IMS_SINT32 nVideoDirection, IN IMS_SINT32 nTextDirection = -1,
            IN IMS_BOOL bEnforceReofferMode = IMS_FALSE);
    virtual IMS_BOOL NegotiateSDP(IN IMS_UINTP nNegoID, IN ISession* pSession,
            OUT IMS_SINT32* nAudioDirection, OUT IMS_SINT32* nVideoDirection,
            OUT IMS_SINT32* nTextDirection, OUT MediaNego::MediaNegoResult& errorReason);
    virtual IMS_BOOL RequestQos(
            IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO);
    virtual void FinalizeSDP(IN IMS_UINTP nNegoID, IN ISession* pSession);
    virtual IMS_BOOL Run(IN IMS_UINTP nNegoID);
    virtual IMS_BOOL Terminate();
    virtual NEGO_STATE GetNegoState(IN IMS_UINTP nNegoID);
    virtual MEDIA_CONTENT_TYPE GetNegotiatedMediaType(IN IMS_UINTP nNegoId);
    virtual IMS_SINT32 GetNegotiatedQuality(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    virtual IMS_SINT32 GetNegotiatedCodecBitrate(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type);
    virtual MEDIA_DIRECTION GetNegotiatedDirection(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType);
    virtual void SetOptions(
            IN IMS_UINTP nNegoId, IN OptionType type, IN IMS_SINT32 param1, IN IMS_SINT32 param2);
    virtual void SetNetworkToneRtpTimer(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer);
    virtual IMS_BOOL NotifySrvccStatus(IN MEDIA_SRVCC_STATUS nStatus);
    virtual IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

protected:
    // for MediaNego
    MediaNego* CreateMediaNego(IN IMS_UINTP nNegoId);
    MediaNego* FindMediaNego(IN IMS_UINTP nNegoId);
    void ConfirmMediaNego(IN IMS_UINTP nNegoId);
    IMS_BOOL DeleteMediaNego(IN IMS_UINTP nNegoId);
    void ClearMediaNego();
    IMSList<ImsMediaMsgQosParam*>* FindQosParam(IN IMS_UINTP nNegoId);
    IMS_BOOL processRequestQos(IN IMS_UINTP nNegoId,
            IN IMSList<ImsMediaMsgQosParam*>* pListQosParams, IN MEDIA_CONTENT_TYPE eMediaType);
    void ClearQosParam();
    // IMediaSessionListener
    virtual IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* param);
    IMS_BOOL IsExistingTypeNode(IN AString strIpAddr, IN IMS_UINT32 nPort);
    IMS_BOOL CreateMediaConfig(IN MEDIA_SERVICE_TYPE eServiceType);
    void SetMediaQuality(IN AudioMediaSession* pAudioSession);
    IMS_BOOL OnMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);
    IMS_BOOL OnResponse(IN IMS_UINTP pParam);
    IMS_BOOL OnNotify(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);
    IMS_BOOL OnSendDtmf(IN IMS_UINTP nParam);
    void ReportToClient(IN IMS_SINT32 eError, IN MEDIA_CONTENT_TYPE eMediaType);
    IMS_BOOL OnChangeNetworkConnection(IN IMS_UINT32 nAccessNetwork);

private:
    IPAddress GetAndroidIP();

protected:
    IMS_UINT32 m_nSlotId;
    IMS_SINTP m_nCallKey;
    IMediaSessionClientListener* m_pClientListener;
    MediaEnvironment* m_pEnvironment;
    IMSMap<IMS_UINTP, MediaNego*> m_objMapMediaNego;
    IMSMap<IMS_UINTP, IMSList<ImsMediaMsgQosParam*>*> m_objMapQosParams;
    AudioController m_objAudioController;
    VideoController m_objVideoController;
    TextController m_objTextController;
    IMS_UINT32 m_nRtpTimer;
    std::mutex m_objMutex;
};
#endif
