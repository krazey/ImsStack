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

#ifndef _IMS_AUDIO_MEDIA_SESSION_H_
#define _IMS_AUDIO_MEDIA_SESSION_H_

// == INCLUDES =============================================================
#include <AudioConfig.h>
#include <MediaQualityThreshold.h>
#include "BaseSession.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfile.h"
#include "config/AudioConfiguration.h"

using namespace android::telephony::imsmedia;

class AudioMediaSession : public BaseSession
{
private:
    AudioMediaSession(IN const AudioMediaSession& obj);
    AudioMediaSession& operator=(IN const AudioMediaSession& obj);

    // == PUBLIC METHOD ==============================================================
public:
    AudioMediaSession(IN IMS_SINT32 nSlodId = 0);
    virtual ~AudioMediaSession();
    void SetConfig(IN AudioConfiguration* pConfig);

    /*
     * Set AudioConfig for libimsmedia from src/dest/negotiated profile
     * @param pSrcProfile : local profile of the SDP negotiation
     * @param pDestProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for error, true for successful
     */
    IMS_BOOL UpdateRtpConfig(IN AudioProfile* pSrcProfile, IN AudioProfile* pDestProfile,
            IN AudioProfile* pNegoProfile);
    IMS_BOOL IsDirectionHold();
    void HoldRtpConfig();
    IMS_BOOL UpdateMediaQualityThreshold(IN IMS_BOOL bIsHold);
    IMS_BOOL UpdateLocalEndPoint(IN AudioProfile* pNegoProfile);
    void UpdateLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort);

    /*
     * request OPEN_SESSION with updated AudioConfig
     */
    IMS_BOOL Open();

    /*
     * request MODIFY_SESSION with updated AudioConfig
     */
    IMS_BOOL Modify();

    /*
     * request ADD_CONFIG with updated AudioConfig
     */
    IMS_BOOL Add();

    /*
     * request DELETE_CONFIG with updated AudioConfig
     */
    IMS_BOOL Delete();

    /*
     * request CONFIRM_CONFIG with updated AudioConfig
     */
    IMS_BOOL Confirm();

    /*
     * request CLOSE_SESSION with updated AudioConfig
     */
    IMS_BOOL Close();

    /*
     * request SET_MEDIA_QUALITY with Audio Media qualityThreshold
     */
    IMS_BOOL SetMediaQuality();
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration);
    // notification - do it later
    //    virtual void SendNotifyToListener(IN IMS_SINT32 nNotify);
    //    virtual void SendNotifyInfoToListener(IMS_SINT32 nEvent, AString strNotifyInfo = IMS_NULL,
    //        IMS_SINT32 nNotifyInfo = -1, IMS_BOOL bNotifyInfo = IMS_FALSE);
    virtual void SendEventToUi(IN IMS_SINT32 nEvent, IN IMS_SINT32 nResult);

protected:
    AudioConfiguration* m_pConfig;
    AudioConfig m_objAudioConfig;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
};

#endif /* End of _IMS_AUDIO_MEDIA_SESSION_H_*/
