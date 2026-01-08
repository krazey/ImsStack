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

#include "IMediaSession.h"
#include "ISession.h"
#include "MediaDef.h"
#include "ServiceTrace.h"
#include "media/MtcMediaProfileManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcMediaProfileManager::MtcMediaProfileManager() :
        m_objMediaProfiles(ImsMap<const ISession*, MediaProfile*>())
{
    IMS_TRACE_D("+MtcMediaProfileManager", 0, 0, 0);
}

PUBLIC VIRTUAL MtcMediaProfileManager::~MtcMediaProfileManager()
{
    IMS_TRACE_D("~MtcMediaProfileManager", 0, 0, 0);

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        MediaProfile* pProfile = m_objMediaProfiles.GetValueAt(index);

        if (pProfile)
        {
            delete pProfile;
        }
    }
}

PUBLIC
void MtcMediaProfileManager::CreateMediaProfile(IN const ISession* piSession, IN IMS_BOOL bForked,
        IN IMS_BOOL bOriginalProfile, IN MEDIA_CONTENT_TYPE eMediaContents,
        IN IMediaSession* piMediaSession)
{
    IMS_TRACE_D("CreateMediaProfile", 0, 0, 0);

    if (!piSession)
    {
        return;
    }

    if (IsMediaProfilePresent(piSession))
    {
        return;
    }

    IMS_UINTP nParamId = 0;

    if (bOriginalProfile && bForked)  // not virtual session && forked session
    {
        for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
        {
            const MediaProfile* pProfile = m_objMediaProfiles.GetValueAt(index);

            if (pProfile->bForked == IMS_FALSE)
            {
                nParamId = pProfile->nNegoId;
                break;
            }
        }

        if (nParamId == 0)
        {
            IMS_TRACE_D("CreateMediaProfile : the call has %d media profiles.",
                    m_objMediaProfiles.GetSize(), 0, 0);

            const MediaProfile* pProfile = m_objMediaProfiles.GetValueAt(0);
            if (pProfile)
            {
                nParamId = pProfile->nNegoId;
            }
        }
    }

    MediaProfile* pProfile = new MediaProfile();
    pProfile->nNegoId = piMediaSession->CreateProfile(nParamId, eMediaContents);
    pProfile->bForked = bForked;
    pProfile->bOriginalProfile = bOriginalProfile;

    m_objMediaProfiles.Add(piSession, pProfile);
}

PUBLIC
void MtcMediaProfileManager::DestroyMediaProfile(
        IN const ISession* piSession, IN IMediaSession* piMediaSession)
{
    IMS_TRACE_D("DestroyMediaProfile", 0, 0, 0);

    // It can be called when the specific media profile should be destroyed.
    if (!IsMediaProfilePresent(piSession))
    {
        return;
    }

    if (piMediaSession)
    {
        piMediaSession->DestroyProfile(GetNegoId(piSession));
    }

    MediaProfile* pProfile = m_objMediaProfiles.GetValue(piSession);
    if (pProfile)
    {
        delete pProfile;
    }

    m_objMediaProfiles.Remove(piSession);
}

PUBLIC
void MtcMediaProfileManager::DestroyAllMediaProfiles(IN IMediaSession* piMediaSession)
{
    for (IMS_UINT32 i = static_cast<IMS_SINT32>(m_objMediaProfiles.GetSize()); i > 0; i--)
    {
        const ISession* piSession = m_objMediaProfiles.GetKeyAt(i - 1);
        DestroyMediaProfile(piSession, piMediaSession);
    }
}

PUBLIC
IMS_UINTP MtcMediaProfileManager::GetNegoId(IN const ISession* piSession) const
{
    const MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return UNDEFINED_NEGO_ID;
    }

    return pProfile->nNegoId;
}

PUBLIC
PemType MtcMediaProfileManager::GetPemType(IN const ISession* piSession) const
{
    const MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return PemType::NONE;
    }

    return pProfile->ePemType;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsActive(IN const ISession* piSession) const
{
    const MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bActive;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsConfirmed(IN const ISession* piSession) const
{
    const MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bConfirmed;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsForked(IN const ISession* piSession) const
{
    const MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bForked;
}

PUBLIC
void MtcMediaProfileManager::SetPemType(IN const ISession* piSession, IN PemType ePemType)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->ePemType = ePemType;
}

PUBLIC
void MtcMediaProfileManager::SetActive(IN const ISession* piSession, IN IMS_BOOL bActive)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->bActive = bActive;
}

PUBLIC
void MtcMediaProfileManager::SetConfirmed(IN const ISession* piSession, IN IMS_BOOL bConfirmed)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->bConfirmed = bConfirmed;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsPemSendInOtherEarlySession(IN const ISession* piSession) const
{
    IMS_BOOL bResult = IMS_FALSE;

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        const ISession* piTargetSession = m_objMediaProfiles.GetKeyAt(index);

        if (piTargetSession == piSession)
        {
            continue;
        }

        if (IsConfirmed(piTargetSession))
        {
            continue;
        }

        PemType ePemType = GetPemType(piTargetSession);

        if ((ePemType == PemType::SENDRECV || ePemType == PemType::SENDONLY) &&
                IsActive(piTargetSession))
        {
            bResult = IMS_TRUE;
            break;
        }
    }

    IMS_TRACE_D("IsPemSendInOtherEarlySession : %d", bResult, 0, 0);
    return bResult;
}

PUBLIC
void MtcMediaProfileManager::UpdateProfileForMediaActivation(IN const ISession* piActiveSession)
{
    IMS_TRACE_D("UpdateProfileForMediaActivation", 0, 0, 0);
    SetActive(piActiveSession, IMS_TRUE);

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        const ISession* piSession = m_objMediaProfiles.GetKeyAt(index);
        if (piSession == piActiveSession)
        {
            continue;
        }

        SetActive(piSession, IMS_FALSE);
    }
}

PUBLIC
ISession* MtcMediaProfileManager::GetSessionWithNegoId(IN IMS_UINTP nNegoId)
{
    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        const ISession* piSession = m_objMediaProfiles.GetKeyAt(index);

        if (nNegoId == GetNegoId(piSession))
        {
            return const_cast<ISession*>(piSession);
        }
    }

    IMS_TRACE_D("GetSessionWithNegoId : ISession is null.", 0, 0, 0);
    return IMS_NULL;
}

PUBLIC
ISession* MtcMediaProfileManager::GetActiveSession() const
{
    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        const ISession* piSession = m_objMediaProfiles.GetKeyAt(index);
        if (IsActive(piSession))
        {
            return const_cast<ISession*>(piSession);
        }
    }

    IMS_TRACE_D("GetActiveSession : There is no active session.", 0, 0, 0);
    return IMS_NULL;
}

PRIVATE
IMS_BOOL MtcMediaProfileManager::IsMediaProfilePresent(IN const ISession* piSession) const
{
    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        if (m_objMediaProfiles.GetKeyAt(index) == piSession)
        {
            IMS_TRACE_D("IsMediaProfilePresent : The media profile already exists.", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
MediaProfile* MtcMediaProfileManager::GetMediaProfile(IN const ISession* piSession) const
{
    IMS_SLONG nIndex = m_objMediaProfiles.GetIndexOfKey(piSession);
    return nIndex >= 0 ? m_objMediaProfiles.GetValueAt(nIndex) : IMS_NULL;
}
