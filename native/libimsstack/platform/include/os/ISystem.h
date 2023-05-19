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
#ifndef INTERFACE_SYSTEM_H_
#define INTERFACE_SYSTEM_H_

#include "AStringArray.h"
#include "ByteArray.h"
#include "ISystemListener.h"
#include "ImsParcel.h"
#include "IpSecSaParameter.h"

class ISystem
{
protected:
    virtual ~ISystem() = default;

public:
    virtual void Destroy() = 0;
    // Add & remove the listener to receive the system events
    virtual void AddListener(
            IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId) = 0;
    virtual void RemoveListener(
            IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId) = 0;

    ////
    // Power-related information
    ////
    virtual IMS_SINT32 GetPowerLevel() = 0;

    ////
    // Device & UICC-related information
    ////
    virtual IMS_SINT32 GetDeviceId(OUT AString& strDeviceId, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetDeviceSoftwareVersion(OUT AString& strSv, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetExternalStoragePath(OUT AString& strExternalStoragePath) = 0;
    virtual IMS_SINT32 GetPhoneNumber(OUT AString& strPhoneNumber, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetSubscriberId(OUT AString& strImsi, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetSimMcc(OUT AString& strMcc, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetSimMnc(OUT AString& strMnc, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetSimCountryIso(OUT AString& strCountry, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetNetworkCountryIso(OUT AString& strCountry, IN IMS_SINT32 nSlotId) = 0;
    // For UICC (ISIM)
    virtual AString GetIsimState(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 ReadIsimFileAttributes(IN IMS_SINT32 nFileId, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 ReadIsimRecord(
            IN IMS_SINT32 nFileId, IN IMS_SINT32 nIndex, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_RESULT RequestIsimAuthentication(
            IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId) = 0;
    // For UICC (USIM)
    virtual IMS_RESULT RequestUsimAuthentication(
            IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId) = 0;

    ////
    // Call-related information
    ////
    virtual IMS_SINT32 GetCsCallState(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 IsEmergencyNumber(IN const AString& strNumber, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetTtyMode(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetRttMode(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetCsCallStateInOtherSlot(IN IMS_SINT32 nSlotId) = 0;

    virtual IMS_SINT32 GetDeviceName(OUT AString& strDeviceName) = 0;
    virtual IMS_SINT32 GetDigestSha1(IN const AString& strIn, OUT AString& strOut) = 0;

    ////
    // Network-related information (mobile)
    ////
    virtual IMS_SINT32 GetNetworkType(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetVoiceNetworkType(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetRoamingState(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetVoiceRoamingType(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetDataRoamingType(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetServiceState(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetVoiceServiceState(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetAccessNetworkInfo(IN IMS_SINT32 nDefaultNetworkType,
            OUT IMS_SINT32& nNetworkType, OUT AStringArray& objAccessNetInfo,
            IN IMS_SINT32 nSlotId) = 0;
    virtual AStringArray GetLastAccessNetworkInfo(
            IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 ActivateDataConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 DeactivateDataConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual AString GetApnName(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetDataConnectionState(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual AStringArray GetHostByName(IN const AString& strHost, IN IMS_SINT32 nIpVersion,
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetIfaceId(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual AString GetIfaceName(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetIpcanCategory(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual AString GetLocalAddress(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId) = 0;
    virtual AStringArray GetPcscfAddresses(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL IsImsEmergencyCallSupported(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL IsImsVoiceCallSupported(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL IsLteEmergencyOnly(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL IsEmergencyAttachSupported(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL IsMobileDataEnabled(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetMocnPlmnInfo(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetMtu(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 BindSocket(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSockFd, IN IMS_SINT32 nSlotId) = 0;

    ////
    // WiFi-related information
    ////
    virtual AString GetWifiBssId() = 0;
    virtual IMS_SINT32 GetWifiConnectionState() = 0;
    virtual IMS_SINT32 GetWifiState() = 0;
    virtual AString GetWifiSsId() = 0;

    ////
    // Timer APIs
    ////
    virtual IMS_SINT32 SetTimer(IN IMS_UINT32 nDuration, IN IMS_UINTP nTimerId) = 0;
    virtual IMS_SINT32 KillTimer(IN IMS_UINTP nTimerId) = 0;

    ////
    // Configuration-related information
    ////
    virtual IMS_SINT32 GetPreference(IN const AString& strFileName, IN const AString& strKey,
            IN IMS_SINT32 nSlotId, OUT AString& strValue) = 0;
    virtual IMS_SINT32 SetPreference(IN const AString& strFileName, IN const AString& strKey,
            IN const AString& strValue, IN IMS_SINT32 nSlotId) = 0;
    virtual AString GetPrivateProperty(
            IN IMS_BOOL bPersistent, IN const AString& strKey, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 SetPrivateProperty(IN IMS_BOOL bPersistent, IN const AString& strKey,
            IN const AString& strValue, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL GetCarrierConfig(IN IMS_SINT32 nSlotId, OUT ImsParcel& objConfig) = 0;

    ////
    // Event control (from native to java)
    ////
    virtual IMS_SINT32 SendEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
            IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 SetEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 ResetEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId) = 0;

    ////
    // WFC information
    ////
    virtual IMS_BOOL IsWifiCallingEnabled(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetWifiCallingPreferences(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL IsWifiCallingProvisioned(IN IMS_SINT32 nSlotId) = 0;
    virtual AString GetWifiCallingAddressId(IN IMS_SINT32 nSlotId) = 0;

    ////
    // Location information
    ////
    virtual IMS_BOOL StartListeningForLocation(
            IN IMS_UINT32 nUpdateIntervalInSec, IN IMS_SINT32 nSlotId) = 0;
    virtual void StopListeningForLocation(IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 GetLastKnownLocation(
            OUT AStringArray& objLocationInfo, IN IMS_SINT32 nType, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_BOOL StartInstantLocationUpdate(IN IMS_SINT32 nSlotId) = 0;

    ////
    // Ims radio interface
    ////
    virtual IMS_SINT32 StartImsTraffic(IN IMS_UINT32 nId, IN IMS_UINT32 nTrafficType,
            IN IMS_UINT32 nAccessNetworkType, IN IMS_UINT32 nDirection, IN IMS_SINT32 nSlotId) = 0;
    virtual void StopImsTraffic(IN IMS_UINT32 nId, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason, IN IMS_SINT32 nSlotId) = 0;

    ////
    // IpSec
    ////
    virtual IMS_SINT32 AddIpSecSaParameter(
            IN const IpSecSaParameter& objSaParam, IN IMS_SINT32 nSlotId) = 0;
    virtual void RemoveIpSecSaParameter(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_SINT32 ApplyIpSecSa(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi,
            IN IMS_SINT32 nSocketFd, IN IMS_SINT32 nSlotId) = 0;
    virtual void RemoveIpSecSa(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd,
            IN IMS_SINT32 nSlotId) = 0;
};

#endif
