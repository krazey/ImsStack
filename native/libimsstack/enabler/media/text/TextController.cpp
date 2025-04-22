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
#include "text/TextController.h"
#include "text/TextProfile.h"
#include "text/TextSession.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
TextController::TextController() :
        m_pSession(IMS_NULL),
        m_nPort(0)
{
}

PUBLIC
TextController::~TextController()
{
    if (m_pSession != NULL)
    {
        delete m_pSession;
        m_pSession = NULL;
    }
}

PUBLIC
IMS_BOOL TextController::CreateSession(IMediaSessionListener* pListener, TextConfiguration* pConfig)
{
    if (pListener == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession() - invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pSession == IMS_NULL)
    {
        IMS_TRACE_D("CreateSession()", 0, 0, 0);
        m_pSession = new TextSession();
        m_pSession->SetMediaSessionListener(pListener);
        m_pSession->SetConfiguration(pConfig);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL TextController::OpenSession()
{
    if (m_pSession != IMS_NULL)
    {
        IMS_TRACE_D("OpenSession() - state[%d]", m_pSession->GetState(), 0, 0);

        if (m_pSession->GetState() == TextSession::STATE_NONE)
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
IMS_BOOL TextController::UpdateSession()
{
    if (m_pSession != IMS_NULL && m_pSession->GetState() != TextSession::STATE_NONE)
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

    IMS_TRACE_E(0, "UpdateSession() - invalid", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL TextController::CloseSession()
{
    if (m_pSession != IMS_NULL)
    {
        IMS_TRACE_D("CloseSession() - state[%d]", m_pSession->GetState(), 0, 0);

        if (m_pSession->GetState() != TextSession::STATE_NONE)
        {
            m_pSession->Close();
        }

        delete m_pSession;
        m_pSession = IMS_NULL;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL TextController::UpdateLocalAddress(IN std::shared_ptr<TextNego> pNego)
{
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
IMS_BOOL TextController::UpdateRtpConfig(IN std::shared_ptr<TextNego> pNego)
{
    if (pNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateRtpConfig() - invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pSession != IMS_NULL)
    {
        return m_pSession->UpdateRtpConfig(
                pNego->ProfileCasting(pNego->GetNegotiatedLocalProfile()),
                pNego->ProfileCasting(pNego->GetNegotiatedPeerProfile()),
                pNego->ProfileCasting(pNego->GetNegotiatedNegoProfile()));
    }

    return IMS_FALSE;
}

PUBLIC
void TextController::UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork)
{
    IMS_TRACE_I("UpdateAccessNetwork() - accessNetwork[%d]", nAccessNetwork, 0, 0);

    if (m_pSession != IMS_NULL)
    {
        m_pSession->SetAccessNetwork(nAccessNetwork);
    }
}

PUBLIC
IMS_BOOL TextController::UpdateQualityThreshold(IN std::shared_ptr<TextNego> pNego)
{
    if (m_pSession == IMS_NULL || pNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateQualityThreshold() - invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    TextProfile* pPeerProfile = pNego->ProfileCasting(pNego->GetNegotiatedPeerProfile());
    IMS_BOOL bEnableRtcp = IMS_TRUE;

    if (pPeerProfile != IMS_NULL && pPeerProfile->GetBandwidthRs() == 0 &&
            pPeerProfile->GetBandwidthRr() == 0)
    {
        bEnableRtcp = IMS_FALSE;
    }

    return m_pSession->UpdateMediaQualityThreshold(IMS_TRUE, bEnableRtcp);
}

PUBLIC
IMS_BOOL TextController::IsSessionOpened()
{
    if (m_pSession != NULL && m_pSession->GetState() != TextSession::STATE_NONE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
