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

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
AudioController::AudioController() :
        m_listAudioSession(ImsList<AudioMediaSession*>()),
        m_nAudioSessionState(AudioMediaSession::STATE_NONE),
        m_eUpdateCondition(EARLY_SESSION),
        m_objLocalAddr(IpAddress::IPv6NONE),
        m_nPort(0),
        m_nCurrentActiveNegoId(UNDEFINED_NEGO_ID)
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
void AudioController::SetConfirmSession(IN IMS_BOOL bConfirmed)
{
    if (bConfirmed)
    {
        m_eUpdateCondition = CONFIRMED_SESSION;
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

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->GetState() == AudioMediaSession::STATE_LIVE)
        {
            pAudioSession->SendDtmf(cDtmfCode);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioController::CreateSession(IN IMediaSessionListener* pListener, IN IMS_UINTP nNegoId,
        AudioConfiguration* pConfig, MEDIA_SERVICE_TYPE eServiceType)
{
    IMS_TRACE_D("CreateSession() - NegoId[%" PFLS_x "], audio list size[%d] ServiceType[%d]",
            nNegoId, m_listAudioSession.GetSize(), eServiceType);

    if (pListener == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession() - invalid params", 0, 0, 0);
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = new AudioMediaSession();
    pAudioSession->SetServiceType(eServiceType);
    pAudioSession->SetNegoId(nNegoId);
    pAudioSession->SetMediaSessionListener(pListener);
    pAudioSession->SetConfig(pConfig);
    m_listAudioSession.Append(pAudioSession);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioController::UpdateSession(
        IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork, IN AudioNego* pNego)
{
    IMS_TRACE_I("UpdateSession() - nNegoId[%" PFLS_x "], nAccessNetwork[%d]", nNegoId,
            nAccessNetwork, 0);

    m_nCurrentActiveNegoId = nNegoId;

    IMS_BOOL bAnbrResult = UpdateAnbrEnabledConfig(
            nNegoId, pNego->ProfileCasting(pNego->GetNegotiatedNegoProfile())->IsAnbrSupported());
    IMS_TRACE_D("UpdateSession() - res: %d, anbr enable: %d", bAnbrResult,
            pNego->ProfileCasting(pNego->GetNegotiatedNegoProfile())->IsAnbrSupported(), 0);

    if (m_eUpdateCondition == READY_TO_CONFIRM && m_listAudioSession.GetSize() > 1)
    {
        UpdateRtpConfig(nNegoId, nAccessNetwork, pNego);
        UpdateQualityThreshold(nNegoId, pNego);
        return ConfirmSession(nNegoId);
    }
    else
    {
        IMS_BOOL bResult = UpdateRtpConfig(nNegoId, nAccessNetwork, pNego);
        UpdateQualityThreshold(nNegoId, pNego);
        SetMediaQuality(nNegoId);

        if (bResult)
        {
            return ModifySession(nNegoId);
        }
    }

    return IMS_FALSE;
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
IMS_BOOL AudioController::AddSession(
        IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork, IN AudioNego* pNego)
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
        if (UpdateRtpConfig(nNegoId, nAccessNetwork, pNego) == IMS_TRUE)
        {
            UpdateQualityThreshold(nNegoId, pNego);
            pAudioSession->SetMediaQuality();
            bResult = pAudioSession->Add();
        }
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
                m_listAudioSession.RemoveAt(nIndex);
            }
            else
            {
                nIndex++;
            }
        }

        m_eUpdateCondition = CONFIRMED_SESSION;
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
        return pAudioSession->Modify();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::SetMediaQuality(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("SetMediaQuality() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId, 0, 0);

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != NULL)
    {
        return pAudioSession->SetMediaQuality();
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::DeleteSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteSession() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_listAudioSession.GetSize(), 0);

    if (nNegoId == UNDEFINED_NEGO_ID)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->IsSameNegoId(nNegoId) == IMS_TRUE)
        {
            delete pAudioSession;
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
IMS_BOOL AudioController::UpdateRtpConfig(
        IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork, IN AudioNego* pNego)
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
        return pAudioSession->UpdateRtpConfig(nAccessNetwork,
                pNego->ProfileCasting(pNego->GetNegotiatedLocalProfile()),
                pNego->ProfileCasting(pNego->GetNegotiatedPeerProfile()),
                pNego->ProfileCasting(pNego->GetNegotiatedNegoProfile()));
    }

    IMS_TRACE_E(0, "UpdateRtpConfig() - invalid param", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::UpdateAnbrEnabledConfig(IN IMS_UINTP nNegoId, IN IMS_BOOL anbrEnabled)
{
    IMS_TRACE_D("UpdateAnbrEnabledConfig() - nNegoId[%" PFLS_x "], anbr enable[%d]", nNegoId,
            anbrEnabled, 0);

    if (nNegoId == UNDEFINED_NEGO_ID)
    {
        IMS_TRACE_E(0, "UpdateAnbrEnabledConfig() - invalid param", 0, 0, 0);
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    if (pAudioSession != IMS_NULL)
    {
        return pAudioSession->UpdateAnbrEnabledConfig(anbrEnabled);
    }

    IMS_TRACE_E(0, "UpdateAnbrEnabledConfig() - invalid param", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::NotifyAnbrReceived(
        IN IMS_UINT32 anbrMediaType, IN IMS_UINT32 anbrDirection, IN IMS_UINT32 anbrBitrate)
{
    IMS_TRACE_D("NotifyAnbrReceived() - anbr bitrate[%d]", anbrBitrate, 0, 0);

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->GetState() == AudioMediaSession::STATE_LIVE)
        {
            return pAudioSession->NotifyAnbrReceived(anbrMediaType, anbrDirection, anbrBitrate);
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioController::UpdateAccessNetwork(IN IMS_UINT32 accessNetwork)
{
    IMS_TRACE_I("UpdateAccessNetwork() - accessNetwork[%d]", accessNetwork, 0, 0);

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession != IMS_NULL && pAudioSession->GetState() == AudioMediaSession::STATE_LIVE)
        {
            pAudioSession->UpdateAccessNetwork(accessNetwork);
            return pAudioSession->Modify();
        }
    }

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
        AudioProfile* pPeerProfile = pNego->ProfileCasting(pNego->GetNegotiatedPeerProfile());
        IMS_BOOL bEnableRtcp = IMS_TRUE;

        if (pPeerProfile != IMS_NULL && pPeerProfile->GetBandwidthRs() == 0 &&
                pPeerProfile->GetBandwidthRr() == 0)
        {
            bEnableRtcp = IMS_FALSE;
        }

        return pAudioSession->UpdateMediaQualityThreshold(
                !MEDIA_DIRECTION_IS_AUDIO_HOLD(pAudioSession->GetDirection()), m_eUpdateCondition,
                bEnableRtcp);
    }

    return IMS_FALSE;
}

PUBLIC
IMS_UINT32 AudioController::GetAudioSessionSize()
{
    return m_listAudioSession.GetSize();
}

PUBLIC
IMS_BOOL AudioController::UpdateMediaDirection(MEDIA_DIRECTION eDirection, IMS_BOOL bRestore)
{
    IMS_BOOL bRet = IMS_FALSE;

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetAt(nIndex);

        if (pAudioSession == IMS_NULL || pAudioSession->GetState() < AudioMediaSession::STATE_LIVE)
        {
            continue;
        }

        MEDIA_DIRECTION eTempDirection = eDirection;

        if (bRestore)
        {
            eTempDirection = pAudioSession->GetPrevDirection();

            if (eTempDirection == MEDIA_DIRECTION_INVALID)
            {
                eTempDirection = pAudioSession->GetDirection();
            }
        }

        IMS_TRACE_I("UpdateMediaDirection() - bRestore[%d], PrevDirection[%d], eTempDirection[%d]",
                bRestore, pAudioSession->GetPrevDirection(), eTempDirection);

        pAudioSession->SetDirection(eTempDirection);
        bRet = pAudioSession->Modify();

        if (bRestore)
        {
            pAudioSession->SetPrevDirection(MEDIA_DIRECTION_INVALID);
        }
    }

    return bRet;
}

PUBLIC void AudioController::SetNetworkToneTimer(IN IMS_UINTP nNegoId, IN IMS_UINT32 nTimer)
{
    IMS_TRACE_I("SetNetworkToneTimer() - NegoId[%" PFLS_x "], CurrentNegoId[%" PFLS_x
                "], timer[%d]",
            nNegoId, m_nCurrentActiveNegoId, nTimer);

    if (nNegoId == UNDEFINED_NEGO_ID)
    {
        nNegoId = m_nCurrentActiveNegoId;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);
    if (pAudioSession != IMS_NULL)
    {
        pAudioSession->SetNetworkToneTimer(nTimer);
    }
}

PUBLIC IMS_SINT32 AudioController::GetInactivityTimer(
        IN InactivitytimerType eType, IN IMS_UINTP nNegoId)
{
    IMS_TRACE_I("GetInactivityTimer() - Type[%d], nNegoId[%" PFLS_x "], CurrentNegoId[%" PFLS_x "]",
            eType, nNegoId, m_nCurrentActiveNegoId);

    if (nNegoId == UNDEFINED_NEGO_ID)
    {
        nNegoId = m_nCurrentActiveNegoId;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);
    if (pAudioSession != IMS_NULL)
    {
        return pAudioSession->GetInactivityTimer(eType);
    }

    return -1;
}

PRIVATE
AudioMediaSession* AudioController::FindAudioSession(IN IMS_UINTP nNegoId)
{
    if (m_listAudioSession.IsEmpty() == IMS_TRUE)
    {
        return IMS_NULL;
    }

    if (nNegoId == UNDEFINED_NEGO_ID)
    {
        return m_listAudioSession.GetAt(0);
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetAt(nIndex);

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

    while (m_listAudioSession.GetSize() > 0)
    {
        AudioMediaSession* pAudioSession = m_listAudioSession.GetValueAt(0);

        if (pAudioSession != IMS_NULL)
        {
            delete pAudioSession;
        }

        m_listAudioSession.RemoveAt(0);
    }

    m_listAudioSession.Clear();
}
