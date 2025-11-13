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

#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MtcDef.h"

class IMediaSession;
class ISession;

class MediaProfile
{
public:
    inline MediaProfile() :
            nNegoId(UNDEFINED_NEGO_ID),
            ePemType(PemType::NONE),
            bActive(IMS_FALSE),
            bConfirmed(IMS_FALSE),
            bForked(IMS_FALSE),
            bOriginalProfile(IMS_TRUE)
    {
    }
    inline virtual ~MediaProfile() {}
    MediaProfile(IN const MediaProfile&) = delete;
    MediaProfile& operator=(IN const MediaProfile&) = delete;

private:
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
    MtcMediaProfileManager(IN const MtcMediaProfileManager&) = delete;
    MtcMediaProfileManager& operator=(IN const MtcMediaProfileManager&) = delete;

public:
    virtual void CreateMediaProfile(IN const ISession* piSession, IN IMS_BOOL bForked,
            IN IMS_BOOL bOriginalProfile, IN MEDIA_CONTENT_TYPE eMediaContents,
            IN IMediaSession* piMediaSession);
    virtual void DestroyMediaProfile(
            IN const ISession* piSession, IN IMediaSession* piMediaSession);
    virtual void DestroyAllMediaProfiles(IN IMediaSession* piMediaSession);

    virtual IMS_UINTP GetNegoId(IN const ISession* piSession) const;
    virtual PemType GetPemType(IN const ISession* piSession) const;
    virtual IMS_BOOL IsActive(IN const ISession* piSession) const;
    virtual IMS_BOOL IsConfirmed(IN const ISession* piSession) const;
    virtual IMS_BOOL IsForked(IN const ISession* piSession) const;

    virtual void SetPemType(IN const ISession* piSession, IN PemType ePemType);
    virtual void SetActive(IN const ISession* piSession, IN IMS_BOOL bActive);
    virtual void SetConfirmed(IN const ISession* piSession, IN IMS_BOOL bConfirmed);

    virtual IMS_BOOL IsPemSendInOtherEarlySession(IN const ISession* piSession) const;
    virtual void UpdateProfileForMediaActivation(IN const ISession* piActiveSession);

    virtual ISession* GetSessionWithNegoId(IN IMS_UINTP nNegoId);
    virtual ISession* GetActiveSession() const;

private:
    IMS_BOOL IsMediaProfilePresent(IN const ISession* piSession) const;
    MediaProfile* GetMediaProfile(IN const ISession* piSession) const;

private:
    ImsMap<const ISession*, MediaProfile*> m_objMediaProfiles;
};

#endif
