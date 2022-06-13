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
        m_pVideoSession(IMS_NULL),
        m_nPort(0)
{
}

PUBLIC
VideoController::~VideoController()
{
    if (m_pVideoSession != NULL)
    {
        delete m_pVideoSession;
        m_pVideoSession = NULL;
    }
}

PUBLIC
IMS_BOOL VideoController::SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    if (m_pVideoSession != IMS_NULL)
    {
        IMS_BOOL bRet = m_pVideoSession->OnVideoMessages(nMsg, pParam);

        if (bRet == IMS_TRUE && nMsg == IMMedia::SELECT_CAMERA_CMD)
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
    if (m_pVideoSession != IMS_NULL)
    {
        return m_pVideoSession->IsDirectionHold();
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL VideoController::HoldSession()
{
    IMS_TRACE_D("HoldSession()", 0, 0, 0);

    if (m_pVideoSession != IMS_NULL)
    {
        m_pVideoSession->UpdateMediaQualityThreshold(IMS_TRUE);
        m_pVideoSession->SetMediaQuality();
        m_pVideoSession->HoldRtpConfig();
        m_pVideoSession->Modify();
    }

    return IMS_FALSE;
}

PUBLIC
void VideoController::CreateSession(IMediaSessionListener* pListener, VideoConfiguration* pConfig)
{
    if (m_pVideoSession == IMS_NULL)
    {
        IMS_TRACE_D("CreateSession()", 0, 0, 0);
        m_pVideoSession = new VideoMediaSession();
        m_pVideoSession->SetMediaSessionListener(pListener);
        m_pVideoSession->SetConfig(pConfig);
    }
}

PUBLIC
IMS_BOOL VideoController::OpenSession()
{
    IMS_TRACE_D("OpenSession()", 0, 0, 0);

    if (m_pVideoSession != IMS_NULL &&
            m_pVideoSession->GetState() == VideoMediaSession::STATE_IDLE &&
            m_pVideoSession->GetCameraId() >= 0)
    {
        m_pVideoSession->UpdateLocalEndPoint(m_objLocalAddr, m_nPort);
        return m_pVideoSession->Open();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::UpdateSession()
{
    IMS_TRACE_D("UpdateSession()", 0, 0, 0);

    if (m_pVideoSession != IMS_NULL)
    {
        if (m_pVideoSession->GetRemotePort() == 0 || m_pVideoSession->GetLocalPort() == 0)
        {
            return CloseSession();
        }
        else
        {
            m_pVideoSession->SetMediaQuality();
            m_pVideoSession->Modify();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL VideoController::CloseSession()
{
    IMS_TRACE_D("CloseSession()", 0, 0, 0);

    if (m_pVideoSession != IMS_NULL)
    {
        if (m_pVideoSession->GetState() != VideoMediaSession::STATE_IDLE)
        {
            m_pVideoSession->Close();
        }

        delete m_pVideoSession;
        m_pVideoSession = IMS_NULL;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void VideoController::UpdateLocalAddress(IN VideoNego* pNego)
{
    IMS_TRACE_I("UpdateLocalAddress()", 0, 0, 0);

    if (pNego == IMS_NULL)
    {
        return;
    }

    m_objLocalAddr = pNego->GetLocalAddr();
    m_nPort = pNego->GetLocalPort();
}

PUBLIC
void VideoController::UpdateRtpConfig(IN VideoNego* pNego)
{
    IMS_TRACE_I("UpdateRtpConfig()", 0, 0, 0);

    if (pNego == IMS_NULL)
    {
        return;
    }

    VideoProfile* pSrcProfile = IMS_NULL;
    VideoProfile* pDestProfile = IMS_NULL;
    VideoProfile* pNegoProfile = IMS_NULL;

    if (pNego->GetNegotiatedProfileSet(pSrcProfile, pDestProfile, pNegoProfile) == IMS_TRUE)
    {
        if (m_pVideoSession != IMS_NULL)
        {
            m_pVideoSession->UpdateRtpConfig(pSrcProfile, pDestProfile, pNegoProfile);
        }
    }
}

PUBLIC
void VideoController::UpdateQualityThreshold()
{
    IMS_TRACE_I("UpdateQualityThreshold()", 0, 0, 0);

    if (m_pVideoSession != IMS_NULL)
    {
        m_pVideoSession->UpdateMediaQualityThreshold(m_pVideoSession->IsDirectionHold());
    }
}