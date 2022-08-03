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
#include "ISessionDescriptor.h"
#include "Configuration.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceNetwork.h"
#include "ServiceEvent.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "text/TextMediaSession.h"
#include "MediaManager.h"
#include "IMMedia.h"

__IMS_TRACE_TAG_USER_DECL__("MED.TS");

PUBLIC TextMediaSession::TextMediaSession(IN IMS_SINT32 nSlotId) :
        BaseSession(nSlotId),
        m_pConfig(IMS_NULL),
        m_objTextConfig(TextConfig()),
        m_objMediaQualityThreshold(MediaQualityThreshold()),
        m_objLocalAddress(IPAddress::IPv6NONE),
        m_nLocalPort(0)
{
    IMS_TRACE_I("+TextMediaSession()", 0, 0, 0);
}

PUBLIC VIRTUAL TextMediaSession::~TextMediaSession()
{
    IMS_TRACE_I("~TextMediaSession()", 0, 0, 0);
}

PUBLIC void TextMediaSession::SetConfig(TextConfiguration* pConfig)
{
    m_pConfig = pConfig;
}

PUBLIC IMS_BOOL TextMediaSession::UpdateRtpConfig(
        IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile, IN TextProfile* pNegoProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegoProfile == IMS_NULL)
    {
        IMS_TRACE_I("UpdateRtpConfig() - invalid parameter", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pNegoProfile->lstPayload.GetSize() == 0 || pPeerProfile->lstPayload.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    // Setting the network properties
    UpdateLocalEndPoint(pNegoProfile);

    // remote network parameters
    m_objTextConfig.setRemoteAddress(
            android::String8(pPeerProfile->objIpAddress.ToString().GetStr()));
    m_objTextConfig.setRemotePort(pPeerProfile->nDataPort);
    m_objTextConfig.setDscp(0); /** TODO: add interface to get text dscp value */

    IMS_SINT32 nTextDerection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;

    switch (pNegoProfile->eDirection)
    {
        case MEDIA_DIRECTION_RECEIVE:
            nTextDerection = RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY;
            break;
        case MEDIA_DIRECTION_SEND:
            nTextDerection = RtpConfig::MEDIA_DIRECTION_SEND_ONLY;
            break;
        case MEDIA_DIRECTION_SEND_RECEIVE:
            nTextDerection = RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE;
            break;
        case MEDIA_DIRECTION_INACTIVE:
            nTextDerection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
            break;
        default:
            nTextDerection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
            break;
    }

    if (pNegoProfile->nDataPort == 0 || pLocalProfile->nDataPort == 0)
    {
        nTextDerection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
    }

    m_objTextConfig.setMediaDirection((int32_t)nTextDerection);

    IMS_TRACE_D("UpdateRtpConfig() - TxPayloadTypeNumber[%d], RxPayloadTypeNumber[%d]",
            m_objTextConfig.getTxPayloadTypeNumber(), m_objTextConfig.getRxPayloadTypeNumber(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - RemoteAddress[%s], RemotePort[%d], MediaDirection[%d]",
            m_objTextConfig.getRemoteAddress().c_str(), m_objTextConfig.getRemotePort(),
            m_objTextConfig.getMediaDirection());

    RtcpConfig* pRtcpConfig = new RtcpConfig();
    pRtcpConfig->setCanonicalName(android::String8("Canonical_Name")); /** TODO_MEDIA */
    pRtcpConfig->setTransmitPort(pNegoProfile->nControlPort);

    if (pNegoProfile->nBandwidthRs == 0 && pNegoProfile->nBandwidthRr == 0)
    {
        pRtcpConfig->setIntervalSec(0);
    }
    else
    {
        pRtcpConfig->setIntervalSec(pNegoProfile->nRtcpInterval);
    }

    pRtcpConfig->setRtcpXrBlockTypes(0);
    m_objTextConfig.setRtcpConfig(*pRtcpConfig);
    delete pRtcpConfig;

    RtcpConfig objRtcpConfig = m_objTextConfig.getRtcpConfig();
    IMS_TRACE_D("UpdateRtpConfig() - RTCP CanonicalName[%s], RtcpXrBlockTypes[%d]",
            objRtcpConfig.getCanonicalName().c_str(), objRtcpConfig.getRtcpXrBlockTypes(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - RTCP TransmitPort[%d], IntervalSec[%d]",
            objRtcpConfig.getTransmitPort(), objRtcpConfig.getIntervalSec(), 0);

    for (IMS_UINT32 nIdxPayload = 0; nIdxPayload < pNegoProfile->lstPayload.GetSize();
            nIdxPayload++)
    {
        TextProfile::Payload* pPayload = pNegoProfile->lstPayload.GetAt(nIdxPayload);

        if (pPayload == IMS_NULL)
            continue;

        m_objTextConfig.setTxPayloadTypeNumber((int32_t)pPayload->objRtpMap.nPayloadNum);
        m_objTextConfig.setRxPayloadTypeNumber((int32_t)pPayload->objRtpMap.nPayloadNum);
        m_objTextConfig.setSamplingRateKHz((int8_t)(pPayload->objRtpMap.nSamplingRate / 1000));

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
        {
            TextProfile::RedFmtp* pFmtp = (TextProfile::RedFmtp*)pPayload->pFmtp;

            if (pFmtp == IMS_NULL)
            {
                IMS_TRACE_E(0, "UpdateProperty() - Invalid pFmtp", 0, 0, 0);
                continue;
            }

            m_objTextConfig.setCodecType(TextConfig::TEXT_T140_RED);
            m_objTextConfig.setRedundantPayload(pFmtp->nRedPayload);
            m_objTextConfig.setRedundantLevel(pFmtp->nRedLevel);
            break;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("t140"))
        {
            m_objTextConfig.setCodecType(TextConfig::TEXT_T140);
            m_objTextConfig.setRedundantPayload(0);
            m_objTextConfig.setRedundantLevel(0);
            break;
        }
        else
        {
            IMS_TRACE_E(0, "UpdateProperty() - Invalid Payload Type", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    m_objTextConfig.setKeepRedundantLevel(pLocalProfile->bKeepRedLevel);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL TextMediaSession::IsDirectionHold()
{
    IMS_UINT32 nDirection = m_objTextConfig.getMediaDirection();
    IMS_TRACE_D("IsDirectionHold() - m_objTextConfig direction[%d]", nDirection, 0, 0);
    return (nDirection == (IMS_UINT32)RtpConfig::MEDIA_DIRECTION_NO_FLOW) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
void TextMediaSession::HoldRtpConfig()
{
    m_objTextConfig.setMediaDirection((int32_t)RtpConfig::MEDIA_DIRECTION_NO_FLOW);
}

PUBLIC
IMS_BOOL TextMediaSession::UpdateMediaQualityThreshold(IN IMS_BOOL bIsHold, IN IMS_BOOL bEnableRtcp)
{
    if (bIsHold == IMS_TRUE)
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
IMS_BOOL TextMediaSession::UpdateLocalEndPoint(IN TextProfile* pNegoProfile)
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
void TextMediaSession::UpdateLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort)
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
IMS_BOOL TextMediaSession::Open()
{
    IMS_TRACE_I("Open()", 0, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_TEXT);
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        pParam->m_pConfig = new TextConfig(m_objTextConfig);

        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IMMedia::REQUEST_OPEN_SESSION, pParam) == IMS_TRUE)
        {
            m_nState = STATE_IDLE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL TextMediaSession::Modify()
{
    IMS_TRACE_I("Modify()", 0, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL && m_nState != STATE_IDLE)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_TEXT);
        pParam->m_pConfig = new TextConfig(m_objTextConfig);

        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IMMedia::REQUEST_MODIFY_SESSION, pParam) == IMS_TRUE)
        {
            if (IsDirectionHold())
            {
                m_nState = STATE_PAUSED;
            }
            else
            {
                m_nState = STATE_LIVE;
            }

            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL TextMediaSession::Close()
{
    IMS_TRACE_I("Close()", 0, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_TEXT);
        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IMMedia::REQUEST_CLOSE_SESSION, pParam) == IMS_TRUE)
        {
            m_nState = STATE_NONE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL TextMediaSession::SetMediaQuality()
{
    IMS_TRACE_I("SetMediaQuality()", 0, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL && m_nState != STATE_IDLE)
    {
        ImsMediaMsgSetMediaQualityParam* pParam =
                new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_TEXT);
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;

        return m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 TextMediaSession::GetLocalPort()
{
    return m_nLocalPort;
}

PUBLIC
IMS_SINT32 TextMediaSession::GetRemotePort()
{
    return m_objTextConfig.getRemotePort();
}

PRIVATE
IMS_BOOL TextMediaSession::SendRtt(IN android::String8 text)
{
    (void)text;
    /** TODO: add implementation */
    return IMS_TRUE;
}