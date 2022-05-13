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

#ifndef _IMS_MEDIA_NEGO_H_
#define _IMS_MEDIA_NEGO_H_

#include "ImsSlot.h"
#include "MediaDef.h"
#include "audio/AudioNego.h"
// #include "video/VideoNego.h"
// #include "text/TextNego.h"

class MediaNego : public ImsSlot
{
public:
    enum MediaNegoResult
    {
        NO_ERROR = 0,
        ERROR_INVALID_DESCRIPTOR,
        ERROR_NO_CODEC_MATCHED,
        ERROR_IP_MISMATCH,
        ERROR_NO_AUDIO,
    };
    MediaNego(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ~MediaNego();

private:
    MediaNego(IN const MediaNego& obj);
    MediaNego& operator=(IN const MediaNego& obj);

public:
    void Create(IN MEDIA_SERVICE_TYPE eServiceType);
    void SetMediaEnvironment(IN MediaEnvironment* pMediaEnvironment);
    IMS_BOOL Forking(IN MediaNego* pMediaNego);
    IMS_BOOL FormSDP(OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
            IN IMS_SINT32 eAudioDir, IN IMS_SINT32 eVideoDir, IN IMS_SINT32 eTextDir);
    IMS_BOOL NegotiateSDP(IN ISession* pSession, OUT IMS_SINT32* eAudioDir,
            OUT IMS_SINT32* eVideoDir, OUT IMS_SINT32* eTextDir, OUT MediaNegoResult& errorReason);
    void FinalizeSDP(IN ISession* pSession);
    void SetNegoState(NEGO_STATE eNegoState) { m_eNegoState = eNegoState; }
    NEGO_STATE GetNegoState() { return m_eNegoState; }
    void SetAudioNego(AudioNego* pAudioNego) { m_pAudioNego = pAudioNego; }
    AudioNego* GetAudioNego() { return m_pAudioNego; }
    MEDIA_DIRECTION GetNegotiatedAudioDirection(void);
    MEDIA_DIRECTION GetNegotiatedVideoDirection(void);
    MEDIA_DIRECTION GetNegotiatedTextDirection(void);
    AUDIO_CODEC GetNegotiatedAudioQuality(void);
    IMediaDescriptor* GetMediaDescriptor(IN IMedia* pIMedia);
    void SetActiveProfile(IMS_BOOL bIsActive) { m_bIsActive = bIsActive; }
    IMS_BOOL IsActiveProfile() { return m_bIsActive; };

private:
    IMSList<IMedia*> GetIMediaListFromSession(
            IN ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType);
    void SetSessionType(IN ISession* pSession);
    void SetMediaDescriptorAsNotSupported(IN IMediaDescriptor* pDescriptor, IN SdpMedia* pSDPMedia);
    IMS_BOOL CheckOneWayVideoCall(void);

protected:
    NEGO_STATE m_eNegoState;
    AudioNego* m_pAudioNego;
    // VideoNego* m_pVideoNego;
    // TextNego*  m_pTextNego;
    MediaEnvironment* m_pMediaEnvironment;
    MEDIA_CONTENT_TYPE m_eSessionType;
    IMS_BOOL m_bIsActive;
};

#endif
