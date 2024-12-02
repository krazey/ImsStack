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

#include "ServiceTrace.h"
#include "BaseSession.h"
#include "MediaEnvironment.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
BaseSession::BaseSession(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_pConfiguration(IMS_NULL),
        m_objLocalAddress(IpAddress::IPv6NONE),
        m_nLocalPort(0),
        m_piMediaSessionListener(IMS_NULL),
        m_pEnvironment(IMS_NULL),
        m_pRtpConfig(IMS_NULL),
        m_ePrevDirection(MEDIA_DIRECTION_INVALID),
        m_nState(0)
{
}

PUBLIC VIRTUAL BaseSession::~BaseSession() {}

PUBLIC
void BaseSession::SetConfiguration(IN MediaConfiguration* pConfiguration)
{
    m_pConfiguration = pConfiguration;
}

PUBLIC VIRTUAL void BaseSession::SetServiceType(MEDIA_SERVICE_TYPE eServiceType)
{
    m_pEnvironment->eServiceType = eServiceType;
}

PUBLIC VIRTUAL void BaseSession::SetMediaSessionListener(IN IMediaSessionListener* pListener)
{
    m_piMediaSessionListener = pListener;
}

PUBLIC VIRTUAL void BaseSession::SetMediaEnvironment(MediaEnvironment* pEnvironment)
{
    m_pEnvironment = pEnvironment;
}

PUBLIC
void BaseSession::SetDirection(IN MEDIA_DIRECTION eDirection)
{
    IMS_TRACE_D("SetDirection() - eDirection[%d]", eDirection, 0, 0);
    SetPrevDirection(GetDirection());

    switch (eDirection)
    {
        case MEDIA_DIRECTION_INVALID:
            m_pRtpConfig->setMediaDirection(RtpConfig::MEDIA_DIRECTION_NO_FLOW);
            break;
        case MEDIA_DIRECTION_SEND:
            m_pRtpConfig->setMediaDirection(RtpConfig::MEDIA_DIRECTION_SEND_ONLY);
            break;
        case MEDIA_DIRECTION_RECEIVE:
            m_pRtpConfig->setMediaDirection(RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
            break;
        case MEDIA_DIRECTION_SEND_RECEIVE:
            m_pRtpConfig->setMediaDirection(RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
            break;
        case MEDIA_DIRECTION_INACTIVE:
            m_pRtpConfig->setMediaDirection(RtpConfig::MEDIA_DIRECTION_INACTIVE);
            break;
    }
}

PUBLIC
MEDIA_DIRECTION BaseSession::GetDirection()
{
    IMS_UINT32 nDirection = m_pRtpConfig->getMediaDirection();

    switch (nDirection)
    {
        case RtpConfig::MEDIA_DIRECTION_NO_FLOW:
            return MEDIA_DIRECTION_INVALID;
        case RtpConfig::MEDIA_DIRECTION_SEND_ONLY:
            return MEDIA_DIRECTION_SEND;
        case RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY:
            return MEDIA_DIRECTION_RECEIVE;
        case RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE:
            return MEDIA_DIRECTION_SEND_RECEIVE;
        case RtpConfig::MEDIA_DIRECTION_INACTIVE:
            return MEDIA_DIRECTION_INACTIVE;
    }

    return MEDIA_DIRECTION_INVALID;
}

PUBLIC
void BaseSession::SetPrevDirection(MEDIA_DIRECTION eDir)
{
    m_ePrevDirection = eDir;
}

PUBLIC
MEDIA_DIRECTION BaseSession::GetPrevDirection()
{
    return m_ePrevDirection;
}

PUBLIC VIRTUAL IMS_SINT32 BaseSession::GetState()
{
    return m_nState;
}

PUBLIC VIRTUAL void BaseSession::SetState(IMS_SINT32 state)
{
    m_nState = state;
}

PUBLIC
void BaseSession::SetAnbrMode(AnbrMode anbrMode)
{
    IMS_TRACE_D("SetAnbrMode() - uplink codec mode[%d] downlink codec mode[%d]",
            anbrMode.getAnbrUplinkCodecMode(), anbrMode.getAnbrDownlinkCodecMode(), 0);
    m_pRtpConfig->setAnbrMode(anbrMode);
}

PUBLIC
void BaseSession::UpdateLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort)
{
    if (!objLocalAddr.ToString().IsNULL())
    {
        m_objLocalAddress = objLocalAddr;
    }

    m_nLocalPort = nPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);
}
