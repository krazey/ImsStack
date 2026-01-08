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

#include "ImsServiceConfig.h"

#include "provider/AosStaticProfile.h"

#include "provider/AosStaticConfig.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosStaticConfig::AosStaticConfig() :
        m_objProfiles(ImsList<AosStaticProfile*>())
{
}

PUBLIC VIRTUAL AosStaticConfig::~AosStaticConfig() {}

PUBLIC
IMS_BOOL AosStaticConfig::Create()
{
    IMS_TRACE_D("Create", 0, 0, 0);
    // for normal
    AosStaticProfile* pProfile = new AosStaticProfile();
    pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    ImsList<ImsServiceName> objServiceName =
            ImsServiceConfig::GetServiceNames(ImsServiceConfig::GetServiceProfile());

    for (IMS_UINT32 i = 0; i < objServiceName.GetSize(); i++)
    {
        ImsServiceName objService = objServiceName.GetAt(i);
        pProfile->AddService(objService.GetAppId(), objService.GetServiceId());
    }
    m_objProfiles.Append(pProfile);

    // for emergency
    AosStaticProfile* pEProfile = new AosStaticProfile();
    pEProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    ImsList<ImsServiceName> objEServiceName =
            ImsServiceConfig::GetServiceNames(ImsServiceConfig::GetEmergencyServiceProfile());

    for (IMS_UINT32 i = 0; i < objEServiceName.GetSize(); i++)
    {
        ImsServiceName objService = objEServiceName.GetAt(i);
        pEProfile->AddService(objService.GetAppId(), objService.GetServiceId());
    }
    m_objProfiles.Append(pEProfile);

    return IMS_TRUE;
}

PUBLIC
void AosStaticConfig::Destroy()
{
    IMS_TRACE_D("Destroy", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objProfiles.GetSize(); i++)
    {
        AosStaticProfile* pProfile = m_objProfiles.GetAt(i);
        if (pProfile != IMS_NULL)
        {
            delete pProfile;
        }
    }

    m_objProfiles.Clear();
}

PUBLIC
AosStaticProfile* AosStaticConfig::GetProfile(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    for (IMS_UINT32 i = 0; i < m_objProfiles.GetSize(); i++)
    {
        AosStaticProfile* pProfile = m_objProfiles.GetAt(i);

        if (pProfile == IMS_NULL)
            continue;

        const ImsList<AosServiceProfile*>& objServices = pProfile->GetServiceProfiles();
        for (IMS_UINT32 j = 0; j < objServices.GetSize(); j++)
        {
            AosServiceProfile* pService = objServices.GetAt(j);
            if (pService == IMS_NULL)
                continue;

            if (strAppId.Equals(pService->GetAppId()) &&
                    strServiceId.Equals(pService->GetServiceId()))
            {
                return pProfile;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
const ImsList<AosStaticProfile*>& AosStaticConfig::GetProfiles() const
{
    return m_objProfiles;
}
