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
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "device/OsLocationInfo.h"
#include "system-intf/System.h"

__IMS_TRACE_TAG_ADAPT__;

PRIVATE GLOBAL const IMS_CHAR OsLocationInfo::COUNTRY_ISO_UNKNOWN[] = "ZZ";

class LocationProperties : public ILocationProperties
{
public:
    LocationProperties();
    virtual ~LocationProperties();

    inline virtual const AString& GetLatitude() const { return m_strLatitude; }
    inline virtual const AString& GetLongitude() const { return m_strLongitude; }
    inline virtual const AString& GetRadius() const { return m_strRadius; }
    inline virtual const AString& GetShape() const { return m_strShape; }
    inline virtual const AString& GetConfidence() const { return m_strConfidence; }
    inline virtual const AString& GetCurrentTime() const { return m_strCurrentTime; }
    inline virtual const AString& GetMethod() const { return m_strMethod; }
    inline virtual const AString& GetCountry() const { return m_strCountry; }
    inline virtual const AString& GetState() const { return m_strState; }
    inline virtual const AString& GetCity() const { return m_strCity; }
    inline virtual const AString& GetPostal() const { return m_strPostal; }
    inline virtual const AString& GetAltitude() const { return m_strAltitude; }
    inline virtual const AString& GetVerticalAccuracy() const { return m_strVerticalAccuracy; }

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

PUBLIC VIRTUAL IMS_BOOL OsLocationInfo::StartLocationInfo(IN IMS_UINT32 nUpdateIntervalInSec)
{
    if (m_bIsStarted)
    {
        return IMS_TRUE;
    }

    if (System::GetInstance()->StartLocationInfo(nUpdateIntervalInSec, GetSlotId()))
    {
        m_bIsStarted = IMS_TRUE;
    }
    else
    {
        m_bIsStarted = IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsLocationInfo::StopLocationInfo()
{
    if (!m_bIsStarted)
    {
        return;
    }

    System::GetInstance()->StopLocationInfo(GetSlotId());
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

    if (System::GetInstance()->GetLocationInformation(objLocationInfo, nType, GetSlotId()) == 1)
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

PUBLIC VIRTUAL IMS_BOOL OsLocationInfo::MakeInstantLocationInfo()
{
    return System::GetInstance()->MakeInstantLocationInfo(GetSlotId());
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
AString OsLocationInfo::GetCountryIso(IN IMS_BOOL bFromUicc /*= IMS_FALSE*/)
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
