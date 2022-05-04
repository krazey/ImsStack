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
#ifndef AOS_STATIC_PROFILE_H_
#define AOS_STATIC_PROFILE_H_

#include "IMSList.h"
#include "AString.h"

/// Define a flow Id for registration
enum class AosRegistrationFlowId
{
    NORMAL = 1,
    EMERGENCY,
    RCS
};

/// Define a type for registration
enum class AosRegistrationType
{
    NORMAL = 0,
    EMERGENCY,
    // For fake registration (w/o network interworking)
    FAKE,
    // For only RCS registration
    RCS
};

class AosServiceProfile
{
public:
    AosServiceProfile(IN AString strAppId, IN AString strServiceId) :
            m_strAppId(strAppId),
            m_ServiceId(strServiceId)
    {
    }

    virtual ~AosServiceProfile() {}

    inline AString& GetAppId() { return m_strAppId; }
    inline AString& GetServiceId() { return m_ServiceId; }

public:
    AString m_strAppId;
    AString m_ServiceId;
};

class AosStaticProfile
{
public:
    AosStaticProfile();
    virtual ~AosStaticProfile();

    /// Define a type for aos profile
    enum class Type
    {
        NORMAL = 0,
        EMERGENCY,
        RCS
    };

    void SetProflieType(IN Type eType);
    void AddService(IN AString strAppId, IN AString strServiceId);

    AString& GetId();
    IMS_SINT32 GetConnectionType();
    IMS_UINT32 GetRegistrationFlowId();
    AString& GetRegistrationId();

    Type GetProfileType() const;
    AosRegistrationType GetRegistrationType() const;

    const IMSList<AosServiceProfile*>& GetServiceProfiles() const;

private:
    IMS_BOOL IsWifiTest();

private:
    Type m_eProfileType;
    AosRegistrationType m_eRegistrationType;
    AString m_strId;
    AString m_strRegistrationId;

    IMS_SINT32 m_nConnectionType;
    IMS_UINT32 m_nRegistrationFlowId;
    IMSList<AosServiceProfile*> m_objServiceProfiles;
};

#endif  // AOS_STATIC_PROFILE_H_
