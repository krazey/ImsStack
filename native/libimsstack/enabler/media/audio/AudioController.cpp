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

#include "audio/AudioController.h"
#include "audio/AudioProfile.h"

__IMS_TRACE_TAG_USER_DECL__("MED.AC");

PUBLIC
AudioController::AudioController() :
        m_nAudioSessionState(AudioMediaSession::STATE_NONE),
        m_eUpdateCondition(EARLY_SESSION),
        m_nPort(0)
{
    m_listAudioSession.Clear();
}

PUBLIC
AudioController::~AudioController()
{
    IMS_TRACE_I("~AudioController() - state[%d]", m_nAudioSessionState, 0, 0);

    if (m_nAudioSessionState != AudioMediaSession::STATE_NONE)
    {
        CloseSession();
    }

    ClearSession();
}

PUBLIC
IMS_BOOL AudioController::IsHoldSession(IN IMS_UINTP nNegoId)
{
    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != IMS_NULL)
    {
        return pAudioSession->IsDirectionHold();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::HoldSession()
{
    IMS_TRACE_D("HoldSession()", 0, 0, 0);

    AudioMediaSession* pAudioSession = FindAudioSession();

    if (pAudioSession != IMS_NULL)
    {
        pAudioSession->UpdateMediaQualityThreshold(IMS_TRUE, IMS_FALSE);
        pAudioSession->SetMediaQuality();
        pAudioSession->HoldRtpConfig();
        pAudioSession->Modify();
    }

    return IMS_FALSE;
}

PUBLIC
void AudioController::SetConfirmSession(IN IMS_BOOL bConfirmed)
{
    if (bConfirmed)
    {
        m_eUpdateCondition = READY_TO_CONFIRM;
    }
    else
    {
        m_eUpdateCondition = EARLY_SESSION;
    }
}

PUBLIC
IMS_BOOL AudioController::SendDtmf(IN IMS_CHAR cDtmfCode)
{
    IMS_TRACE_D("SendDtmf() - code[%c]", cDtmfCode, 0, 0);

    AudioMediaSession* pAudioSession = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->GetState() == AudioMediaSession::STATE_LIVE)
        {
            pAudioSession->SendDtmf(cDtmfCode);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioController::CreateSession(
        IN IMediaSessionListener* pListener, IN IMS_UINTP nNegoId, AudioConfiguration* pConfig)
{
    IMS_TRACE_D("CreateSession() - nNegoId[%" PFLS_x "], audio list size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    if (pListener == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession() - invalid params", 0, 0, 0);
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = new AudioMediaSession();
    pAudioSession->SetNegoId(nNegoId);
    pAudioSession->SetMediaSessionListener(pListener);
    pAudioSession->SetConfig(pConfig);
    m_listAudioSession.Append(pAudioSession);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioController::UpdateSession(IN IMS_UINTP nNegoId)
{
    if (m_eUpdateCondition == READY_TO_CONFIRM && m_listAudioSession.GetSize() > 1)
    {
        return ConfirmSession(nNegoId);
    }
    else
    {
        return ModifySession(nNegoId);
    }
}

PUBLIC
IMS_BOOL AudioController::OpenSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_I("OpenSession() - nNegoId[%" PFLS_x "], port[%d]", nNegoId, m_nPort, 0);

    if (m_nPort == 0)
    {
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != IMS_NULL && m_nAudioSessionState == AudioMediaSession::STATE_NONE)
    {
        pAudioSession->SetLocalEndPoint(m_objLocalAddr, m_nPort);

        if (pAudioSession->Open() == IMS_TRUE)
        {
            m_nAudioSessionState = AudioMediaSession::STATE_IDLE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::AddSession(IN IMS_UINTP nNegoId, IN AudioNego* pNego)
{
    IMS_TRACE_I("AddSession() - nNegoId[%" PFLS_x "], audio list size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    if (m_listAudioSession.GetSize() == 1)
    {
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    IMS_BOOL bResult = IMS_FALSE;

    if (pAudioSession != IMS_NULL && m_nAudioSessionState != AudioMediaSession::STATE_NONE)
    {
        UpdateQualityThreshold(nNegoId, pNego);
        pAudioSession->SetMediaQuality();
        bResult = pAudioSession->Add();
    }

    if (bResult == IMS_TRUE)
    {
        for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
        {
            pAudioSession = m_listAudioSession.GetAt(nIndex);

            if (pAudioSession != IMS_NULL)
            {
                if (pAudioSession->IsSameNegoId(nNegoId) == IMS_FALSE)
                {
                    pAudioSession->SetState(AudioMediaSession::STATE_PAUSED);
                }
            }
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioController::ConfirmSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ConfirmSession() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    if (m_listAudioSession.GetSize() <= 1)
    {
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession == NULL)
    {
        return IMS_FALSE;
    }

    pAudioSession->SetMediaQuality();

    if (pAudioSession->Confirm() == IMS_TRUE)
    {
        IMS_SINT32 nIndex = 0;

        while (m_listAudioSession.GetSize() > nIndex)
        {
            pAudioSession = m_listAudioSession.GetAt(nIndex);

            if (pAudioSession == IMS_NULL)
            {
                IMS_TRACE_E(0, "ConfirmSession() - invalid pAudioSession", 0, 0, 0);
                m_listAudioSession.RemoveAt(nIndex);
            }
            else if (pAudioSession->IsSameNegoId(nNegoId) == IMS_FALSE)
            {
                delete pAudioSession;
                pAudioSession = IMS_NULL;
                m_listAudioSession.RemoveAt(nIndex);
            }
            else
            {
                nIndex++;
            }
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::ModifySession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ModifySession() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != NULL)
    {
        pAudioSession->SetMediaQuality();
        return pAudioSession->Modify();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::DeleteSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteSession() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    if (nNegoId == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->IsSameNegoId(nNegoId) == IMS_TRUE)
        {
            delete pAudioSession;
            pAudioSession = IMS_NULL;
            m_listAudioSession.RemoveAt(nIndex);
            return IMS_TRUE;
        }
    }

    IMS_TRACE_E(0, "DeleteSession() - Nothing matched with this NegoId", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::CloseSession()
{
    IMS_TRACE_I("CloseSession() - state[%d]", m_nAudioSessionState, 0, 0);
    AudioMediaSession* pAudioSession = FindAudioSession();

    if (pAudioSession != IMS_NULL)
    {
        if (pAudioSession->Close() == IMS_TRUE)
        {
            m_nAudioSessionState = AudioMediaSession::STATE_NONE;
            ClearSession();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::UpdateRtpConfig(IN IMS_UINTP nNegoId, IN AudioNego* pNego)
{
    IMS_TRACE_D("UpdateRtpConfig() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    if (pNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateRtpConfig() - invalid param", 0, 0, 0);
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != IMS_NULL)
    {
        return pAudioSession->UpdateRtpConfig(pNego->GetNegotiatedLocalProfile(),
                pNego->GetNegotiatedPeerProfile(), pNego->GetNegotiatedNegoProfile());
    }

    IMS_TRACE_E(0, "UpdateRtpConfig() - invalid param", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::UpdateLocalAddress(IN AudioNego* pNego)
{
    if (pNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_objLocalAddr = pNego->GetLocalAddress();
    m_nPort = pNego->GetLocalPort();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioController::UpdateQualityThreshold(IN IMS_UINTP nNegoId, IN AudioNego* pNego)
{
    IMS_TRACE_I("UpdateQualityThreshold() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != IMS_NULL && pNego != IMS_NULL)
    {
        AudioProfile* pPeerProfile = pNego->GetNegotiatedPeerProfile();
        IMS_BOOL bEnableRtcp = IMS_TRUE;

        if (pPeerProfile != IMS_NULL && pPeerProfile->nBandwidthRs == 0 &&
                pPeerProfile->nBandwidthRr == 0)
        {
            bEnableRtcp = IMS_FALSE;
        }

        return pAudioSession->UpdateMediaQualityThreshold(IsHoldSession(nNegoId), bEnableRtcp);
    }

    return IMS_FALSE;
}

PUBLIC
IMS_UINT32 AudioController::GetAudioSessionSize()
{
    return m_listAudioSession.GetSize();
}

PRIVATE
AudioMediaSession* AudioController::FindAudioSession(IN IMS_UINTP nNegoId)
{
    if (m_listAudioSession.IsEmpty() == IMS_TRUE)
    {
        return IMS_NULL;
    }

    if (nNegoId == IMS_NULL)
    {
        return m_listAudioSession.GetAt(0);
    }

    AudioMediaSession* pAudioSession = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->IsSameNegoId(nNegoId) == IMS_TRUE)
        {
            return pAudioSession;
        }
    }

    return IMS_NULL;
}

PRIVATE
void AudioController::ClearSession()
{
    IMS_TRACE_D("ClearSession() size[%d]", m_listAudioSession.GetSize(), 0, 0);

    AudioMediaSession* pAudioSession = IMS_NULL;

    while (m_listAudioSession.GetSize() > 0)
    {
        pAudioSession = m_listAudioSession.GetValueAt(0);

        if (pAudioSession != IMS_NULL)
        {
            delete pAudioSession;
            pAudioSession = IMS_NULL;
        }

        m_listAudioSession.RemoveAt(0);
    }

    m_listAudioSession.Clear();
}