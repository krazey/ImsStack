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
#include "PlatformContext.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "device/OsLocationInfo.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC GLOBAL const IMS_CHAR OsLocationInfo::COUNTRY_ISO_UNKNOWN[] = "ZZ";

class LocationProperties : public ILocationProperties
{
public:
    LocationProperties();
    virtual ~LocationProperties();

    inline const AString& GetLatitude() const override { return m_strLatitude; }
    inline const AString& GetLongitude() const override { return m_strLongitude; }
    inline const AString& GetRadius() const override { return m_strRadius; }
    inline const AString& GetShape() const override { return m_strShape; }
    inline const AString& GetConfidence() const override { return m_strConfidence; }
    inline const AString& GetCurrentTime() const override { return m_strCurrentTime; }
    inline const AString& GetMethod() const override { return m_strMethod; }
    inline const AString& GetCountry() const override { return m_strCountry; }
    inline const AString& GetState() const override { return m_strState; }
    inline const AString& GetCity() const override { return m_strCity; }
    inline const AString& GetPostal() const override { return m_strPostal; }
    inline const AString& GetAltitude() const override { return m_strAltitude; }
    inline const AString& GetVerticalAccuracy() const override { return m_strVerticalAccuracy; }

    inline void SetLatitude(IN const AString& strLatitude) { m_strLatitude = strLatitude; }
    inline void SetLongitude(IN const AString& strLongitude) { m_strLongitude = strLongitude; }
    inline void SetRadius(IN const AString& strRadius) { m_strRadius = strRadius; }
    inline void SetShape(IN const AString& strShape) { m_strShape = strShape; }
    inline void SetConfidence(IN const AString& strConfidence) { m_strConfidence = strConfidence; }
    inline void SetCurrentTime(IN const AString& strCurrentTime)
    {
        m_strCurrentTime = strCurrentTime;
    }
    inline void SetMethod(IN const AString& strMethod) { m_strMethod = strMethod; }
    inline void SetCountry(IN const AString& strCountry) { m_strCountry = strCountry; }
    inline void SetState(IN const AString& strState) { m_strState = strState; }
    inline void SetCity(IN const AString& strCity) { m_strCity = strCity; }
    inline void SetPostal(IN const AString& strPostal) { m_strPostal = strPostal; }
    inline void SetAltitude(IN const AString& strAltitude) { m_strAltitude = strAltitude; }
    inline void SetVerticalAccuracy(IN const AString& strVerticalAccuracy)
    {
        m_strVerticalAccuracy = strVerticalAccuracy;
    }

private:
    AString m_strLatitude;
    AString m_strLongitude;
    AString m_strRadius;
    AString m_strShape;
    AString m_strConfidence;
    AString m_strCurrentTime;
    AString m_strMethod;
    AString m_strCountry;
    AString m_strState;
    AString m_strCity;
    AString m_strPostal;
    AString m_strAltitude;
    AString m_strVerticalAccuracy;
};

PUBLIC
LocationProperties::LocationProperties() :
        m_strLatitude(AString::ConstEmpty()),
        m_strLongitude(AString::ConstEmpty()),
        m_strRadius(AString::ConstEmpty()),
        m_strShape(AString::ConstEmpty()),
        m_strConfidence(AString::ConstEmpty()),
        m_strCurrentTime(AString::ConstEmpty()),
        m_strMethod(AString::ConstEmpty()),
        m_strCountry(AString::ConstEmpty()),
        m_strState(AString::ConstEmpty()),
        m_strCity(AString::ConstEmpty()),
        m_strPostal(AString::ConstEmpty()),
        m_strAltitude(AString::ConstEmpty()),
        m_strVerticalAccuracy(AString::ConstEmpty())
{
}

PUBLIC VIRTUAL LocationProperties::~LocationProperties() {}

PUBLIC
OsLocationInfo::OsLocationInfo(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_bIsStarted(IMS_FALSE),
        m_strLastKnownCountry(COUNTRY_ISO_UNKNOWN),
        m_pLocationProperties(IMS_NULL)
{
}

PUBLIC VIRTUAL OsLocationInfo::~OsLocationInfo() {}

PUBLIC VIRTUAL IMS_BOOL OsLocationInfo::StartListeningForLocation(
        IN IMS_UINT32 nUpdateIntervalInSec)
{
    if (m_bIsStarted)
    {
        return IMS_TRUE;
    }

    if (PlatformContext::GetInstance()->GetSystem()->StartListeningForLocation(
                nUpdateIntervalInSec, GetSlotId()))
    {
        m_bIsStarted = IMS_TRUE;
    }
    else
    {
        m_bIsStarted = IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsLocationInfo::StopListeningForLocation()
{
    if (!m_bIsStarted)
    {
        return;
    }

    PlatformContext::GetInstance()->GetSystem()->StopListeningForLocation(GetSlotId());
    m_bIsStarted = IMS_FALSE;
}

PUBLIC VIRTUAL ILocationProperties* OsLocationInfo::GetLocationProperties(IN IMS_SINT32 nType)
{
    AStringArray objLocationInfo;

    if (m_pLocationProperties != IMS_NULL)
    {
        delete m_pLocationProperties;
        m_pLocationProperties = IMS_NULL;
    }

    if (PlatformContext::GetInstance()->GetSystem()->GetLastKnownLocation(
                objLocationInfo, nType, GetSlotId()) == 1)
    {
        m_pLocationProperties = new LocationProperties();

        m_pLocationProperties->SetLatitude(objLocationInfo.GetElementAt(0));
        m_pLocationProperties->SetLongitude(objLocationInfo.GetElementAt(1));
        m_pLocationProperties->SetRadius(objLocationInfo.GetElementAt(2));
        m_pLocationProperties->SetShape(objLocationInfo.GetElementAt(3));
        m_pLocationProperties->SetConfidence(objLocationInfo.GetElementAt(4));
        m_pLocationProperties->SetCurrentTime(objLocationInfo.GetElementAt(5));
        m_pLocationProperties->SetMethod(objLocationInfo.GetElementAt(6));
        m_pLocationProperties->SetCountry(objLocationInfo.GetElementAt(7));
        m_pLocationProperties->SetState(objLocationInfo.GetElementAt(8));
        m_pLocationProperties->SetCity(objLocationInfo.GetElementAt(9));
        m_pLocationProperties->SetPostal(objLocationInfo.GetElementAt(10));
        m_pLocationProperties->SetAltitude(objLocationInfo.GetElementAt(11));
        m_pLocationProperties->SetVerticalAccuracy(objLocationInfo.GetElementAt(12));

        const AString& strCountry = m_pLocationProperties->GetCountry();

        if (strCountry.GetLength() != 0)
        {
            SetLastKnownCountry(strCountry);
        }
        else
        {
            AString strCountryIso = GetCountryIso();

            if (strCountryIso.GetLength() != 0)
            {
                SetLastKnownCountry(strCountryIso);
            }
            else
            {
                SetLastKnownCountry(COUNTRY_ISO_UNKNOWN);
            }

            m_pLocationProperties->SetCountry(GetLastKnownCountry());
        }
    }

    return m_pLocationProperties;
}

PUBLIC VIRTUAL IMS_BOOL OsLocationInfo::StartInstantLocationUpdate()
{
    return PlatformContext::GetInstance()->GetSystem()->StartInstantLocationUpdate(GetSlotId());
}

PUBLIC VIRTUAL void OsLocationInfo::SetDefaultLocationProperties(
        IN IMS_BOOL bFromUicc /*= IMS_TRUE*/)
{
    IMS_TRACE_D("SetDefaultLocationProperties", 0, 0, 0);

    if (m_pLocationProperties != IMS_NULL)
    {
        delete m_pLocationProperties;
    }

    m_pLocationProperties = new LocationProperties();

    AString strCountry = GetCountryIso(bFromUicc);

    if (strCountry.GetLength() == 0)
    {
        // ZZ to represent "Unknown or Invalid Territory, ISO 3166-1 alpha-2
        m_pLocationProperties->SetCountry(COUNTRY_ISO_UNKNOWN);
        SetLastKnownCountry(COUNTRY_ISO_UNKNOWN);
    }
    else
    {
        m_pLocationProperties->SetCountry(strCountry);
        SetLastKnownCountry(strCountry);
    }
}

PUBLIC VIRTUAL const AString& OsLocationInfo::GetLastKnownCountry() const
{
    return m_strLastKnownCountry;
}

PRIVATE
AString OsLocationInfo::GetCountryIso(IN IMS_BOOL bFromUicc /*= IMS_FALSE*/) const
{
    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(GetSlotId());
    AString strCountry;

    piSubsInfo->GetNetworkCountry(strCountry);

    if (bFromUicc && (strCountry.GetLength() == 0))
    {
        // If country ISO from the attached network is null, get it from UICC.
        piSubsInfo->GetCountry(strCountry);
    }

    return strCountry;
}

PRIVATE
void OsLocationInfo::SetLastKnownCountry(IN const AString& strCountry)
{
    if (!m_strLastKnownCountry.Equals(strCountry))
    {
        IMS_TRACE_D("SetLastKnownCountry :: %s >> %s", m_strLastKnownCountry.GetStr(),
                strCountry.GetStr(), 0);

        m_strLastKnownCountry = strCountry;
    }
}
