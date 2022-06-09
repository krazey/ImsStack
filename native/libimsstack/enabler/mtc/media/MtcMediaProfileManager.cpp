#include "media/MtcMediaProfileManager.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcMediaProfileManager::MtcMediaProfileManager() :
        m_objMediaProfiles(IMSMap<ISession*, MediaProfile*>())
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
void MtcMediaProfileManager::CreateMediaProfile(IN ISession* piSession, IN IMS_BOOL bForked,
        IN IMS_BOOL bOriginalProfile, IN MEDIA_CONTENT_TYPE eMediaContents,
        IN IMediaSession* piMediaSession)
{
    IMS_TRACE_D("CreateMediaProfile", 0, 0, 0);

    if (IsMediaProfilePresent(piSession))
    {
        return;
    }

    IMS_UINTP nParamId = 0;

    if (bForked == IMS_TRUE)  // forked session
    {
        for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
        {
            MediaProfile* pProfile = m_objMediaProfiles.GetValueAt(index);

            if (!pProfile)
            {
                continue;
            }

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

            MediaProfile* pProfile = m_objMediaProfiles.GetValueAt(0);
            if (pProfile)
            {
                nParamId = pProfile->nNegoId;
            }
        }
    }

    if (bOriginalProfile == IMS_FALSE)  // virtual early session
    {
        nParamId = 0;
    }

    MediaProfile* pProfile = new MediaProfile();
    pProfile->nNegoId = piMediaSession->CreateProfile(nParamId, eMediaContents);
    pProfile->bForked = bForked;
    pProfile->bOriginalProfile = bOriginalProfile;

    m_objMediaProfiles.Add(piSession, pProfile);
}

PUBLIC
void MtcMediaProfileManager::DestroyMediaProfile(
        IN ISession* piSession, IN IMediaSession* piMediaSession)
{
    IMS_TRACE_D("DestroyMediaProfile", 0, 0, 0);

    // It can be called when the specific media profile should be destroyed.
    if (!IsMediaProfilePresent(piSession))
    {
        return;
    }

    piMediaSession->DestroyProfile(GetNegoId(piSession));

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
    IMS_TRACE_D("DestroyAllMediaProfiles", 0, 0, 0);

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        ISession* piSession = m_objMediaProfiles.GetKeyAt(index);
        DestroyMediaProfile(piSession, piMediaSession);
    }
}

PUBLIC
IMS_UINTP MtcMediaProfileManager::GetNegoId(IN ISession* piSession)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_NULL;
    }

    return pProfile->nNegoId;
}

PUBLIC
PemType MtcMediaProfileManager::GetPemType(IN ISession* piSession)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return PemType::NONE;
    }

    return pProfile->ePemType;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsActive(IN ISession* piSession)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bActive;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsConfirmed(IN ISession* piSession)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bConfirmed;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsForked(IN ISession* piSession)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bForked;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsOriginalProfile(IN ISession* piSession)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return IMS_FALSE;
    }

    return pProfile->bOriginalProfile;
}

PUBLIC
void MtcMediaProfileManager::SetPemType(IN ISession* piSession, IN PemType ePemType)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->ePemType = ePemType;
}

PUBLIC
void MtcMediaProfileManager::SetActive(IN ISession* piSession, IN IMS_BOOL bActive)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->bActive = bActive;
}

PUBLIC
void MtcMediaProfileManager::SetConfirmed(IN ISession* piSession, IN IMS_BOOL bConfirmed)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->bConfirmed = bConfirmed;
}

PUBLIC
void MtcMediaProfileManager::SetForked(IN ISession* piSession, IN IMS_BOOL bForked)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->bConfirmed = bForked;
}

PUBLIC
void MtcMediaProfileManager::SetAsOriginalProfile(
        IN ISession* piSession, IN IMS_BOOL bOriginalProfile)
{
    MediaProfile* pProfile = GetMediaProfile(piSession);

    if (!pProfile)
    {
        return;
    }

    pProfile->bOriginalProfile = bOriginalProfile;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsPemSendInOtherEarlySession(IN ISession* piSession)
{
    IMS_BOOL bResult = IMS_FALSE;

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        ISession* piTargetSession = m_objMediaProfiles.GetKeyAt(index);

        if (piTargetSession == piSession)
        {
            continue;
        }

        if (IsConfirmed(piTargetSession))
        {
            continue;
        }

        PemType ePemType = GetPemType(piTargetSession);

        if (IsActive(piTargetSession) &&
                ((ePemType == PemType::SENDRECV) || (ePemType == PemType::SENDONLY)))
        {
            bResult = IMS_TRUE;
            break;
        }
    }

    IMS_TRACE_D("IsPemSendInOtherEarlySession : %d", bResult, 0, 0);
    return bResult;
}

PUBLIC
IMS_BOOL MtcMediaProfileManager::IsConfirmedDialogState()
{
    IMS_BOOL bResult = IMS_FALSE;

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        MediaProfile* pProfile = m_objMediaProfiles.GetValueAt(index);
        if (!pProfile)
        {
            continue;
        }

        if (pProfile->bConfirmed)
        {
            bResult = IMS_TRUE;
            break;
        }
    }

    IMS_TRACE_D("IsConfirmedDialogState : %d", bResult, 0, 0);

    return bResult;
}

PUBLIC
void MtcMediaProfileManager::UpdateProfileForMediaActivation(IN ISession* piActiveSession)
{
    IMS_TRACE_D("UpdateProfileForMediaActivation", 0, 0, 0);
    SetActive(piActiveSession, IMS_TRUE);

    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        ISession* piSession = m_objMediaProfiles.GetKeyAt(index);
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
        ISession* piSession = m_objMediaProfiles.GetKeyAt(index);

        if (nNegoId == GetNegoId(piSession))
        {
            return piSession;
        }
    }

    IMS_TRACE_D("GetSessionWithNegoId : ISession is null.", 0, 0, 0);
    return IMS_NULL;
}

PUBLIC
ISession* MtcMediaProfileManager::GetActiveSession()
{
    for (IMS_UINT32 index = 0; index < m_objMediaProfiles.GetSize(); index++)
    {
        ISession* piSession = m_objMediaProfiles.GetKeyAt(index);

        if (IsActive(piSession))
        {
            return piSession;
        }
    }

    IMS_TRACE_D("GetActiveSession : There is no active session.", 0, 0, 0);
    return IMS_NULL;
}

PRIVATE
IMS_BOOL MtcMediaProfileManager::IsMediaProfilePresent(IN ISession* piSession)
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
MediaProfile* MtcMediaProfileManager::GetMediaProfile(IN ISession* piSession)
{
    return m_objMediaProfiles.GetValue(piSession);
}
