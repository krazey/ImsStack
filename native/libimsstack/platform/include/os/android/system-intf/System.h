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
#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <binder/Parcel.h>

#include "AStringArray.h"
#include "ByteArray.h"
#include "IpSecSaParameter.h"
#include "system-intf/ISystemListener.h"

class AccessNetworkInfo;
class SystemPrivate;
class SystemCallback;

class System
{
private:
    System();
    virtual ~System();

    System(IN const System&) = delete;
    System& operator=(IN const System&) = delete;

public:
    static System* GetInstance();

public:
    void SetCallback(IN SystemCallback* pCallback);

    // It's called from the JNI to notify the system events
    void NotifyData(IN const android::Parcel& in, OUT android::Parcel& out);

    // Add & remove the listener to receive the system events
    void AddListener(
            IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId);
    void RemoveListener(
            IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId);

    ////
    // Power-related information
    ////
    IMS_SINT32 GetPowerLevel();

    ////
    // Device&  UICC-related information
    ////
    IMS_SINT32 GetDeviceId(OUT AString& strDeviceId, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetDeviceSoftwareVersion(OUT AString& strSv, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetExternalStoragePath(OUT AString& strExternalStoragePath);
    IMS_SINT32 GetPhoneNumber(OUT AString& strPhoneNumber, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetSubscriberId(OUT AString& strImsi, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetMcc(OUT AString& strMcc, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetMnc(OUT AString& strMnc, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetOperator(OUT AString& strOperator, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetCountry(OUT AString& strCountry, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetNetworkCountry(OUT AString& strCountry, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetEmergencyNumberListFromSim(OUT AString& strEnlFromSim, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetEmergencyPriorityFromModem(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsUiccGbaSupported(IN IMS_SINT32 nSlotId);

    // For UICC (ISIM)
    AString GetIsimState(IN IMS_SINT32 nSlotId);
    IMS_SINT32 ReadIsimFileAttributes(IN IMS_SINT32 nFileId, IN IMS_SINT32 nSlotId);
    IMS_SINT32 ReadIsimRecord(IN IMS_SINT32 nFileId, IN IMS_SINT32 nIndex, IN IMS_SINT32 nSlotId);
    IMS_RESULT RequestIsimAuthentication(
            IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId);
    // For UICC (USIM)
    IMS_RESULT RequestUsimAuthentication(
            IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId);

    ////
    // Call-related information
    ////
    IMS_SINT32 GetCallState(IN IMS_SINT32 nSlotId);
    IMS_SINT32 IsEmergencyNumber(IN const AString& strNumber, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetTtyMode(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetRttMode(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetCallStateInOtherSlot(IN IMS_SINT32 nSlotId);

    IMS_SINT32 GetDeviceName(OUT AString& strDeviceName);
    IMS_SINT32 GetDigestSha1(IN const AString& strIn, OUT AString& strOut);

    ////
    // Network-related information (mobile)
    ////
    IMS_SINT32 GetNetworkType(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetVoiceNetworkType(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetRoamingState(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetVoiceRoamingType(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetDataRoamingType(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetServiceState(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetVoiceServiceState(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetAccessNetworkInfo(IN IMS_SINT32 nDefaultNetworkType, OUT IMS_SINT32& nNetworkType,
            OUT AStringArray& objAccessNetInfo, IN IMS_SINT32 nSlotId);
    AStringArray GetLastAccessNetworkInfo(IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 ActivateDataConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 DeactivateDataConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    AString GetApnName(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetDataConnectionState(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    AStringArray GetHostByName(IN const AString& strHost, IN IMS_SINT32 nIpVersion,
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetIfaceId(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    AString GetIfaceName(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetIpcanCategory(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    AString GetLocalAddress(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId);
    AStringArray GetPcscfAddresses(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId);
    IMS_BOOL IsImsEmergencyCallSupported(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsImsVoiceCallSupported(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetLteRsrpStrength(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsLteEmergencyOnly(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsEmergencyAttachSupported(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsMobileDataEnabled(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetMocnPlmnInfo(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetMtu(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 BindSocket(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSockFd, IN IMS_SINT32 nSlotId);

    ////
    // WiFi-related information
    ////
    AString GetWifiBssId();
    IMS_SINT32 GetWifiDetailedState();
    IMS_SINT32 GetWifiState();
    AString GetWifiSsId();

    ////
    // Alarm timer control
    ////
    IMS_SINT32 SetAlarm(IN IMS_UINT32 nDuration, IN IMS_UINTP nAlarmId);
    IMS_SINT32 KillAlarm(IN IMS_UINTP nAlarmId);

    ////
    // Configuration-related information
    ////
    IMS_SINT32 GetPreference(IN const AString& strFileName, IN const AString& strKey,
            IN IMS_UINT32 nValueType, IN IMS_SINT32 nSlotId, OUT AString& strValue);
    IMS_SINT32 SetPreference(IN const AString& strFileName, IN const AString& strKey,
            IN IMS_UINT32 nValueType, IN const AString& strValue, IN IMS_SINT32 nSlotId);
    AString GetPrivateProperty(
            IN IMS_BOOL bPersistent, IN const AString& strKey, IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetPrivateProperty(IN IMS_BOOL bPersistent, IN const AString& strKey,
            IN const AString& strValue, IN IMS_SINT32 nSlotId);
    IMS_BOOL GetCarrierConfig(IN IMS_SINT32 nSlotId, OUT android::Parcel& objConfig);

    ////
    // Event control (from native to java)
    ////
    IMS_SINT32 SendEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
            IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId);
    IMS_SINT32 ResetEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId);

    ////
    // WFC information
    ////
    IMS_BOOL IsWifiCallingEnabled(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetWifiCallingPreferences(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsWifiCallingProvisioned(IN IMS_SINT32 nSlotId);
    AString GetWifiCallingAddressId(IN IMS_SINT32 nSlotId);

    ////
    // Location information
    ////
    IMS_BOOL StartLocationInfo(IN IMS_UINT32 nUpdateIntervalInSec, IN IMS_SINT32 nSlotId);
    void StopLocationInfo(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetLocationInformation(
            OUT AStringArray& objLocationInfo, IN IMS_SINT32 nType, IN IMS_SINT32 nSlotId);
    IMS_BOOL MakeInstantLocationInfo(IN IMS_SINT32 nSlotId);

    ////
    // TRM
    ////
    IMS_SINT32 SetTrm(IN IMS_UINT32 nServiceType, IN IMS_SINT32 nSlotId);

    ////
    // VoNR
    ////
    IMS_SINT32 NotifyCallState(IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode,
            IN IMS_UINT32 nDirection, IN IMS_SINT32 nSlotId);
    IMS_SINT32 RequestCallPreference(
            IN IMS_UINT32 nRat, IN IMS_UINT32 nType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetImsSession(IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetImsVoice(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode, IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetImsSignaling(IN IMS_UINT32 nType, IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetUacCheck(IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_SINT32 nSlotId);
    IMS_SINT32 SetVoice(IN IMS_UINT32 nState, IN IMS_BOOL bIsEmergency, IN IMS_SINT32 nSlotId);

    ////
    // IpSec
    ////
    IMS_SINT32 AddIpSecSaParameter(IN const IpSecSaParameter& objSaParam, IN IMS_SINT32 nSlotId);
    void RemoveIpSecSaParameter(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSlotId);
    IMS_SINT32 ApplyIpSecSa(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd,
            IN IMS_SINT32 nSlotId);
    void RemoveIpSecSa(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd,
            IN IMS_SINT32 nSlotId);

private:
    IMS_SINT32 GetInt(IN IMS_SINT32 nOperation, IN IMS_SINT32 nDefaultValue = 0,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_SINT32 GetInt2(IN IMS_SINT32 nOperation, IN IMS_SINT32 nParam,
            IN IMS_SINT32 nDefaultValue = 0, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_SINT32 GetString(
            IN IMS_SINT32 nOperation, OUT AString& strValue, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_SINT32 RequestSimAuthentication(IN IMS_SINT32 nOperation, IN const AString& strNonce,
            IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    void AddListenerIfCategoryMatched(IN IMS_UINT32 nCategories, IN IMS_UINT32 nCategory,
            IN ISystemListener* piListener, IN IMS_SINT32 nSlotId);
    void RemoveListenerIfCategoryMatched(IN IMS_UINT32 nCategories, IN IMS_UINT32 nCategory,
            IN ISystemListener* piListener, IN IMS_SINT32 nSlotId);

    void NotifyNetworkCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyWifiCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyCallCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyPowerCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyAlarmCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyConfigCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyEventCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifySimCategory(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN IMS_UINT32 nCategory,
            IN const android::Parcel& in);
    void NotifyTrmCategory(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);
    void NotifyVoNrCategory(
            IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in);

private:
    SystemPrivate* m_pSystemP;
    SystemCallback* m_pCallback;
};

#endif
