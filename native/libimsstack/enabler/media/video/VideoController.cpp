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

#include "ServiceTrace.h"
#include "video/VideoController.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
VideoController::VideoController() :
        m_pSession(IMS_NULL),
        m_eCallState(EARLY_SESSION),
        m_objLocalAddr(IpAddress::IPv6NONE),
        m_nPort(0)
{
}

PUBLIC
VideoController::~VideoController()
{
    if (m_pSession != IMS_NULL)
    {
        delete m_pSession;
        m_pSession = IMS_NULL;
    }
}

PUBLIC
void VideoController::SetCallSessionState(IN IMS_BOOL bConfirmed)
{
    m_eCallState = (bConfirmed) ? CONFIRMED_SESSION : EARLY_SESSION;
}

PUBLIC
IMS_BOOL VideoController::SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    if (m_pSession != IMS_NULL)
    {
        return m_pSession->OnMessages(nMsg, pParam);
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::CreateSession(
        IMediaSessionListener* pListener, VideoConfiguration* pConfig)
{
    if (pListener == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession() - invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pSession == IMS_NULL)
    {
        IMS_TRACE_D("CreateSession()", 0, 0, 0);
        m_pSession = new VideoSession();
        m_pSession->SetMediaSessionListener(pListener);
        m_pSession->SetConfiguration(pConfig);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoController::OpenSession()
{
    if (m_pSession != IMS_NULL)
    {
        IMS_TRACE_D("OpenSession() - state[%d]", m_pSession->GetState(), 0, 0);

        if (m_pSession->GetState() == VideoSession::STATE_NONE)
        {
            m_pSession->SetLocalEndPoint(m_objLocalAddr, m_nPort);

            if (m_nPort > 0)
            {
                return m_pSession->Open();
            }
            else
            {
                IMS_TRACE_D("skip OpenSession() - Port is 0", 0, 0, 0);
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::UpdateSession()
{
    if (m_pSession != IMS_NULL && m_pSession->GetState() != VideoSession::STATE_NONE)
    {
        IMS_TRACE_D("UpdateSession() - state[%d]", m_pSession->GetState(), 0, 0);

        if (m_pSession->GetRemotePort() == 0 || m_pSession->GetLocalPort() == 0)
        {
            return CloseSession();
        }
        else
        {
            return m_pSession->Modify();
        }
    }

    IMS_TRACE_E(0, "UpdateSession() - invalid", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::CloseSession()
{
    if (m_pSession != IMS_NULL)
    {
        IMS_TRACE_D("CloseSession() - state[%d]", m_pSession->GetState(), 0, 0);

        if (m_pSession->GetState() != VideoSession::STATE_NONE)
        {
            m_pSession->Close();
            delete m_pSession;
            m_pSession = IMS_NULL;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::UpdateLocalAddress(IN std::shared_ptr<VideoNego> pNego)
{
    IMS_TRACE_I("UpdateLocalAddress()", 0, 0, 0);

    if (pNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateLocalAddress() - invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    m_objLocalAddr = pNego->GetLocalAddress();
    m_nPort = pNego->GetLocalPort();
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoController::UpdateRtpConfig(IN std::shared_ptr<VideoNego> pNego, IN IMS_BOOL bHold)
{
    if (pNego != IMS_NULL && m_pSession != IMS_NULL)
    {
        IMS_TRACE_I("UpdateRtpConfig()", 0, 0, 0);
        return m_pSession->UpdateRtpConfig(
                pNego->ProfileCasting(pNego->GetNegotiatedLocalProfile()),
                pNego->ProfileCasting(pNego->GetNegotiatedPeerProfile()),
                pNego->ProfileCasting(pNego->GetNegotiatedNegoProfile()),
                m_eCallState == CONFIRMED_SESSION, bHold);
    }

    IMS_TRACE_E(0, "UpdateRtpConfig() - invalid", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
void VideoController::UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork)
{
    IMS_TRACE_I("UpdateAccessNetwork() - accessNetwork[%d]", nAccessNetwork, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        m_pSession->SetAccessNetwork(nAccessNetwork);
    }
}

PUBLIC
void VideoController::SetMtu(IN IMS_SINT32 nMtu)
{
    IMS_TRACE_I("SetMtu() - mtu[%d]", nMtu, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        m_pSession->SetMtu(nMtu);
    }
}

PUBLIC
IMS_BOOL VideoController::ApplyQualityThreshold()
{
    if (m_pSession == IMS_NULL || m_pSession->GetState() == VideoSession::STATE_NONE)
    {
        IMS_TRACE_E(0, "ApplyQualityThreshold(): invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pSession->UpdateMediaQualityThreshold())
    {
        return m_pSession->SetMediaQuality();
    }

    IMS_TRACE_E(0, "ApplyQualityThreshold(): fail to update", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::IsSessionOpened()
{
    if (m_pSession != IMS_NULL && m_pSession->GetState() != VideoSession::STATE_NONE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void VideoController::SetMediaPemType(IN MEDIA_PEM_TYPE ePemType)
{
    if (m_pSession != IMS_NULL)
    {
        m_pSession->SetMediaPemType(ePemType);
    }
}
