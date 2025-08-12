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
#include <binder/Parcel.h>

#include "PlatformContext.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "device/OsLocationInfo.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

PUBLIC GLOBAL const IMS_CHAR OsLocationInfo::COUNTRY_ISO_UNKNOWN[] = "ZZ";

class LocationProperties : public ILocationProperties
{
public:
    LocationProperties();
    ~LocationProperties() override = default;

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

class LocationUpdateRequest final : public AsyncExecutor
{
public:
    inline LocationUpdateRequest(IN IMS_SINT32 nId, IN ILocationUpdateListener* piListener) :
            AsyncExecutor(IMS_TRUE),
            m_nId(nId),
            m_piListener(piListener)
    {
    }
    ~LocationUpdateRequest() override = default;

public:
    inline IMS_SINT32 GetId() const { return m_nId; }
    inline ILocationUpdateListener* GetListener() const { return m_piListener; }
    inline void NotifyLocationUpdateCompleted() { m_piListener->LocationUpdate_OnCompleted(); }

protected:
    inline void OnExecute(IN IMS_UINTP /*nParam1*/, IN IMS_UINTP /*nParam2*/) override
    {
        NotifyLocationUpdateCompleted();
    }

private:
    IMS_SINT32 m_nId;
    ILocationUpdateListener* m_piListener;
};

class LocationUpdateCompletedHandler final : public AsyncExecutor
{
public:
    inline explicit LocationUpdateCompletedHandler(IN AsyncExecutor::IListener* piListener) :
            AsyncExecutor(piListener, IMS_FALSE),
            m_nId(0)
    {
    }
    ~LocationUpdateCompletedHandler() override = default;

public:
    inline void OnExecute(IN IMS_UINTP /*nParam1*/, IN IMS_UINTP nParam2) override
    {
        m_nId = static_cast<IMS_SINT32>(nParam2);
    }
    inline IMS_SINT32 GetId() const { return m_nId; }

private:
    IMS_SINT32 m_nId;
};

PUBLIC
OsLocationInfo::OsLocationInfo(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_bIsStarted(IMS_FALSE),
        m_strLastKnownCountry(COUNTRY_ISO_UNKNOWN),
        m_pLocationProperties(IMS_NULL)
{
    m_pLocationUpdateCompletedHandler = new LocationUpdateCompletedHandler(this);

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_LOCATION, this, GetSlotId());
}

PUBLIC VIRTUAL OsLocationInfo::~OsLocationInfo()
{
    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_LOCATION, this, GetSlotId());

    for (IMS_UINT32 i = 0; i < m_objLocationUpdateRequests.GetSize(); ++i)
    {
        delete m_objLocationUpdateRequests.GetAt(i);
    }
    m_objLocationUpdateRequests.Clear();

    delete m_pLocationUpdateCompletedHandler;
}

PUBLIC VIRTUAL void OsLocationInfo::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP /*nWParam*/, IN IMS_UINTP nLParam)
{
    IMS_TRACE_D("OsLocationInfo: System_NotifyEvent - slotId=%d, event=%d", GetSlotId(), nEvent, 0);

    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(nLParam);

    if (pParcel == IMS_NULL)
    {
        return;
    }

    if (nEvent == EVENT_LOCATION_UPDATE_COMPLETED)
    {
        IMS_SINT32 nId = pParcel->readInt32();
        m_pLocationUpdateCompletedHandler->Execute(0, nId);
    }
}

PUBLIC VIRTUAL void OsLocationInfo::AsyncExecutor_OnExecuteCompleted(IN AsyncExecutor* pExecutor)
{
    if (m_pLocationUpdateCompletedHandler == pExecutor)
    {
        NotifyLocationUpdateCompleted(m_pLocationUpdateCompletedHandler->GetId());
    }
}

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

PUBLIC VIRTUAL void OsLocationInfo::RequestLocationUpdate(
        IN IMS_SINT32 nWaitTimeMs, IN ILocationUpdateListener* piListener)
{
    IMS_SINT32 nId = PlatformContext::GetInstance()->GetSystem()->RequestLocationUpdate(
            nWaitTimeMs, GetSlotId());
    LocationUpdateRequest* pRequest = new LocationUpdateRequest(nId, piListener);

    if (nId > 0)
    {
        m_objLocationUpdateRequests.Append(pRequest);
    }
    else
    {
        IMS_TRACE_I("LUR-failed", 0, 0, 0);
        pRequest->Execute();
    }
}

PUBLIC VIRTUAL void OsLocationInfo::CancelLocationUpdate(IN ILocationUpdateListener* piListener)
{
    IMS_SINT32 i = static_cast<IMS_SINT32>(m_objLocationUpdateRequests.GetSize()) - 1;

    for (; i >= 0; --i)
    {
        LocationUpdateRequest* pRequest = m_objLocationUpdateRequests.GetAt(i);

        if (pRequest->GetListener() == piListener)
        {
            m_objLocationUpdateRequests.RemoveAt(i);

            PlatformContext::GetInstance()->GetSystem()->CancelLocationUpdate(
                    pRequest->GetId(), GetSlotId());

            delete pRequest;
        }
    }
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
    const ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(GetSlotId());
    AString strCountry;

    piSubsInfo->GetNetworkCountryIso(strCountry);

    if (bFromUicc && (strCountry.GetLength() == 0))
    {
        // If country ISO from the attached network is null, get it from UICC.
        piSubsInfo->GetSimCountryIso(strCountry);
    }

    strCountry = strCountry.MakeUpper();

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

PRIVATE
void OsLocationInfo::NotifyLocationUpdateCompleted(IN IMS_SINT32 nId)
{
    for (IMS_UINT32 i = 0; i < m_objLocationUpdateRequests.GetSize(); ++i)
    {
        LocationUpdateRequest* pRequest = m_objLocationUpdateRequests.GetAt(i);

        if (pRequest->GetId() == nId)
        {
            m_objLocationUpdateRequests.RemoveAt(i);
            pRequest->NotifyLocationUpdateCompleted();
            delete pRequest;
            break;
        }
    }
}
