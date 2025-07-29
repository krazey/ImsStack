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

#include "MediaDef.h"
#include "ServiceTrace.h"

#include "IMediaSessionListener.h"
#include "IJniMedia.h"
#include "MediaManager.h"
#include "MediaResourceManager.h"
#include "config/VideoConfiguration.h"
#include "video/VideoSession.h"
#include "video/VideoProfileUtil.h"

#include <VideoConfig.h>

using namespace android::telephony::imsmedia;

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoSession::VideoSession(IN IMS_SINT32 nSlotId) :
        BaseSession(nSlotId),
        m_nCameraId(CAMERA_ID_NONE),
        m_nCameraZoom(-1),
        m_bPreviewSurfaceSet(IMS_FALSE),
        m_bDisplaySurfaceSet(IMS_FALSE),
        m_ePemType(MEDIA_PEM_TYPE::NONE)
{
    IMS_TRACE_I("+VideoSession() - state[%d]", m_nState, 0, 0);

    m_pRtpConfig = new VideoConfig();
}

PUBLIC VIRTUAL VideoSession::~VideoSession()
{
    IMS_TRACE_I("~VideoSession() - state[%d]", GetState(), 0, 0);

    if (m_pRtpConfig)
    {
        delete m_pRtpConfig;
    }
}

PUBLIC IMS_BOOL VideoSession::UpdateRtpConfig(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN VideoProfile* pNegoProfile, IN IMS_BOOL bConfirmedSession,
        IN IMS_BOOL bHold)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegoProfile == IMS_NULL ||
            m_pRtpConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateRtpConfig(): invalid parameter", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pLocalProfile->GetPayloadList().GetSize() == 0 ||
            pNegoProfile->GetPayloadList().GetSize() == 0 ||
            pPeerProfile->GetPayloadList().GetSize() == 0)
    {
        IMS_TRACE_E(0, "UpdateRtpConfig(): empty payload list", 0, 0, 0);
        return IMS_FALSE;
    }

    VideoProfile::Payload* pLocalPayload;
    VideoProfile::Payload* pNegoPayload;

    if (pLocalProfile->GetNegotiatedPayloadIndex() < 0)
    {
        pLocalPayload = pLocalProfile->GetPayloadAt(0);
    }
    else
    {
        pLocalPayload = pLocalProfile->GetPayloadAt(pLocalProfile->GetNegotiatedPayloadIndex());
    }

    pNegoPayload = pNegoProfile->GetPayloadAt(0);

    if (pLocalPayload == IMS_NULL || pNegoPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SetLocalEndPoint(pLocalProfile->GetIpAddress(), pLocalProfile->GetDataPort());
    SetRemoteEndPoint(pPeerProfile->GetIpAddress(), pPeerProfile->GetDataPort());

    VideoConfig* pVideoConfig = REINTERPRET_CAST(VideoConfig*, m_pRtpConfig);

    pVideoConfig->setTxPayloadTypeNumber(pLocalPayload->GetRtpMap().GetPayloadNumber());
    pVideoConfig->setRxPayloadTypeNumber(pNegoPayload->GetRtpMap().GetPayloadNumber());

    if (GetConfiguration() != IMS_NULL)
    {
        pVideoConfig->setDscp(GetConfiguration()->GetVideoDscp());
    }
    pVideoConfig->setMaxMtuBytes(1500);

    MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);

    if (pMediaManager != IMS_NULL)
    {
        MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

        if (pResourceMngr != IMS_NULL)
        {
            pVideoConfig->setMaxMtuBytes(pResourceMngr->GetRtpFragmentSize());
        }
    }

    IMS_SINT32 nVideoDirection;

    if (pNegoProfile->GetDataPort() == 0 || pLocalProfile->GetDataPort() == 0)
    {
        nVideoDirection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
    }
    else
    {
        switch (pNegoProfile->GetDirection())
        {
            case MEDIA_DIRECTION_RECEIVE:
                nVideoDirection = bHold ? RtpConfig::MEDIA_DIRECTION_INACTIVE
                                        : RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY;
                if (!bConfirmedSession && m_ePemType == MEDIA_PEM_TYPE::SENDRECV)
                {
                    nVideoDirection = RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE;
                    IMS_TRACE_D("UpdateRtpConfig() - media direction[%d]", nVideoDirection, 0, 0);
                }
                break;
            case MEDIA_DIRECTION_SEND:
                nVideoDirection = bHold ? RtpConfig::MEDIA_DIRECTION_INACTIVE
                                        : RtpConfig::MEDIA_DIRECTION_SEND_ONLY;
                if ((GetConfiguration() != IMS_NULL &&
                            GetConfiguration()->IsRecvOnlyEarlySessionEnabled()) &&
                        !bConfirmedSession && m_ePemType != MEDIA_PEM_TYPE::SENDRECV &&
                        m_ePemType != MEDIA_PEM_TYPE::SENDONLY)
                {
                    nVideoDirection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
                    IMS_TRACE_D("UpdateRtpConfig() - media direction[%d]", nVideoDirection, 0, 0);
                }
                break;
            case MEDIA_DIRECTION_SEND_RECEIVE:
                nVideoDirection = RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE;
                if ((GetConfiguration() != IMS_NULL &&
                            GetConfiguration()->IsRecvOnlyEarlySessionEnabled()) &&
                        !bConfirmedSession && m_ePemType != MEDIA_PEM_TYPE::SENDRECV &&
                        m_ePemType != MEDIA_PEM_TYPE::SENDONLY)
                {
                    nVideoDirection = RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY;
                    IMS_TRACE_D("UpdateRtpConfig() - media direction[%d]", nVideoDirection, 0, 0);
                }
                break;
            case MEDIA_DIRECTION_INACTIVE:
                nVideoDirection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
                break;
            default:
                nVideoDirection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
                break;
        }
    }

    if (GetConfiguration()->isVideoDirectionHoldUsingInactive() &&
            (nVideoDirection == RtpConfig::MEDIA_DIRECTION_SEND_ONLY ||
                    nVideoDirection == RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY))
    {
        nVideoDirection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
        IMS_TRACE_I("UpdateRtpConfig() - updated direction[%d]", nVideoDirection, 0, 0);
    }

    pVideoConfig->setMediaDirection((int32_t)nVideoDirection);

    IMS_TRACE_D("UpdateRtpConfig() - MediaDirection[%d], TxPayload[%d], RxPayload[%d]",
            pVideoConfig->getMediaDirection(), pVideoConfig->getTxPayloadTypeNumber(),
            pVideoConfig->getRxPayloadTypeNumber());
    IMS_TRACE_D("UpdateRtpConfig() - RemoteAddress[%s], RemotePort[%d]",
            pVideoConfig->getRemoteAddress().c_str(), pVideoConfig->getRemotePort(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - Dscp[%d], Mtu[%d]", u_char(pVideoConfig->getDscp()),
            pVideoConfig->getMaxMtuBytes(), 0);

    RtcpConfig objRtcpConfig;
    objRtcpConfig.setCanonicalName(android::String8("Canonical_Name"));
    objRtcpConfig.setTransmitPort(pPeerProfile->GetControlPort());

    if (pNegoProfile->GetBandwidthRs() == 0 && pNegoProfile->GetBandwidthRr() == 0)
    {
        objRtcpConfig.setIntervalSec(0);
    }
    else
    {
        objRtcpConfig.setIntervalSec(pNegoProfile->GetRtcpInterval());
    }

    objRtcpConfig.setRtcpXrBlockTypes(0);
    pVideoConfig->setRtcpConfig(objRtcpConfig);

    IMS_TRACE_D("UpdateRtpConfig() - RTCP TransmitPort[%d], IntervalSec[%d], RtcpXrBlockTypes[%d]",
            objRtcpConfig.getTransmitPort(), objRtcpConfig.getIntervalSec(),
            objRtcpConfig.getRtcpXrBlockTypes());

    if (m_nCameraId == CAMERA_ID_NONE)
    {
        pVideoConfig->setVideoMode(VideoConfig::VIDEO_MODE_PAUSE_IMAGE);
    }
    else
    {
        pVideoConfig->setVideoMode(VideoConfig::VIDEO_MODE_RECORDING);
    }

    if (pNegoPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
    {
        auto pFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pNegoPayload->GetFmtp());

        pVideoConfig->setCodecType(VideoConfig::CODEC_AVC);
        pVideoConfig->setCodecProfile(convertAvcProfile((pFmtp->GetProfile())));

        /** TODO: check the case for setting AVC_LEVEL_1B */
        pVideoConfig->setCodecLevel(convertAvcLevel(pFmtp->GetLevel()));

        pVideoConfig->setFramerate(pFmtp->GetFramerate());
        pVideoConfig->setBitrate(pFmtp->GetBitrate());
        pVideoConfig->setPacketizationMode(pFmtp->GetPacketizationMode());
        pVideoConfig->setCodecSprop(android::String8(pFmtp->GetSpropParam().GetStr()));

        IMS_UINT32 nWidth = 0;
        IMS_UINT32 nHeight = 0;
        VideoProfileUtil::GetWidthHeightFromResolution(pFmtp->GetResolution(), &nWidth, &nHeight);
        pVideoConfig->setResolutionWidth(nWidth);
        pVideoConfig->setResolutionHeight(nHeight);
    }
    else if (pNegoPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
    {
        auto pFmtp = std::static_pointer_cast<VideoProfile::HevcFmtp>(pNegoPayload->GetFmtp());

        pVideoConfig->setCodecType(VideoConfig::CODEC_HEVC);
        pVideoConfig->setCodecProfile(convertHevcProfile((pFmtp->GetProfile())));

        /** The levelId received from sdp profile should be converted to a value divided by 3. */
        pVideoConfig->setCodecLevel(convertHevcLevel(pFmtp->GetLevel() / 3));

        pVideoConfig->setFramerate(pFmtp->GetFramerate());
        pVideoConfig->setBitrate(pFmtp->GetBitrate());
        pVideoConfig->setPacketizationMode(pFmtp->GetPacketizationMode());
        pVideoConfig->setCodecSprop(android::String8(pFmtp->GetSpropParam().GetStr()));

        IMS_UINT32 nWidth = 0;
        IMS_UINT32 nHeight = 0;
        VideoProfileUtil::GetWidthHeightFromResolution(pFmtp->GetResolution(), &nWidth, &nHeight);
        pVideoConfig->setResolutionWidth(nWidth);
        pVideoConfig->setResolutionHeight(nHeight);
    }

    pVideoConfig->setSamplingRateKHz((int8_t)(pNegoPayload->GetRtpMap().GetSamplingRate() / 1000));
    if (GetConfiguration() != IMS_NULL)
    {
        pVideoConfig->setIntraFrameInterval(GetConfiguration()->GetVideoIframeIntervalSec());
    }
    pVideoConfig->setCameraId(m_nCameraId);
    pVideoConfig->setCameraZoom(m_nCameraZoom);
    pVideoConfig->setPauseImagePath(android::String8("/image/path"));
    pVideoConfig->setDeviceOrientationDegree(0);
    pVideoConfig->setCvoValue(pNegoProfile->GetCvoId());

    IMS_UINT32 nRtcpFbAttr = VideoConfig::RTP_FB_NONE;

    if (pNegoPayload->GetRtcpFbAttr().IsNackSupported())
    {
        nRtcpFbAttr |= VideoConfig::RTP_FB_NACK;
    }

    if (pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported())
    {
        nRtcpFbAttr |= VideoConfig::RTP_FB_TMMBR;
        nRtcpFbAttr |= VideoConfig::RTP_FB_TMMBN;
    }

    if (pNegoPayload->GetRtcpFbAttr().IsPliSupported())
    {
        nRtcpFbAttr |= VideoConfig::PSFB_PLI;
    }

    if (pNegoPayload->GetRtcpFbAttr().IsFirSupported())
    {
        nRtcpFbAttr |= VideoConfig::PSFB_FIR;
    }

    pVideoConfig->setRtcpFbType(nRtcpFbAttr);

    IMS_TRACE_D("UpdateRtpConfig() - VideoMode[%d], Codectype[%d]", pVideoConfig->getVideoMode(),
            pVideoConfig->getCodecType(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - Framerate[%d], Bitrate[%d]", pVideoConfig->getFramerate(),
            pVideoConfig->getBitrate(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - CodecProfile[%d], CodecLevel[%d], Sprop[%s]",
            pVideoConfig->getCodecProfile(), pVideoConfig->getCodecLevel(),
            pVideoConfig->getCodecSprop().c_str());
    IMS_TRACE_D("UpdateRtpConfig() - IntraFrameInterval[%d], PacketizationMode[%d]",
            pVideoConfig->getIntraFrameInterval(), pVideoConfig->getPacketizationMode(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - CameraId[%d], CameraZoom[%d]", pVideoConfig->getCameraId(),
            pVideoConfig->getCameraZoom(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - ResolutionWidth[%d], ResolutionHeight[%d]",
            pVideoConfig->getResolutionWidth(), pVideoConfig->getResolutionHeight(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - PauseImagePath[%s], DeviceOrientationDegree[%d]",
            pVideoConfig->getPauseImagePath().c_str(), pVideoConfig->getDeviceOrientationDegree(),
            0);
    IMS_TRACE_D("UpdateRtpConfig() - CvoValue[%d], RtcpFbType[%d]", pVideoConfig->getCvoValue(),
            pVideoConfig->getRtcpFbType(), 0);

    return IMS_TRUE;
}

PUBLIC
void VideoSession::SetMtu(IN IMS_SINT32 nMtu)
{
    if (m_pRtpConfig != IMS_NULL)
    {
        VideoConfig* pVideoConfig = static_cast<VideoConfig*>(m_pRtpConfig);
        pVideoConfig->setMaxMtuBytes(nMtu);
        IMS_TRACE_D("SetMtu() - mtu[%d]", pVideoConfig->getMaxMtuBytes(), 0, 0);
    }
}

PUBLIC
IMS_BOOL VideoSession::UpdateMediaQualityThreshold()
{
    if (GetConfiguration() == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateMediaQualityThreshold() - invalid configuration", 0, 0, 0);
        return IMS_FALSE;
    }

    if (GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(
                std::vector<int32_t>{GetConfiguration()->GetRtpInactivityTimerMillis()});
        m_objMediaQualityThreshold.setVideoBitrateBps(
                GetConfiguration()->GetVideoLowestBitrateBps());
    }
    else
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(std::vector<int32_t>{0});
        m_objMediaQualityThreshold.setVideoBitrateBps(0);
    }

    m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
            m_pRtpConfig->getRtcpConfig().getIntervalSec() > 0
                    ? GetConfiguration()->GetRtcpInactivityTimerMillis()
                    : 0);

    IMS_TRACE_D("UpdateMediaQualityThreshold() - RtpInactivity[%d], RtcpInactivity[%d], "
                "VideoLowestBitrate[%d]",
            (m_objMediaQualityThreshold.getRtpInactivityTimerMillis().empty())
                    ? -1
                    : m_objMediaQualityThreshold.getRtpInactivityTimerMillis().front(),
            m_objMediaQualityThreshold.getRtcpInactivityTimerMillis(),
            m_objMediaQualityThreshold.getVideoBitrateBps());

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoSession::OnMessages(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    IMS_TRACE_I("OnMessages() - Msg[%d, %s]", nMsg, IJniMedia::PrintMsg(nMsg), 0);

    IMS_BOOL bRet = IMS_FALSE;

    switch (nMsg)
    {
        case IJniMedia::SETSURFACE_CMD:
            bRet = OnSetSurfaceCmd(pParam);
            break;
        case IJniMedia::SELECT_CAMERA_CMD:
            bRet = OnSelectCameraCmd(pParam);
            break;
        case IJniMedia::CHANGE_CAMERA_ZOOM_CMD:
            bRet = OnChangeCameraZoomCmd(pParam);
            break;
        case IJniMedia::SET_PAUSE_IMAGE_CMD:
            bRet = OnSetPauseImageCmd(pParam);
            break;
        case IJniMedia::CHANGE_ORIENTATION_CMD:
            bRet = OnChangeOrientation(pParam);
            break;
        default:
            break;
    }
    return bRet;
}

PUBLIC
IMS_BOOL VideoSession::Open()
{
    IMS_TRACE_I("Open() - state[%d], cameraId[%d]", m_nState, m_nCameraId, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_VIDEO);
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        pParam->m_pConfig = IMS_NULL;

        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IJniMedia::REQUEST_OPEN_SESSION, pParam) == IMS_TRUE)
        {
            m_nState = STATE_IDLE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoSession::Modify()
{
    if (m_pRtpConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    VideoConfig* pVideoConfig = REINTERPRET_CAST(VideoConfig*, m_pRtpConfig);
    IMS_TRACE_I("Modify() - state[%d], VideoMode[%d]", m_nState, pVideoConfig->getVideoMode(), 0);

    if (m_piMediaSessionListener != IMS_NULL && m_nState >= STATE_IDLE)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_VIDEO);
        pParam->m_pConfig = new VideoConfig(REINTERPRET_CAST(VideoConfig*, m_pRtpConfig));
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_MODIFY_SESSION, pParam);

        if (m_bPreviewSurfaceSet)
        {
            OnSetSurfaceCmd(reinterpret_cast<IMS_UINTP>(new ImsMediaVideoParam(SURFACE_NEAR)));
        }

        if (m_bDisplaySurfaceSet)
        {
            OnSetSurfaceCmd(reinterpret_cast<IMS_UINTP>(new ImsMediaVideoParam(SURFACE_FAR)));
        }

        switch (pVideoConfig->getVideoMode())
        {
            case VideoConfig::VIDEO_MODE_PREVIEW:
                m_nState = STATE_PREVIEW;
                return IMS_TRUE;
            case VideoConfig::VIDEO_MODE_RECORDING:
                if (GetDirection() == MEDIA_DIRECTION_RECEIVE)
                {
                    m_nState = STATE_RENDERING;
                }
                else if (MEDIA_DIRECTION_INVOLVED_SEND(GetDirection()))
                {
                    m_nState = STATE_RECORDING;
                }
                break;
            case VideoConfig::VIDEO_MODE_PAUSE_IMAGE:
                if (GetDirection() == MEDIA_DIRECTION_RECEIVE)
                {
                    m_nState = STATE_RENDERING;
                }
                else if (MEDIA_DIRECTION_INVOLVED_SEND(GetDirection()))
                {
                    m_nState = STATE_PAUSE_IMAGE;
                }
                break;
        }

        if (MEDIA_DIRECTION_IS_VIDEO_HOLD(GetDirection()))
        {
            m_nState = STATE_PAUSED;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoSession::Close()
{
    IMS_TRACE_I("Close() - state[%d]", m_nState, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO);
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_CLOSE_SESSION, pParam);

        m_nState = STATE_NONE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoSession::SetMediaQuality()
{
    IMS_TRACE_I("SetMediaQuality() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL && m_nState != STATE_NONE)
    {
        ImsMediaMsgSetMediaQualityParam* pParam =
                new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_VIDEO);
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;

        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
    }
    return bResult;
}

PUBLIC
IMS_SINT32 VideoSession::GetCameraId()
{
    return m_nCameraId;
}

PUBLIC
IMS_SINT32 VideoSession::GetVideoMode()
{
    if (m_pRtpConfig != IMS_NULL)
    {
        VideoConfig* pVideoConfig = REINTERPRET_CAST(VideoConfig*, m_pRtpConfig);
        return pVideoConfig->getVideoMode();
    }

    return -1;
}

PUBLIC
MediaQualityThreshold* VideoSession::GetMediaQualityThreshold()
{
    return &m_objMediaQualityThreshold;
}

void VideoSession::SetMediaPemType(IN MEDIA_PEM_TYPE ePemType)
{
    if (m_ePemType != MEDIA_PEM_TYPE::SENDRECV && m_ePemType != MEDIA_PEM_TYPE::SENDONLY)
    {
        m_ePemType = ePemType;
        IMS_TRACE_D("SetMediaPemType() - Pem Type[%d]", m_ePemType, 0, 0);
    }
}

PRIVATE IMS_BOOL VideoSession::OnSetSurfaceCmd(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != IMS_NULL)
    {
        IMS_TRACE_I("OnSetSurfaceCmd() - state[%d], surface type[%d]", m_nState, param->nValue, 0);

        if (m_piMediaSessionListener != IMS_NULL)
        {
            ImsMediaMsgParamBase newParam(MEDIA_TYPE_VIDEO);

            if (param->nValue == SURFACE_FAR)
            {
                if (m_bDisplaySurfaceSet == IMS_FALSE)
                {
                    m_bDisplaySurfaceSet = IMS_TRUE;

                    if (m_nState == STATE_NONE)
                    {
                        return IMS_TRUE;
                    }
                }

                m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                        IJniMedia::REQUEST_SET_DISPLAY_SURFACE, &newParam);
            }
            else if (param->nValue == SURFACE_NEAR)
            {
                if (m_bPreviewSurfaceSet == IMS_FALSE)
                {
                    m_bPreviewSurfaceSet = IMS_TRUE;

                    if (m_nState == STATE_NONE)
                    {
                        return IMS_TRUE;
                    }
                }

                m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                        IJniMedia::REQUEST_SET_PREVIEW_SURFACE, &newParam);
            }
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSession::OnSelectCameraCmd(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != IMS_NULL)
    {
        IMS_TRACE_I("OnSelectCameraCmd() - state[%d], camera id[%d]", m_nState, param->nValue, 0);

        VideoConfig* pVideoConfig = REINTERPRET_CAST(VideoConfig*, m_pRtpConfig);

        m_nCameraId = param->nValue;

        // the operation independent with the media direction
        if (m_nCameraId != CAMERA_ID_NONE)
        {
            switch (m_nState)
            {
                case STATE_IDLE:
                    // start preview mode
                    pVideoConfig->setVideoMode(VideoConfig::VIDEO_MODE_PREVIEW);
                    pVideoConfig->setCameraId(m_nCameraId);
                    this->Modify();
                    break;
                case STATE_PAUSE_IMAGE:
                    // recovery from pause state
                    pVideoConfig->setVideoMode(VideoConfig::VIDEO_MODE_RECORDING);
                    pVideoConfig->setCameraId(m_nCameraId);
                    this->Modify();
                    break;
                case STATE_RECORDING:
                    // change camera
                    pVideoConfig->setCameraId(m_nCameraId);
                    this->Modify();
                    break;
                default:
                    IMS_TRACE_E(0, "OnSelectCameraCmd() - invalid state[%d]", m_nState, 0, 0);
                    break;
            }
        }
        else  // camera off cases
        {
            if (m_nState == STATE_RECORDING)
            {
                pVideoConfig->setVideoMode(VideoConfig::VIDEO_MODE_PAUSE_IMAGE);
                pVideoConfig->setCameraId(m_nCameraId);
                this->Modify();
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL VideoSession::OnChangeCameraZoomCmd(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != IMS_NULL)
    {
        IMS_TRACE_I(
                "OnChangeCameraZoomCmd() - state[%d], camera zoom[%d]", m_nState, param->nValue, 0);
        m_nCameraZoom = param->nValue;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSession::OnSetPauseImageCmd(IN IMS_UINTP /*pParam*/)
{
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL VideoSession::OnChangeOrientation(IN IMS_UINTP pParam)
{
    ImsMediaVideoParam* param = reinterpret_cast<ImsMediaVideoParam*>(pParam);

    if (param != IMS_NULL)
    {
        IMS_TRACE_I(
                "OnChangeOrientation() - state[%d], orientation[%d]", m_nState, param->nValue, 0);

        if (m_nState != STATE_PREVIEW)
        {
            REINTERPRET_CAST(VideoConfig*, m_pRtpConfig)->setDeviceOrientationDegree(param->nValue);
            this->Modify();
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_UINT32 VideoSession::convertAvcProfile(IN IMS_UINT32 nProfile)
{
    switch (nProfile)
    {
        case AVC_PROFILE_B:
            return VideoConfig::AVC_PROFILE_BASELINE;
        case AVC_PROFILE_CB:
            return VideoConfig::AVC_PROFILE_CONSTRAINED_BASELINE;
        case AVC_PROFILE_H:
            return VideoConfig::AVC_PROFILE_HIGH;
        case AVC_PROFILE_M:  // FALL-THROUGH
        case AVC_PROFILE_E:
            return VideoConfig::AVC_PROFILE_MAIN;
        case AVC_PROFILE_NONE:  // FALL-THROUGH
        default:
            return VideoConfig::CODEC_PROFILE_NONE;
    }
}

PRIVATE
IMS_UINT32 VideoSession::convertHevcProfile(IN IMS_UINT32 nProfile)
{
    switch (nProfile)
    {
        case HEVC_PROFILE_MAIN:
            return VideoConfig::HEVC_PROFILE_MAIN;
        case HEVC_PROFILE_MAIN10:
            return VideoConfig::HEVC_PROFILE_MAIN10;
        case HEVC_PROFILE_NONE:  // FALL-THROUGH
        default:
            return VideoConfig::CODEC_PROFILE_NONE;
    }
}

PRIVATE
IMS_UINT32 VideoSession::convertAvcLevel(IN IMS_UINT32 nLevel)
{
    switch (nLevel)
    {
        case 31:
            return VideoConfig::AVC_LEVEL_31;
        case 30:
            return VideoConfig::AVC_LEVEL_3;
        case 22:
            return VideoConfig::AVC_LEVEL_22;
        case 21:
            return VideoConfig::AVC_LEVEL_21;
        case 20:
            return VideoConfig::AVC_LEVEL_2;
        case 13:
            return VideoConfig::AVC_LEVEL_13;
        case 12:
            return VideoConfig::AVC_LEVEL_12;
        case 11:
            return VideoConfig::AVC_LEVEL_11;
        case 10:  // FALL-THROUGH
        default:
            return VideoConfig::AVC_LEVEL_1;
    }
}

PRIVATE
IMS_UINT32 VideoSession::convertHevcLevel(IN IMS_UINT32 nLevel)
{
    switch (nLevel)
    {
        case 41:
            return VideoConfig::HEVC_MAINTIER_LEVEL_41;
        case 40:
            return VideoConfig::HEVC_MAINTIER_LEVEL_4;
        case 31:
            return VideoConfig::HEVC_MAINTIER_LEVEL_31;
        case 30:
            return VideoConfig::HEVC_MAINTIER_LEVEL_3;
        case 21:
            return VideoConfig::HEVC_MAINTIER_LEVEL_21;
        case 20:
            return VideoConfig::HEVC_MAINTIER_LEVEL_2;
        case 10:  // FALL-THROUGH
        default:
            return VideoConfig::HEVC_MAINTIER_LEVEL_1;
    }
}

PRIVATE
VideoConfiguration* VideoSession::GetConfiguration()
{
    if (m_pConfiguration == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetConfiguration() - m_pConfiguration is null", 0, 0, 0);
        return IMS_NULL;
    }

    return static_cast<VideoConfiguration*>(m_pConfiguration);
}
