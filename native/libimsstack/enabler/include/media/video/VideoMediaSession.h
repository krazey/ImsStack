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

#ifndef _IMS_VIDEO_MEDIA_SESSION_H_
#define _IMS_VIDEO_MEDIA_SESSION_H_

// == INCLUDES =============================================================
#include <MediaQualityThreshold.h>
#include <VideoConfig.h>
#include "BaseSession.h"
#include "video/VideoDef.h"
#include "video/VideoProfile.h"
#include "config/VideoConfiguration.h"

using namespace android::telephony::imsmedia;

class VideoMediaSession : public BaseSession
{
public:
    enum
    {
        STATE_IDLE = 0,
        STATE_PREVIEW,
        STATE_RECORDING,
        STATE_PAUSE_IMAGE,
        STATE_RENDERING,
        STATE_PAUSED,
    };

    static const IMS_SINT32 CAMERA_ID_NONE = 99;

    VideoMediaSession(IN IMS_SINT32 nSlodId = 0);
    virtual ~VideoMediaSession();
    void SetConfig(IN VideoConfiguration* pConfig);

    /*
     * Set VideoConfig for libimsmedia from src/dest/negotiated profile
     * @param pSrcProfile : local profile of the SDP negotiation
     * @param pDestProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for error, true for successful
     */
    IMS_BOOL UpdateRtpConfig(IN VideoProfile* pSrcProfile, IN VideoProfile* pDestProfile,
            IN VideoProfile* pNegoProfile);
    IMS_BOOL IsDirectionHold();
    void HoldRtpConfig();
    IMS_BOOL UpdateMediaQualityThreshold(IN IMS_BOOL bIsHold, IN VideoProfile* pVideoProfile);
    IMS_BOOL UpdateLocalEndPoint(IN VideoProfile* pNegoProfile);
    void UpdateLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort);
    IMS_BOOL OnVideoMessages(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

    /*
     * request OPEN_SESSION with updated VideoConfig
     */
    IMS_BOOL Open();

    /*
     * request MODIFY_SESSION with updated VideoConfig
     */
    IMS_BOOL Modify();

    /*
     * request CLOSE_SESSION with updated VideoConfig
     */
    IMS_BOOL Close();

    /*
     * request SET_MEDIA_QUALITY with Video Media qualityThreshold
     */
    IMS_BOOL SetMediaQuality();
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration);
    // notification - do it later
    //    virtual void SendNotifyToListener(IN IMS_SINT32 nNotify);
    //    virtual void SendNotifyInfoToListener(IMS_SINT32 nEvent, AString strNotifyInfo = IMS_NULL,
    //        IMS_SINT32 nNotifyInfo = -1, IMS_BOOL bNotifyInfo = IMS_FALSE);
    virtual void SendEventToUi(IN IMS_SINT32 nEvent, IN IMS_SINT32 nResult);
    IMS_SINT32 GetLocalPort();
    IMS_SINT32 GetRemotePort();
    IMS_SINT32 GetCameraId();

private:
    IMS_BOOL OnSetSurfaceCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnSelectCameraCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnChangeCameraZoomCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnSetPauseImageCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnChangeOrientation(IN IMS_UINTP pParam);
    void SetStateFromVideoMode(IN IMS_SINT32 mode);

protected:
    VideoConfiguration* m_pConfig;
    VideoConfig m_objVideoConfig;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    IMS_SINT32 m_nCameraId;
    IMS_SINT32 m_nCameraZoom;
    IMS_BOOL m_bPreviewSurfaceSet;
    IMS_BOOL m_bDisplaySurfaceSet;
};

#endif /* End of _IMS_VIDEO_MEDIA_SESSION_H_*/
