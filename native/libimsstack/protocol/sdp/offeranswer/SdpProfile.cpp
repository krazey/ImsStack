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
#include "IMSMap.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"

#include "offeranswer/SdpProfile.h"

class SdpProfilePrivate
{
public:
    SdpProfilePrivate();
    ~SdpProfilePrivate();

    SdpProfilePrivate(IN const SdpProfilePrivate&) = delete;
    SdpProfilePrivate& operator=(IN const SdpProfilePrivate&) = delete;

public:
    void InitFeatures(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nNewFeatures);
    IMS_SINT32 GetFeatures(IN IMS_SINT32 nSlotId);

public:
    // Pair: slot-id / features
    IMSMap<IMS_SINT32, IMS_SINT32> m_objFeatures;
};

PUBLIC
SdpProfilePrivate::SdpProfilePrivate()
{
    for (IMS_UINT32 i = 0; i < SystemConfig::GetMaxSimSlot(); ++i)
    {
        m_objFeatures.Add(i, SdpProfile::FEATURE_A_PRECONDITION_SUPPORTED);
    }
}

PUBLIC
SdpProfilePrivate::~SdpProfilePrivate()
{
    m_objFeatures.Clear();
}

PUBLIC
void SdpProfilePrivate::InitFeatures(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nNewFeatures)
{
    IMS_SLONG nIndex = m_objFeatures.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return;
    }

    IMS_SINT32& nFeatures = m_objFeatures.GetValueAt(nIndex);

    nFeatures = nNewFeatures;
}

PUBLIC
IMS_SINT32 SdpProfilePrivate::GetFeatures(IN IMS_SINT32 nSlotId)
{
    IMS_SLONG nIndex = m_objFeatures.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return SdpProfile::FEATURE_NONE;
    }

    return m_objFeatures.GetValueAt(nIndex);
}

PRIVATE
SdpProfile::SdpProfile() :
        m_pPrivate(new SdpProfilePrivate())
{
}

PRIVATE
SdpProfile::~SdpProfile()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
void SdpProfile::InitFeatures(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nNewFeatures)
{
    m_pPrivate->InitFeatures(nSlotId, nNewFeatures);
}

PUBLIC
IMS_SINT32 SdpProfile::GetFeatures(IN IMS_SINT32 nSlotId) const
{
    return m_pPrivate->GetFeatures(nSlotId);
}

PUBLIC
IMS_BOOL SdpProfile::IsAttributeDirectionRequiredForRemovedMedia() const
{
    return IsAttributeDirectionRequiredForRemovedMedia(ThreadService::GetCurrentSlotId());
}

PUBLIC
IMS_BOOL SdpProfile::IsAttributeDirectionRequiredForRemovedMedia(IN IMS_SINT32 nSlotId) const
{
    return (GetFeatures(nSlotId) & FEATURE_A_DIRECTION_REQUIRED_FOR_REMOVED_MEDIA) != 0;
}

PUBLIC
IMS_BOOL SdpProfile::IsAttributePreconditionSupported(IN IMS_SINT32 nSlotId) const
{
    return (GetFeatures(nSlotId) & FEATURE_A_PRECONDITION_SUPPORTED) != 0;
}

PUBLIC GLOBAL SdpProfile* SdpProfile::GetInstance()
{
    static SdpProfile* s_pProfile = IMS_NULL;

    if (s_pProfile == IMS_NULL)
    {
        s_pProfile = new SdpProfile();
    }

    return s_pProfile;
}
