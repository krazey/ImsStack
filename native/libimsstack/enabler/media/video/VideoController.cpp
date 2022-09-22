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

#include "video/VideoController.h"
#include "ServiceTrace.h"
#include "video/VideoProfile.h"

__IMS_TRACE_TAG_USER_DECL__("MED.VC");

PUBLIC
VideoController::VideoController() :
        m_pSession(IMS_NULL),
        m_nPort(0)
{
}

PUBLIC
VideoController::~VideoController()
{
    if (m_pSession != NULL)
    {
        delete m_pSession;
        m_pSession = NULL;
    }
}

PUBLIC
IMS_BOOL VideoController::SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    if (m_pSession != IMS_NULL)
    {
        IMS_BOOL bRet = m_pSession->OnMessages(nMsg, pParam);

        if (bRet == IMS_TRUE && nMsg == IMMedia::SELECT_CAMERA_CMD &&
                m_pSession->GetState() == STATE_IDLE)
        {
            OpenSession();
        }

        return bRet;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::IsHoldSession()
{
    if (m_pSession != IMS_NULL)
    {
        return m_pSession->IsDirectionHold();
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoController::HoldSession()
{
    IMS_TRACE_D("HoldSession()", 0, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        m_pSession->UpdateMediaQualityThreshold(IMS_TRUE, IMS_TRUE);
        m_pSession->SetMediaQuality();
        m_pSession->HoldRtpConfig();
        return m_pSession->Modify();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::CreateSession(
        IMediaSessionListener* pListener, VideoConfiguration* pConfig)
{
    if (pListener == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_pSession == IMS_NULL)
    {
        IMS_TRACE_D("CreateSession()", 0, 0, 0);
        m_pSession = new VideoMediaSession();
        m_pSession->SetMediaSessionListener(pListener);
        m_pSession->SetConfig(pConfig);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::OpenSession()
{
    IMS_TRACE_D("OpenSession()", 0, 0, 0);

    if (m_pSession != IMS_NULL && m_pSession->GetState() == VideoMediaSession::STATE_IDLE &&
            m_pSession->GetCameraId() != VideoMediaSession::CAMERA_ID_NONE)
    {
        m_pSession->UpdateLocalEndPoint(m_objLocalAddr, m_nPort);
        return m_pSession->Open();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::UpdateSession()
{
    IMS_TRACE_D("UpdateSession()", 0, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        if (m_pSession->GetRemotePort() == 0 || m_pSession->GetLocalPort() == 0)
        {
            return CloseSession();
        }
        else
        {
            m_pSession->SetMediaQuality();
            return m_pSession->Modify();
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::CloseSession()
{
    IMS_TRACE_D("CloseSession()", 0, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        if (m_pSession->GetState() != VideoMediaSession::STATE_IDLE)
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
IMS_BOOL VideoController::UpdateLocalAddress(IN VideoNego* pNego)
{
    IMS_TRACE_I("UpdateLocalAddress()", 0, 0, 0);

    if (pNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_objLocalAddr = pNego->GetLocalAddress();
    m_nPort = pNego->GetLocalPort();
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoController::UpdateRtpConfig(IN VideoNego* pNego)
{
    IMS_TRACE_I("UpdateRtpConfig()", 0, 0, 0);

    if (pNego != NULL && m_pSession != IMS_NULL)
    {
        return m_pSession->UpdateRtpConfig(pNego->GetNegotiatedLocalProfile(),
                pNego->GetNegotiatedPeerProfile(), pNego->GetNegotiatedNegoProfile());
    }

    return IMS_FALSE;
}

PUBLIC
void VideoController::UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork)
{
    IMS_TRACE_I("UpdateAccessNetwork() - accessNetwork[%d]", nAccessNetwork, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        m_pSession->UpdateAccessNetwork(nAccessNetwork);
    }
}

PUBLIC
IMS_BOOL VideoController::UpdateQualityThreshold(IN VideoNego* pNego)
{
    IMS_TRACE_I("UpdateQualityThreshold()", 0, 0, 0);

    if (m_pSession == IMS_NULL || pNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    VideoProfile* pPeerProfile = pNego->GetNegotiatedPeerProfile();
    IMS_BOOL bEnableRtcp = IMS_TRUE;

    if (pPeerProfile != IMS_NULL && pPeerProfile->nBandwidthRs == 0 &&
            pPeerProfile->nBandwidthRr == 0)
    {
        bEnableRtcp = IMS_FALSE;
    }

    return m_pSession->UpdateMediaQualityThreshold(m_pSession->IsDirectionHold(), bEnableRtcp);
}

PUBLIC
IMS_BOOL VideoController::IsSessionOpened()
{
    if (m_pSession != NULL)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}