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

#include "IMediaSessionListener.h"
#include "IJniMedia.h"
#include "MediaManager.h"
#include "text/TextMediaSession.h"
#include "text/TextProfileUtil.h"

#include <TextConfig.h>
using namespace android::telephony::imsmedia;

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextMediaSession::TextMediaSession(IN IMS_SINT32 nSlotId) :
        BaseSession(nSlotId),
        m_pConfig(IMS_NULL),
        m_objMediaQualityThreshold(MediaQualityThreshold()),
        m_objLocalAddress(IpAddress::IPv6NONE),
        m_nLocalPort(0)
{
    IMS_TRACE_I("+TextMediaSession()", 0, 0, 0);

    m_pRtpConfig = new TextConfig();
}

PUBLIC VIRTUAL TextMediaSession::~TextMediaSession()
{
    IMS_TRACE_I("~TextMediaSession() - state[%d]", GetState(), 0, 0);

    if (m_pRtpConfig)
    {
        delete m_pRtpConfig;
    }
}

PUBLIC void TextMediaSession::SetConfig(TextConfiguration* pConfig)
{
    m_pConfig = pConfig;
}

PUBLIC IMS_BOOL TextMediaSession::UpdateRtpConfig(
        IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile, IN TextProfile* pNegoProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegoProfile == IMS_NULL ||
            m_pRtpConfig == NULL)
    {
        return IMS_FALSE;
    }

    if (pNegoProfile->GetPayloadList().GetSize() == 0 ||
            pPeerProfile->GetPayloadList().GetSize() == 0)
    {
        return IMS_FALSE;
    }

    TextConfig* pTextConfig = REINTERPRET_CAST(TextConfig*, m_pRtpConfig);

    // Setting the network properties
    UpdateLocalEndPoint(pNegoProfile->GetIpAddress(), pNegoProfile->GetDataPort());
    // remote network parameters
    pTextConfig->setRemoteAddress(
            android::String8(pPeerProfile->GetIpAddress().ToString().GetStr()));
    pTextConfig->setRemotePort(pPeerProfile->GetDataPort());
    pTextConfig->setDscp(0); /** TODO: add interface to get text dscp value */

    IMS_SINT32 nTextDirection;

    if (pNegoProfile->GetDataPort() == 0 || pLocalProfile->GetDataPort() == 0)
    {
        nTextDirection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
    }
    else
    {
        switch (pNegoProfile->GetDirection())
        {
            case MEDIA_DIRECTION_SEND_RECEIVE:
                nTextDirection = RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE;
                break;
            case MEDIA_DIRECTION_RECEIVE:  // FALL-THROUGH
            case MEDIA_DIRECTION_SEND:     // FALL-THROUGH
            case MEDIA_DIRECTION_INACTIVE:
                nTextDirection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
                break;
            default:
                nTextDirection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
                break;
        }
    }
    pTextConfig->setMediaDirection((int32_t)nTextDirection);

    IMS_TRACE_D("UpdateRtpConfig() - RemoteAddress[%s], RemotePort[%d]",
            pTextConfig->getRemoteAddress().c_str(), pTextConfig->getRemotePort(), 0);

    RtcpConfig objRtcpConfig;
    objRtcpConfig.setCanonicalName(android::String8("Canonical_Name")); /** TODO_MEDIA */
    objRtcpConfig.setTransmitPort(pPeerProfile->GetControlPort());
    objRtcpConfig.setIntervalSec(pNegoProfile->GetRtcpInterval());
    objRtcpConfig.setRtcpXrBlockTypes(0);
    pTextConfig->setRtcpConfig(objRtcpConfig);

    IMS_TRACE_D("UpdateRtpConfig() - RTCP TransmitPort[%d], IntervalSec[%d], RtcpXrBlockTypes[%d]",
            objRtcpConfig.getTransmitPort(), objRtcpConfig.getIntervalSec(),
            objRtcpConfig.getRtcpXrBlockTypes());

    for (IMS_UINT32 nIdxPayload = 0; nIdxPayload < pNegoProfile->GetPayloadList().GetSize();
            nIdxPayload++)
    {
        TextProfile::Payload* pPayload = pNegoProfile->GetPayloadAt(nIdxPayload);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            TextProfile::RedFmtp* pFmtp = (TextProfile::RedFmtp*)pPayload->GetFmtp();

            if (pFmtp == IMS_NULL)
            {
                IMS_TRACE_E(0, "UpdateRtpConfig() - Invalid pFmtp", 0, 0, 0);
                continue;
            }

            pTextConfig->setTxPayloadTypeNumber((int32_t)pPayload->GetRtpMap().GetPayloadNumber());
            pTextConfig->setRxPayloadTypeNumber((int32_t)pPayload->GetRtpMap().GetPayloadNumber());
            pTextConfig->setSamplingRateKHz(
                    (int8_t)(pPayload->GetRtpMap().GetSamplingRate() / 1000));
            pTextConfig->setCodecType(TextConfig::TEXT_T140_RED);
            pTextConfig->setRedundantPayload(pFmtp->GetRedPayload());
            pTextConfig->setRedundantLevel(pFmtp->GetRedLevel());
            break;
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"))
        {
            pTextConfig->setTxPayloadTypeNumber((int32_t)pPayload->GetRtpMap().GetPayloadNumber());
            pTextConfig->setRxPayloadTypeNumber((int32_t)pPayload->GetRtpMap().GetPayloadNumber());
            pTextConfig->setSamplingRateKHz(
                    (int8_t)(pPayload->GetRtpMap().GetSamplingRate() / 1000));
            pTextConfig->setCodecType(TextConfig::TEXT_T140);
            pTextConfig->setRedundantPayload(0);
            pTextConfig->setRedundantLevel(0);
        }
        else
        {
            IMS_TRACE_E(0, "UpdateRtpConfig() - Invalid Payload Type", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    pTextConfig->setKeepRedundantLevel(pLocalProfile->GetKeepRedundantLevel());

    IMS_TRACE_D("UpdateRtpConfig() - MediaDirection[%d], TxPayload[%d], RxPayload[%d]",
            pTextConfig->getMediaDirection(), pTextConfig->getTxPayloadTypeNumber(),
            pTextConfig->getRxPayloadTypeNumber());
    IMS_TRACE_D("UpdateRtpConfig() - CodecType[%d], RedPayload[%d], RedLevel[%d]",
            pTextConfig->getCodecType(), pTextConfig->getRedundantPayload(),
            pTextConfig->getRedundantLevel());
    return IMS_TRUE;
}

PUBLIC
void TextMediaSession::UpdateAccessNetwork(IMS_UINT32 nAccessNetwork)
{
    if (m_pRtpConfig != NULL)
    {
        m_pRtpConfig->setAccessNetwork(nAccessNetwork);
        IMS_TRACE_D("UpdateAccessNetwork() - accessNetwork[%d]", m_pRtpConfig->getAccessNetwork(),
                0, 0);
    }
}

PUBLIC
IMS_BOOL TextMediaSession::UpdateMediaQualityThreshold(
        IN IMS_BOOL bActiveSession, IN IMS_BOOL bEnableRtcp)
{
    /** TODO_MEDIA need to get real value when it's ready. */
    if (bActiveSession)
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(
                std::vector<int32_t>{m_pConfig->GetRtpInactivityTimerMillis()});

        m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
                (bEnableRtcp) ? m_pConfig->GetRtcpInactivityTimerMillis() : 0);
    }
    else
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(std::vector<int32_t>{0});
        m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
                m_pConfig->GetRtcpInactivityTimerMillis());
    }

    IMS_TRACE_D("UpdateMediaQualityThreshold() - bActiveSession[%d], RtpInactivity[%d], "
                "RtcpInactivity[%d]",
            bActiveSession,
            (m_objMediaQualityThreshold.getRtpInactivityTimerMillis().empty())
                    ? -1
                    : m_objMediaQualityThreshold.getRtpInactivityTimerMillis().front(),
            m_objMediaQualityThreshold.getRtcpInactivityTimerMillis());

    return IMS_TRUE;
}

PUBLIC
void TextMediaSession::UpdateLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort)
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
    IMS_TRACE_I("Open() - state[%d]", m_nState, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_TEXT);
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        pParam->m_pConfig = new TextConfig(REINTERPRET_CAST(TextConfig*, m_pRtpConfig));

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
IMS_BOOL TextMediaSession::Modify()
{
    IMS_TRACE_I("Modify() - state[%d]", m_nState, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL && m_nState != STATE_NONE)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_TEXT);
        pParam->m_pConfig = new TextConfig(REINTERPRET_CAST(TextConfig*, m_pRtpConfig));

        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IJniMedia::REQUEST_MODIFY_SESSION, pParam) == IMS_TRUE)
        {
            m_nState = STATE_LIVE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL TextMediaSession::Close()
{
    IMS_TRACE_I("Close() - state[%d]", m_nState, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_TEXT);
        if (m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                    IJniMedia::REQUEST_CLOSE_SESSION, pParam) == IMS_TRUE)
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
    IMS_TRACE_I("SetMediaQuality() - state[%d]", m_nState, 0, 0);

    if (m_piMediaSessionListener != IMS_NULL && m_nState >= STATE_IDLE)
    {
        ImsMediaMsgSetMediaQualityParam* pParam =
                new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_TEXT);
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;

        return m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
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
    return m_pRtpConfig->getRemotePort();
}
