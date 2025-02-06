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
#ifndef MOCK_I_SYSTEM_H_
#define MOCK_I_SYSTEM_H_

#include "AStringArray.h"
#include "ByteArray.h"
#include "ISystem.h"
#include "ISystemListener.h"
#include "ImsAccessNetworkInfoType.h"
#include "ImsParcel.h"
#include "IpSecSaParameter.h"

class MockISystem : public ISystem
{
public:
    inline MockISystem() {}
    inline virtual ~MockISystem() {}

    MOCK_METHOD(void, Destroy, (), (override));
    // Add & remove the listener to receive the system events
    MOCK_METHOD(void, AddListener,
            (IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, RemoveListener,
            (IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId),
            (override));

    ////
    // Common information
    ////
    MOCK_METHOD(void, GetUuid,
            (IN IMS_SINT32 nVersion, IN const AString& strName, OUT AString& strUuid), (override));
    MOCK_METHOD(IMS_SINT32, GetPowerLevel, (), (override));

    ////
    // Device & UICC-related information
    ////
    MOCK_METHOD(IMS_SINT32, GetDeviceId, (OUT AString & strDeviceId, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, GetDeviceSoftwareVersion, (OUT AString & strSv, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(
            IMS_SINT32, GetExternalStoragePath, (OUT AString & strExternalStoragePath), (override));
    MOCK_METHOD(IMS_SINT32, GetPhoneNumber, (OUT AString & strPhoneNumber, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, GetSubscriberId, (OUT AString & strImsi, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, GetSimMcc, (OUT AString & strMcc, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetSimMnc, (OUT AString & strMnc, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetSimCountryIso, (OUT AString & strCountry, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, GetNetworkCountryIso, (OUT AString & strCountry, IN IMS_SINT32 nSlotId),
            (override));
    // For UICC (ISIM)
    MOCK_METHOD(AString, GetIsimState, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(AStringArray, GetIsimRecord, (IN IMS_SINT32 nFileId, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_RESULT, RequestIsimAuthentication,
            (IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId), (override));
    // For UICC (USIM)
    MOCK_METHOD(IMS_RESULT, RequestUsimAuthentication,
            (IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId), (override));

    ////
    // Call-related information
    ////
    MOCK_METHOD(IMS_SINT32, GetCsCallState, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, IsEmergencyNumber, (IN const AString& strNumber, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, GetTtyMode, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetRttMode, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetCsCallStateInOtherSlot, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsCrossSimRedialingAvailable, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetDeviceName, (OUT AString & strDeviceName), (override));

    ////
    // Network-related information (mobile)
    ////
    MOCK_METHOD(IMS_SINT32, GetNetworkType, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetVoiceNetworkType, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetRoamingState, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetVoiceRoamingType, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetDataRoamingType, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetServiceState, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetVoiceServiceState, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetAccessNetworkInfo,
            (IN IMS_SINT32 nDefaultNetworkType, OUT IMS_SINT32& nNetworkType,
                    OUT AStringArray& objAccessNetInfo, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(AStringArray, GetLastAccessNetworkInfo,
            (IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, RequestNetwork, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, ReleaseNetwork, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(AString, GetApnName, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetDataConnectionState, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(AStringArray, GetHostByName,
            (IN const AString& strHost, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nApnType,
                    IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(
            IMS_SINT32, GetIfaceId, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(AString, GetIfaceName, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetIpcanCategory, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(AString, GetLocalAddress,
            (IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(AStringArray, GetPcscfAddresses,
            (IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsImsEmergencyCallSupported, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsImsVoiceCallSupported, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyOnly, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyAttachSupported, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsMobileDataEnabled, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetMocnPlmnInfo, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetMtu, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, BindSocket,
            (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSockFd, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(
            IMS_BOOL, IsIpv6Preferred, (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));

    ////
    // WiFi-related information
    ////
    MOCK_METHOD(AString, GetWifiBssId, (), (override));
    MOCK_METHOD(IMS_SINT32, GetWifiConnectionState, (), (override));
    MOCK_METHOD(IMS_SINT32, GetWifiState, (), (override));
    MOCK_METHOD(AString, GetWifiSsId, (), (override));

    ////
    // Timer APIs
    ////
    MOCK_METHOD(IMS_SINT32, SetTimer, (IN IMS_UINT32 nDuration, IN IMS_UINTP nTimerId), (override));
    MOCK_METHOD(IMS_SINT32, KillTimer, (IN IMS_UINTP nTimerId), (override));

    ////
    // Configuration-related information
    ////
    MOCK_METHOD(IMS_SINT32, GetPreference,
            (IN const AString& strFileName, IN const AString& strKey, IN IMS_SINT32 nSlotId,
                    OUT AString& strValue),
            (override));
    MOCK_METHOD(IMS_SINT32, SetPreference,
            (IN const AString& strFileName, IN const AString& strKey, IN const AString& strValue,
                    IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(AString, GetPrivateProperty,
            (IN IMS_BOOL bPersistent, IN const AString& strKey, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, SetPrivateProperty,
            (IN IMS_BOOL bPersistent, IN const AString& strKey, IN const AString& strValue,
                    IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_BOOL, GetCarrierConfig, (IN IMS_SINT32 nSlotId, OUT ImsParcel& objConfig),
            (override));

    ////
    // Event control (from native to java)
    ////
    MOCK_METHOD(IMS_SINT32, SendEvent,
            (IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
                    IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, SetEvent, (IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, ResetEvent, (IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId), (override));

    ////
    // WFC information
    ////
    MOCK_METHOD(IMS_BOOL, IsWifiCallingEnabled, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetWifiCallingPreferences, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, IsWifiCallingProvisioned, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(AString, GetWifiCallingAddressId, (IN IMS_SINT32 nSlotId), (override));

    ////
    // Location information
    ////
    MOCK_METHOD(IMS_BOOL, StartListeningForLocation,
            (IN IMS_UINT32 nUpdateIntervalInSec, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, StopListeningForLocation, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetLastKnownLocation,
            (OUT AStringArray & objLocationInfo, IN IMS_SINT32 nType, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_BOOL, StartInstantLocationUpdate, (IN IMS_SINT32 nSlotId), (override));

    ////
    // Ims radio interface
    ////
    MOCK_METHOD(IMS_SINT32, StartImsTraffic,
            (IN IMS_UINT32 nId, IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nAccessNetworkType,
                    IN IMS_UINT32 nDirection, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, StopImsTraffic, (IN IMS_UINT32 nId, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, TriggerEpsFallback, (IN IMS_UINT32 nEpsfbReason, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, SetTrafficPriority,
            (IN IMS_UINT32 nPriorityType, IN IMS_SINT32 nSlotId), (override));

    ////
    // IpSec
    ////
    MOCK_METHOD(IMS_SINT32, AddIpSecSaParameter,
            (IN const IpSecSaParameter& objSaParam, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, RemoveIpSecSaParameter, (IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, ApplyIpSecSa,
            (IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd,
                    IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, RemoveIpSecSa,
            (IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd,
                    IN IMS_SINT32 nSlotId),
            (override));
};

#endif
