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

#ifndef _AUDIO_CONTROLLER_H_
#define _AUDIO_CONTROLLER_H_

#include "MediaDef.h"
#include "IMediaSessionListener.h"
#include "config/AudioConfiguration.h"
#include "audio/AudioMediaSession.h"
#include "audio/AudioNego.h"

class AudioController
{
public:
    enum AudioUpdateCondition
    {
        EARLY_SESSION = 0,
        READY_TO_CONFIRM,   // session just become confirm
        CONFIRMED_SESSION,  // in confirmed already
    };

    AudioController();
    ~AudioController();
    IMS_BOOL IsHoldSession(IN IMS_UINTP nNegoId);
    IMS_BOOL HoldSession();
    void SetConfirmSession(IN IMS_BOOL bConfirmed);
    IMS_BOOL SendDtmf(IN IMS_UINTP nNegoId, IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration);
    IMS_BOOL CreateSession(
            IN IMediaSessionListener* pListener, IN IMS_UINTP nNegoId, AudioConfiguration* pConfig);
    IMS_BOOL OpenSession(IN IMS_UINTP nNegoId);
    IMS_BOOL UpdateSession(IN IMS_UINTP nNegoId);
    IMS_BOOL AddSession(IN IMS_UINTP nNegoId);
    IMS_BOOL ConfirmSession(IN IMS_UINTP nNegoId);
    IMS_BOOL ModifySession(IN IMS_UINTP nNegoId);
    IMS_BOOL DeleteSession(IN IMS_UINTP nNegoId);
    void CloseSession();
    void UpdateRtpConfig(IN IMS_UINTP nNegoId, IN AudioNego* pNego);
    void UpdateLocalAddress(IN AudioNego* pNego);
    void UpdateQualityThreshold(IN IMS_UINTP nNegoId);

private:
    AudioMediaSession* FindAudioSession(IN IMS_UINTP nNegoId = IMS_NULL);
    void ClearSession();

    IMSList<AudioMediaSession*> m_listAudioSession;
    IMS_SINT32 m_nAudioSessionState;
    IMS_UINT32 m_eUpdateCondition;
    IPAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
};

#endif