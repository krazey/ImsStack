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

#ifndef VIDEO_SESSION_H_
#define VIDEO_SESSION_H_

#include <MediaQualityThreshold.h>
#include "BaseSession.h"
#include "video/VideoDef.h"
#include "video/VideoProfile.h"
#include "config/VideoConfiguration.h"

class VideoSession : public BaseSession
{
public:
    enum
    {
        /** The state that video session created and no operation is on going */
        STATE_IDLE = 0,
        /** The state that video session created and openSession is finished */
        STATE_OPENED,
        /** The state that video session created and camera is running in preview mode and no
           rx/rtcp streaming is on going */
        STATE_PREVIEW,
        /** The state that video session is running tx with camera recording streaming, rx
           displaying rtp from the network and rtcp is running by the configuration  */
        STATE_RECORDING,
        /** The state that video session is running with paused image streaming and has the same rx,
           rtcp streaming with STATE_RECORDING */
        STATE_PAUSE_IMAGE,
        /** The state that video session is running same with rx, rtcp streaming with
           STATE_RECORDING but tx streaming is disabled */
        STATE_RENDERING,
        /** The state that video session is running only rtcp streaming */
        STATE_PAUSED,
    };

    static const IMS_SINT32 CAMERA_ID_NONE = -1;

    explicit VideoSession(IN IMS_SINT32 nSlotId = 0);
    virtual ~VideoSession();
    void SetConfig(IN VideoConfiguration* pConfig);

    /**
     * @brief Set VideoConfig for libpixelimsmedia from src/dest/negotiated profile
     * @param pLocalProfile : local profile of the SDP negotiation
     * @param pPeerProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for error, true for successful
     */
    IMS_BOOL UpdateRtpConfig(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            IN VideoProfile* pNegoProfile);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information
     */
    void UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork);

    /**
     * @brief Set MTU size in the VideoConfig
     *
     * @param nMtu : The MTU size to be set to VideoConfig
     */
    void SetMtu(IN IMS_SINT32 nMtu);

    /**
     * @brief Update MediaQualityThreshold parameters and send it to the java
     *
     * @param bActiveSession Set IMS_TRUE if this session is active
     * @param bConfirmedSession Set IMS_TRUE if this session is confirmed session
     * @param bEnableRtcp Set IMS_TRUE to enable monitoring Rtcp inacitivity, IMS_FALSE to disable
     * rtcp monitoring
     * @return IMS_BOOL Returns IMS_TRUE when the sending MediaQualityThreshold is done
     * successfully, IMS_FALSE when it is failed with invalid arguments
     */
    IMS_BOOL UpdateMediaQualityThreshold(
            IN IMS_BOOL bActiveSession, IN IMS_BOOL bConfirmedSession, IN IMS_BOOL bEnableRtcp);

    /**
     * @brief Set the local ip address and port number
     *
     * @param objLocalAddr The local ip address
     * @param nPort The local port number
     */
    void UpdateLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort);

    /**
     * @brief Handles the message from the telecom
     *
     * @param nMsg The message type
     * @param pParam The message parameter
     * @return IMS_BOOL Returns IMS_TRUE when the parameter is valid, IMS_FALSE when it is invalid
     */
    IMS_BOOL OnMessages(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

    /** request OPEN_SESSION with updated VideoConfig */
    IMS_BOOL Open();

    /** request MODIFY_SESSION with updated VideoConfig */
    IMS_BOOL Modify();

    /** request CLOSE_SESSION with updated VideoConfig */
    IMS_BOOL Close();

    /** request SET_MEDIA_QUALITY with Video Media qualityThreshold */
    IMS_BOOL SetMediaQuality();

    /** Get local port number */
    IMS_SINT32 GetLocalPort();

    /** Get remote port number */
    IMS_SINT32 GetRemotePort();

    /** Get camera id */
    IMS_SINT32 GetCameraId();

private:
    IMS_BOOL OnSetSurfaceCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnSelectCameraCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnChangeCameraZoomCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnSetPauseImageCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnChangeOrientation(IN IMS_UINTP pParam);
    IMS_UINT32 convertAvcProfile(IN IMS_UINT32 nProfile);
    IMS_UINT32 convertHevcProfile(IN IMS_UINT32 nProfile);
    IMS_UINT32 convertAvcLevel(IN IMS_UINT32 nLevel);
    IMS_UINT32 convertHevcLevel(IN IMS_UINT32 nLevel);

    VideoConfiguration* m_pConfig;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IpAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    IMS_SINT32 m_nCameraId;
    IMS_SINT32 m_nCameraZoom;
    IMS_BOOL m_bPreviewSurfaceSet;
    IMS_BOOL m_bDisplaySurfaceSet;
};

#endif
