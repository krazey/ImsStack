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

#ifndef MTC_MEDIA_PROFILE_MANAGER_H_
#define MTC_MEDIA_PROFILE_MANAGER_H_

#include "IMediaSession.h"
#include "ISession.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"

class MediaProfile
{
public:
    inline MediaProfile() :
            nNegoId(IMS_NULL),
            ePemType(PemType::NONE),
            bActive(IMS_FALSE),
            bConfirmed(IMS_FALSE),
            bForked(IMS_FALSE),
            bOriginalProfile(IMS_TRUE)
    {
    }

    inline MediaProfile(IN const MediaProfile& objRHS)
    {
        nNegoId = objRHS.nNegoId;
        ePemType = objRHS.ePemType;
        bActive = objRHS.bActive;
        bConfirmed = objRHS.bConfirmed;
        bForked = objRHS.bForked;
        bOriginalProfile = objRHS.bOriginalProfile;
    }

    inline virtual ~MediaProfile()
    {
        if (nNegoId != IMS_NULL)
        {
            nNegoId = IMS_NULL;
        }
    }

private:
    MediaProfile& operator=(IN const MediaProfile& objRHS);
    friend class MtcMediaProfileManager;

private:
    IMS_UINTP nNegoId;
    PemType ePemType;
    IMS_BOOL bActive;
    IMS_BOOL bConfirmed;
    IMS_BOOL bForked;
    IMS_BOOL bOriginalProfile;
};

class MtcMediaProfileManager
{
public:
    MtcMediaProfileManager();
    virtual ~MtcMediaProfileManager();

public:
    void CreateMediaProfile(IN ISession* piSession, IN IMS_BOOL bForked,
            IN IMS_BOOL bOriginalProfile, IN MEDIA_CONTENT_TYPE eMediaContents,
            IN IMediaSession* piMediaSession);
    void DestroyMediaProfile(IN ISession* piSession, IN IMediaSession* piMediaSession);
    void DestroyAllMediaProfiles(IN IMediaSession* piMediaSession);

    IMS_UINTP GetNegoId(IN ISession* piSession);
    PemType GetPemType(IN ISession* piSession);
    IMS_BOOL IsActive(IN ISession* piSession);
    IMS_BOOL IsConfirmed(IN ISession* piSession);
    IMS_BOOL IsForked(IN ISession* piSession);
    IMS_BOOL IsOriginalProfile(IN ISession* piSession);

    void SetPemType(IN ISession* piSession, IN PemType ePemType);
    void SetActive(IN ISession* piSession, IN IMS_BOOL bActive);
    void SetConfirmed(IN ISession* piSession, IN IMS_BOOL bConfirmed);
    void SetForked(IN ISession* piSession, IN IMS_BOOL bForked);
    void SetAsOriginalProfile(IN ISession* piSession, IN IMS_BOOL bOriginalProfile);

    IMS_BOOL IsPemSendInOtherEarlySession(IN ISession* piSession);  // EarlySession? or ISession?
    IMS_BOOL IsConfirmedDialogState();
    void UpdateProfileForMediaActivation(IN ISession* piActiveSession);

    ISession* GetSessionWithNegoId(IN IMS_UINTP nNegoId);
    ISession* GetActiveSession();

private:
    IMS_BOOL IsMediaProfilePresent(IN ISession* piSession);
    MediaProfile* GetMediaProfile(IN ISession* piSession);
    // IMS_BOOL IsCreateMediaProfile(); // does it need..?
    // IMS_BOOL AddMediaSetWithOriginNegoId(); // does it need...?

private:
    // MtcMediaManager* m_pMediaManager;
    // IMS_UINTP pFirstMediaNegoId;
    IMSMap<ISession*, MediaProfile*> m_objMediaProfiles;
};

#endif
