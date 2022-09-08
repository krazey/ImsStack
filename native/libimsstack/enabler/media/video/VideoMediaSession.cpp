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

#include <stdio.h>
#include <VideoConfig.h>
#include "ISessionDescriptor.h"
#include "Configuration.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceNetwork.h"
#include "ServiceEvent.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "video/VideoMediaSession.h"
#include "MediaManager.h"
#include "IMMedia.h"

__IMS_TRACE_TAG_USER_DECL__("MED.VS");

PUBLIC VideoMediaSession::VideoMediaSession(IN IMS_SINT32 nSlotId) :
        BaseSession(nSlotId),
        m_pConfig(IMS_NULL),
        m_objConfig(VideoConfig()),
        m_objMediaQualityThreshold(MediaQualityThreshold()),
        m_objLocalAddress(IPAddress::IPv6NONE),
        m_nLocalPort(0),
        m_nCameraId(CAMERA_ID_NONE),
        m_nCameraZoom(-1),
        m_bPreviewSurfaceSet(IMS_FALSE),
        m_bDisplaySurfaceSet(IMS_FALSE),
        m_nPrevDirection(-1)
{
    IMS_TRACE_I("+VideoMediaSession()", 0, 0, 0);
}

PUBLIC VIRTUAL VideoMediaSession::~VideoMediaSession()
{
    IMS_TRACE_I("~VideoMediaSession() - state[%d]", GetState(), 0, 0);
}

PUBLIC void VideoMediaSession::SetConfig(VideoConfiguration* pConfig)
{
    m_pConfig = pConfig;
}

PUBLIC IMS_BOOL VideoMediaSession::UpdateRtpConfig(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN VideoProfile* pNegoProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pNegoProfile->lstPayload.GetSize() == 0 || pPeerProfile->lstPayload.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    // Get Negotiated Payload from negotiated Payload index...
    VideoProfile::Payload* pDestPayload;
    VideoProfile::Payload* pNegoPayload;

    IMS_TRACE_D("UpdateRtpConfig() - nNegotiated nPeerPayloadIndex[%d], nNegoPayloadIndex[%d]",
            pPeerProfile->nNegotiatedPayloadIndex, pNegoProfile->nNegotiatedPayloadIndex, 0);

    if (pNegoProfile->nNegotiatedPayloadIndex < 0)
    {
        pNegoPayload = pNegoProfile->lstPayload.GetAt(0);
    }
    else
    {
        pNegoPayload = pNegoProfile->lstPayload.GetAt(pNegoProfile->nNegotiatedPayloadIndex);
    }

    if (pPeerProfile->nNegotiatedPayloadIndex < 0)
    {
        pDestPayload = pPeerProfile->lstPayload.GetAt(0);
    }
    else
    {
        pDestPayload = pPeerProfile->lstPayload.GetAt(pPeerProfile->nNegotiatedPayloadIndex);
    }

    if (pNegoPayload == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Setting the network properties
    UpdateLocalEndPoint(pNegoProfile);

    if (pLocalProfile->nNegotiatedPayloadIndex < 0)
    {
        m_objConfig.setTxPayloadTypeNumber((int32_t)pNegoPayload->objRtpMap.nPayloadNum);
    }
    else
    {
        VideoProfile::Payload* pLocalPayload =
                pLocalProfile->lstPayload.GetAt(pLocalProfile->nNegotiatedPayloadIndex);

        if (pLocalPayload == IMS_NULL)
        {
            return IMS_FALSE;
        }

        m_objConfig.setTxPayloadTypeNumber((int32_t)pLocalPayload->objRtpMap.nPayloadNum);
    }

    // remote network parameters
    m_objConfig.setRemoteAddress(android::String8(pPeerProfile->objIpAddress.ToString().GetStr()));
    m_objConfig.setRemotePort(pPeerProfile->nDataPort);
    m_objConfig.setDscp(m_pConfig->GetVideoDscp());
    m_objConfig.setMaxMtuBytes(1500);
    m_objConfig.setRxPayloadTypeNumber(pDestPayload->objRtpMap.nPayloadNum);
    MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlodId);

    if (pMediaManager != IMS_NULL)
    {
        m_objConfig.setMaxMtuBytes(
                pMediaManager->GetResourceManager()->GetRtpFragmentSize(m_objLocalAddress));
    }

    IMS_SINT32 nVideoDerection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;

    switch (pNegoProfile->eDirection)
    {
        case MEDIA_DIRECTION_RECEIVE:
            nVideoDerection = RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY;
            break;
        case MEDIA_DIRECTION_SEND:
            nVideoDerection = RtpConfig::MEDIA_DIRECTION_SEND_ONLY;
            break;
        case MEDIA_DIRECTION_SEND_RECEIVE:
            nVideoDerection = RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE;
            break;
        case MEDIA_DIRECTION_INACTIVE:
            nVideoDerection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
            break;
        default:
            nVideoDerection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
            break;
    }

    if (pNegoProfile->nDataPort == 0 || pLocalProfile->nDataPort == 0)
    {
        nVideoDerection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
    }

    m_nPrevDirection = nVideoDerection;
    m_objConfig.setMediaDirection((int32_t)nVideoDerection);

    IMS_TRACE_D("UpdateRtpConfig() - TxPayloadTypeNumber[%d], RxPayloadTypeNumber[%d]",
            m_objConfig.getTxPayloadTypeNumber(), m_objConfig.getRxPayloadTypeNumber(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - RemoteAddress[%s], RemotePort[%d]",
            m_objConfig.getRemoteAddress().c_str(), m_objConfig.getRemotePort(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - Dscp[%d], MaxMtuBytes[%d], MediaDirection[%d]",
            m_objConfig.getDscp(), m_objConfig.getMaxMtuBytes(), m_objConfig.getMediaDirection());

    RtcpConfig objRtcpConfig;
    objRtcpConfig.setCanonicalName(android::String8("Canonical_Name"));
    objRtcpConfig.setTransmitPort(pNegoProfile->nControlPort);

    if (pNegoProfile->nBandwidthRs == 0 && pNegoProfile->nBandwidthRr == 0)
    {
        objRtcpConfig.setIntervalSec(0);
    }
    else
    {
        objRtcpConfig.setIntervalSec(pNegoProfile->nRtcpInterval);
    }

    objRtcpConfig.setRtcpXrBlockTypes(0);
    m_objConfig.setRtcpConfig(objRtcpConfig);

    IMS_TRACE_D("UpdateRtpConfig() - RTCP CanonicalName[%s], RtcpXrBlockTypes[%d]",
            objRtcpConfig.getCanonicalName().c_str(), objRtcpConfig.getRtcpXrBlockTypes(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - RTCP TransmitPort[%d], IntervalSec[%d]",
            objRtcpConfig.getTransmitPort(), objRtcpConfig.getIntervalSec(), 0);

    /** TODO: add statement to split case between recording and pause image mode */
    m_objConfig.setVideoMode(VideoConfig::VIDEO_MODE_RECORDING);

    if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
    {
        VideoProfile::AvcFmtp* pFmtp =
                reinterpret_cast<VideoProfile::AvcFmtp*>(pNegoPayload->pFmtp);

        m_objConfig.setCodecType(VideoConfig::CODEC_AVC);
        m_objConfig.setFramerate(pFmtp->nFrameRate);
        m_objConfig.setBitrate(pFmtp->nBitrate);
        m_objConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));

        IMS_UINT32 nTempAvcProfile = VideoConfig::CODEC_PROFILE_NONE;

        switch (pFmtp->nProfile)
        {
            case AVC_PROFILE_NONE:
                nTempAvcProfile = VideoConfig::CODEC_PROFILE_NONE;
                break;
            case AVC_PROFILE_B:
                nTempAvcProfile = VideoConfig::AVC_PROFILE_BASELINE;
                break;
            case AVC_PROFILE_CB:
                nTempAvcProfile = VideoConfig::AVC_PROFILE_CONSTRAINED_BASELINE;
                break;
            case AVC_PROFILE_H:
                nTempAvcProfile = VideoConfig::AVC_PROFILE_HIGH;
                break;
            case AVC_PROFILE_M:
            case AVC_PROFILE_E:
                nTempAvcProfile = VideoConfig::AVC_PROFILE_MAIN;
                break;
            default:
                break;
        }

        m_objConfig.setCodecProfile(nTempAvcProfile);

        IMS_UINT32 nTempAvcLevel = VideoConfig::AVC_LEVEL_1;

        switch (pFmtp->nLevel)
        {
            case 31:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_31;
                break;
            case 30:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_3;
                break;
            case 22:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_22;
                break;
            case 21:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_21;
                break;
            case 20:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_2;
                break;
            case 13:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_13;
                break;
            case 12:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_12;
                break;
            case 11:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_11;
                break;
            case 10:
            default:
                nTempAvcLevel = VideoConfig::AVC_LEVEL_1;
                break;
        }

        /** TODO: check the case for setting AVC_LEVEL_1B */
        m_objConfig.setCodecLevel(nTempAvcLevel);
        m_objConfig.setIntraFrameInterval(m_pConfig->GetVideoIframeIntervalSec());
        m_objConfig.setPacketizationMode(pFmtp->nPacketizationMode);
        m_objConfig.setCameraId(m_nCameraId);
        m_objConfig.setCameraZoom(m_nCameraZoom);

        IMS_UINT32 nWidth = 0;
        IMS_UINT32 nHeight = 0;
        VideoProfileUtil::GetWidthHeightFromResolution(pFmtp->eResolution, &nWidth, &nHeight);
        m_objConfig.setResolutionWidth(nWidth);
        m_objConfig.setResolutionHeight(nHeight);
        m_objConfig.setPauseImagePath(android::String8("/image/path"));
        m_objConfig.setDeviceOrientationDegree(0);
        m_objConfig.setCvoValue(pNegoProfile->nCvoId);

        IMS_UINT32 nRtcpFbAttr = VideoConfig::RTP_FB_NONE;

        if (pNegoPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
        {
            nRtcpFbAttr += VideoConfig::RTP_FB_NACK;
        }

        if (pNegoPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
        {
            nRtcpFbAttr += VideoConfig::RTP_FB_TMMBR;
            nRtcpFbAttr += VideoConfig::RTP_FB_TMMBN;
        }

        if (pNegoPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
        {
            nRtcpFbAttr += VideoConfig::PSFB_PLI;
        }

        if (pNegoPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
        {
            nRtcpFbAttr += VideoConfig::PSFB_FIR;
        }

        m_objConfig.setRtcpFbType(nRtcpFbAttr);

        IMS_TRACE_D("UpdateRtpConfig() - VideoMode[%d], Codectype[%d]", m_objConfig.getVideoMode(),
                m_objConfig.getCodecType(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - Framerate[%d], Bitrate[%d]", m_objConfig.getFramerate(),
                m_objConfig.getBitrate(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - CodecProfil[%d], CodecLevel[%d]",
                m_objConfig.getCodecProfile(), m_objConfig.getCodecLevel(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - IntraFrameInterval[%d], PacketizationMode[%d]",
                m_objConfig.getIntraFrameInterval(), m_objConfig.getPacketizationMode(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - CameraId[%d], CameraZoom[%d]", m_objConfig.getCameraId(),
                m_objConfig.getCameraZoom(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - ResolutionWidth[%d], ResolutionHeight[%d]",
                m_objConfig.getResolutionWidth(), m_objConfig.getResolutionHeight(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - PauseImagePath[%s], DeviceOrientationDegree[%d]",
                m_objConfig.getPauseImagePath().c_str(), m_objConfig.getDeviceOrientationDegree(),
                0);
        IMS_TRACE_D("UpdateRtpConfig() - CvoValue[%d], RtcpFbType[%d]", m_objConfig.getCvoValue(),
                m_objConfig.getRtcpFbType(), 0);
    }
    else if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
    {
        /** TODO_MEDIA */

        // CODEC_LEVEL_NONE,
        // HEVC_HIGHTIER_LEVEL_1,
        // HEVC_HIGHTIER_LEVEL_2,
        // HEVC_HIGHTIER_LEVEL_21,
        // HEVC_HIGHTIER_LEVEL_3,
        // HEVC_HIGHTIER_LEVEL_31,
        // HEVC_HIGHTIER_LEVEL_4,
        // HEVC_HIGHTIER_LEVEL_41,
        // HEVC_MAINTIER_LEVEL_1,
        // HEVC_MAINTIER_LEVEL_2,
        // HEVC_MAINTIER_LEVEL_21,
        // HEVC_MAINTIER_LEVEL_3,
        // HEVC_MAINTIER_LEVEL_31,
        // HEVC_MAINTIER_LEVEL_4,
        // HEVC_MAINTIER_LEVEL_41,
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoMediaSession::IsDirectionHold()
{
    IMS_UINT32 nDirection = m_objConfig.getMediaDirection();
    IMS_TRACE_D("IsDirectionHold() - m_objConfig direction[%d]", nDirection, 0, 0);
    return (nDirection == (IMS_UINT32)RtpConfig::MEDIA_DIRECTION_NO_FLOW) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
void VideoMediaSession::HoldRtpConfig()
{
    m_objConfig.setMediaDirection((int32_t)RtpConfig::MEDIA_DIRECTION_NO_FLOW);
}

PUBLIC
IMS_BOOL VideoMediaSession::UpdateMediaQualityThreshold(
        IN IMS_BOOL bIsHold, IN IMS_BOOL bEnableRtcp)
{
    /** TODO_MEDIA need to get real value when it's ready. */
    if (bIsHold)
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(0);
        m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
                m_pConfig->GetRtcpInactivityTimerMillis());
        m_objMediaQualityThreshold.setRtpPacketLossDurationMillis(0);
        m_objMediaQualityThreshold.setRtpPacketLossRate(0);
        m_objMediaQualityThreshold.setJitterDurationMillis(0);
        m_objMediaQualityThreshold.setRtpJitterMillis(0);
    }
    else
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(
                m_pConfig->GetRtpInactivityTimerMillis());

        if (bEnableRtcp == IMS_TRUE)
        {
            m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
                    m_pConfig->GetRtcpInactivityTimerMillis());
        }
        else
        {
            m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(0);
        }

        m_objMediaQualityThreshold.setRtpPacketLossDurationMillis(15000);
        m_objMediaQualityThreshold.setRtpPacketLossRate(30);
        m_objMediaQualityThreshold.setJitterDurationMillis(15000);
        m_objMediaQualityThreshold.setRtpJitterMillis(100);
    }

    IMS_TRACE_D("UpdateMediaQualityThreshold() - IsHold[%d], RtpInactivity[%d], RtcpInactivity[%d]",
            bIsHold, m_objMediaQualityThreshold.getRtpInactivityTimerMillis(),
            m_objMediaQualityThreshold.getRtcpInactivityTimerMillis());
    IMS_TRACE_D("UpdateMediaQualityThreshold() - PacketLossDurationMillis[%d], PacketLossRate[%d]",
            m_objMediaQualityThreshold.getRtpPacketLossDurationMillis(),
            m_objMediaQualityThreshold.getRtpPacketLossRate(), 0);
    IMS_TRACE_D("UpdateMediaQualityThreshold() - JitterDurationMillis[%d], JitterMillis[%d]",
            m_objMediaQualityThreshold.getJitterDurationMillis(),
            m_objMediaQualityThreshold.getRtpJitterMillis(), 0);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoMediaSession::UpdateLocalEndPoint(IN VideoProfile* pNegoProfile)
{
    if (pNegoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pNegoProfile->objIpAddress.ToString().IsNULL())
    {
        m_objLocalAddress = pNegoProfile->objIpAddress;
    }

    m_nLocalPort = pNegoProfile->nDataPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);

    return IMS_TRUE;
}

PUBLIC
void VideoMediaSession::UpdateLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort)
{
    if (!objLocalAddr.ToString().IsNULL())
    {
        m_objLocalAddress = objLocalAddr;
    }

    m_nLocalPort = nPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);
}

PUBLIC
IMS_BOOL VideoMediaSession::OnMessages(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    IMS_TRACE_I("OnMessages() - Msg[%d, %s]", nMsg, IMMedia::PrintMsg(nMsg), 0);

    IMS_BOOL bRet = IMS_TRUE;

    switch (nMsg)
    {
        case IMMedia::SETSURFACE_CMD:
            bRet = OnSetSurfaceCmd(pParam);
            break;
        case IMMedia::SELECT_CAMERA_CMD:
            bRet = OnSelectCameraCmd(pParam);
            break;
        case IMMedia::CHANGE_CAMERA_ZOOM_CMD:
            bRet = OnChangeCameraZoomCmd(pParam);
            break;
        case IMMedia::SET_PAUSE_IMAGE_CMD:
            bRet = OnSetPauseImageCmd(pParam);
            break;
        case IMMedia::CHANGE_ORIENTATION_CMD:
            bRet = OnChangeOrientation(pParam);
            break;
        default:
            break;
    }
    return bRet;
}

PUBLIC
IMS_BOOL VideoMediaSession::Open()
{
    IMS_TRACE_I("Open() - cameraId[%d]", m_nCameraId, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_VIDEO);
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        m_objConfig.setVideoMode(VideoConfig::VIDEO_MODE_PREVIEW);
        m_objConfig.setCameraId(m_nCameraId);
        pParam->m_pConfig = new VideoConfig(m_objConfig);

        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IMMedia::REQUEST_OPEN_SESSION, pParam) == IMS_TRUE)
        {
            SetStateFromVideoMode(m_objConfig.getVideoMode());

            if (m_bPreviewSurfaceSet)
            {
                OnSetSurfaceCmd(reinterpret_cast<IMS_UINTP>(new ImsMediaVideoParam(SURFACE_NEAR)));
            }

            if (m_bDisplaySurfaceSet)
            {
                OnSetSurfaceCmd(reinterpret_cast<IMS_UINTP>(new ImsMediaVideoParam(SURFACE_FAR)));
            }

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoMediaSession::Modify()
{
    IMS_TRACE_I("Modify()", 0, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL && m_nState != STATE_IDLE)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_VIDEO);
        pParam->m_pConfig = new VideoConfig(m_objConfig);
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_MODIFY_SESSION, pParam);

        if (m_objConfig.getMediaDirection() != RtpConfig::MEDIA_DIRECTION_NO_FLOW)
        {
            SetStateFromVideoMode(m_objConfig.getVideoMode());
        }
        else
        {
            m_nState = STATE_PAUSED;
        }
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoMediaSession::Close()
{
    IMS_TRACE_I("Close()", 0, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO);
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_CLOSE_SESSION, pParam);

        m_nState = STATE_IDLE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoMediaSession::SetMediaQuality()
{
    IMS_TRACE_I("SetMediaQuality()", 0, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL && m_nState != STATE_IDLE)
    {
        ImsMediaMsgSetMediaQualityParam* pParam =
                new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_VIDEO);
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;

        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
    }
    return bResult;
}

PUBLIC
IMS_SINT32 VideoMediaSession::GetLocalPort()
{
    return m_nLocalPort;
}

PUBLIC
IMS_SINT32 VideoMediaSession::GetRemotePort()
{
    return m_objConfig.getRemotePort();
}

PUBLIC
IMS_SINT32 VideoMediaSession::GetCameraId()
{
    return m_nCameraId;
}

PRIVATE
IMS_BOOL VideoMediaSession::OnSetSurfaceCmd(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != NULL)
    {
        IMS_TRACE_I("OnSetSurfaceCmd() - surface type[%d]", param->nValue, 0, 0);

        if (m_piMediaSessionListener != IMS_NULL)
        {
            ImsMediaMsgParamBase newParam(MEDIA_TYPE_VIDEO);

            if (param->nValue == SURFACE_FAR)
            {
                if (m_bDisplaySurfaceSet == IMS_FALSE)
                {
                    m_bDisplaySurfaceSet = IMS_TRUE;

                    if (m_nState == STATE_IDLE)
                    {
                        return IMS_TRUE;
                    }
                }

                m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                        IMMedia::REQUEST_SET_DISPLAY_SURFACE, &newParam);
            }
            else if (param->nValue == SURFACE_NEAR)
            {
                if (m_bPreviewSurfaceSet == IMS_FALSE)
                {
                    m_bPreviewSurfaceSet = IMS_TRUE;

                    if (m_nState == STATE_IDLE)
                    {
                        return IMS_TRUE;
                    }
                }

                m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                        IMMedia::REQUEST_SET_PREVIEW_SURFACE, &newParam);
            }
        }

        delete param;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoMediaSession::OnSelectCameraCmd(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != NULL)
    {
        IMS_TRACE_I("OnSelectCameraCmd() - camera id[%d]", param->nValue, 0, 0);

        if (m_nCameraId != param->nValue)
        {
            m_nCameraId = param->nValue;
        }

        delete param;

        if (m_nState == STATE_PREVIEW || m_nState == STATE_RECORDING)
        {
            if (m_nCameraId != CAMERA_ID_NONE)
            {
                if (m_objConfig.getMediaDirection() == RtpConfig::MEDIA_DIRECTION_INACTIVE)
                {
                    m_objConfig.setMediaDirection(m_nPrevDirection);
                }

                m_objConfig.setCameraId(m_nCameraId);
                this->Modify();
            }
            else
            {
                /** TODO: go to pause Image mode, currently set media direction to inactive to
                 * disable the camera */
                m_nPrevDirection = (IMS_SINT32)m_objConfig.getMediaDirection();
                m_objConfig.setMediaDirection(RtpConfig::MEDIA_DIRECTION_INACTIVE);
                this->Modify();
            }
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoMediaSession::OnChangeCameraZoomCmd(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != NULL)
    {
        IMS_TRACE_I("OnChangeCameraZoomCmd() - camera zoom[%d]", param->nValue, 0, 0);
        m_nCameraZoom = param->nValue;
        delete param;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoMediaSession::OnSetPauseImageCmd(IN IMS_UINTP pParam)
{
    (void)pParam;
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL VideoMediaSession::OnChangeOrientation(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != NULL)
    {
        IMS_TRACE_I(
                "OnChangeOrientation() - orientation[%d], state[%d]", param->nValue, m_nState, 0);

        if (m_nState != STATE_PREVIEW)
        {
            m_objConfig.setDeviceOrientationDegree(param->nValue);
            this->Modify();
        }

        delete param;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void VideoMediaSession::SetStateFromVideoMode(IN IMS_SINT32 mode)
{
    switch (mode)
    {
        case VideoConfig::VIDEO_MODE_PREVIEW:
            m_nState = STATE_PREVIEW;
            break;
        case VideoConfig::VIDEO_MODE_RECORDING:
            m_nState = STATE_RECORDING;
            break;
        case VideoConfig::VIDEO_MODE_PAUSE_IMAGE:
            m_nState = STATE_PAUSE_IMAGE;
    }
}
