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
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "SystemConfig.h"

#include "Engine.h"
#include "IConfiguration.h"

#include "GeolocationHelper.h"
#include "GeolocationPidfCreator.h"

class GeolocationHelperPrivate
{
public:
    GeolocationHelperPrivate();
    ~GeolocationHelperPrivate();

    GeolocationHelperPrivate(IN const GeolocationHelperPrivate&) = delete;
    GeolocationHelperPrivate& operator=(IN const GeolocationHelperPrivate&) = delete;

public:
    inline void CreatePidfCreator(IN IMS_SINT32 nSlotId)
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            return;
        }

        if (m_ppCreator[nSlotId] == IMS_NULL)
        {
            m_ppCreator[nSlotId] = new GeolocationPidfCreator(nSlotId);
        }
    }

    inline void DestroyPidfCreator(IN IMS_SINT32 nSlotId)
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            return;
        }

        if (m_ppCreator[nSlotId] != IMS_NULL)
        {
            delete m_ppCreator[nSlotId];
            m_ppCreator[nSlotId] = IMS_NULL;
        }
    }

    inline GeolocationPidfCreator* GetPidfCreator(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppCreator[nSlotId];
    }

private:
    GeolocationPidfCreator** m_ppCreator;
};

PUBLIC
GeolocationHelperPrivate::GeolocationHelperPrivate() :
        m_ppCreator(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppCreator = new GeolocationPidfCreator*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppCreator[i] = IMS_NULL;
    }
}

PUBLIC
GeolocationHelperPrivate::~GeolocationHelperPrivate()
{
    if (m_ppCreator != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppCreator[i] != IMS_NULL)
            {
                delete m_ppCreator[i];
            }
        }

        delete[] m_ppCreator;
    }
}

PRIVATE GLOBAL GeolocationHelper* GeolocationHelper::s_pGeolocationHelper = IMS_NULL;

PRIVATE
GeolocationHelper::GeolocationHelper() :
        m_pPrivate(new GeolocationHelperPrivate())
{
}

PRIVATE
GeolocationHelper::~GeolocationHelper()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
void GeolocationHelper::CreatePidfCreator(IN IMS_SINT32 nSlotId)
{
    m_pPrivate->CreatePidfCreator(nSlotId);
}

PUBLIC
void GeolocationHelper::DestroyPidfCreator(IN IMS_SINT32 nSlotId)
{
    m_pPrivate->DestroyPidfCreator(nSlotId);
}

PUBLIC
GeolocationPidfCreator* GeolocationHelper::GetPidfCreator(IN IMS_SINT32 nSlotId)
{
    return m_pPrivate->GetPidfCreator(nSlotId);
}

PUBLIC GLOBAL GeolocationHelper* GeolocationHelper::GetInstance()
{
    if (s_pGeolocationHelper == IMS_NULL)
    {
        s_pGeolocationHelper = new GeolocationHelper();
    }

    return s_pGeolocationHelper;
}

// Creates an identifier for Content-ID header field
PUBLIC GLOBAL AString GeolocationHelper::CreateContentId(
        IN IMS_SINT32 nSlotId, IN const AString& strDomain /*= AString::ConstNull()*/)
{
    IMS_UINT32 nRandom = IMS_SYS_GetSRandom0();
    IMS_UINT32 nMicroSeconds = IMS_SYS_GetTimeInMicroSeconds();

    AString strContentId;

    if (strDomain.GetLength() > 0)
    {
        strContentId.Sprintf("%05x%05x@%s", nMicroSeconds, nRandom, strDomain.GetStr());
    }
    else
    {
        const ISubscriberConfig* piSubsConfig =
                Engine::GetConfiguration()->GetSubscriberConfig(nSlotId);

        strContentId.Sprintf("%05x%05x@%s", nMicroSeconds, nRandom,
                (piSubsConfig == IMS_NULL) ? "unavailable.com"
                                           : piSubsConfig->GetHomeDomainName().GetStr());
    }

    return strContentId;
}

// Returns the recent country or newly updated country
PUBLIC GLOBAL const AString& GeolocationHelper::GetCountry(
        IN IMS_SINT32 nSlotId, IN IMS_BOOL bLocationUpdate)
{
    ILocationInfo* piLocationInfo =
            PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(nSlotId);

    if (piLocationInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    if (bLocationUpdate)
    {
        const ILocationProperties* piLocation =
                piLocationInfo->GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY);

        if (piLocation != IMS_NULL)
        {
            return piLocation->GetCountry();
        }
        else
        {
            return AString::ConstNull();
        }
    }
    else
    {
        return piLocationInfo->GetLastKnownCountry();
    }
}
