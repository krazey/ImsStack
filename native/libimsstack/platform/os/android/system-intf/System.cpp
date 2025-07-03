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
#include <utils/String8.h>

#include "ImsMap.h"
#include "ImsStrLib.h"
#include "OsParcel.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"
#include "system-intf/System.h"
#include "system-intf/SystemCallback.h"
#include "system-intf/SystemConstants.h"

using namespace android;

__IMS_TRACE_TAG_IPL__;

class SystemListenerHolder
{
public:
    SystemListenerHolder();
    ~SystemListenerHolder();

    SystemListenerHolder(IN const SystemListenerHolder&) = delete;
    SystemListenerHolder& operator=(IN const SystemListenerHolder&) = delete;

public:
    void AddListener(IN IMS_UINT32 nCategory, IN ISystemListener* piListener);
    void RemoveListener(IN IMS_UINT32 nCategory, const IN ISystemListener* piListener);
    ImsList<ISystemListener*>* GetListeners(IN IMS_UINT32 nCategory);

    static const IMS_CHAR* CategoryToString(IN IMS_UINT32 nCategory);

private:
    IMutex* m_piLock;
    // <category, listener>
    ImsMap<IMS_UINT32, ImsList<ISystemListener*>> m_objListenerMap;
};

PUBLIC
SystemListenerHolder::SystemListenerHolder() :
        m_piLock(IMS_NULL),
        m_objListenerMap(ImsMap<IMS_UINT32, ImsList<ISystemListener*>>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();

    // Initialize the listener map to avoid memory re-allocation
    ImsList<ISystemListener*> objListeners;
    // clang-format off
    IMS_UINT32 nCategories[] = {
            SystemConstants::CATEGORY_NETWORK,
            SystemConstants::CATEGORY_WIFI,
            SystemConstants::CATEGORY_POWER,
            SystemConstants::CATEGORY_TIMER,
            SystemConstants::CATEGORY_CONFIG,
            SystemConstants::CATEGORY_EVENT,
            SystemConstants::CATEGORY_ISIM,
            SystemConstants::CATEGORY_USIM,
            SystemConstants::CATEGORY_RADIO,
            SystemConstants::CATEGORY_LOCATION
    };
    // clang-format on
    IMS_UINT32 nCategoryCount = sizeof(nCategories) / sizeof(nCategories[0]);

    for (IMS_UINT32 i = 0; i < nCategoryCount; ++i)
    {
        m_objListenerMap.Add(nCategories[i], objListeners);
    }
}

PUBLIC
SystemListenerHolder::~SystemListenerHolder()
{
    m_objListenerMap.Clear();
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
ImsList<ISystemListener*>* SystemListenerHolder::GetListeners(IN IMS_UINT32 nCategory)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objListenerMap.GetIndexOfKey(nCategory);

    if (nIndex < 0)
    {
        IMS_TRACE_D("%s :: No category", CategoryToString(nCategory), 0, 0);
        return IMS_NULL;
    }

    ImsList<ISystemListener*>& objListeners = m_objListenerMap.GetValueAt(nIndex);

    if (objListeners.IsEmpty())
    {
        IMS_TRACE_D("%s :: No listeners", CategoryToString(nCategory), 0, 0);
        return IMS_NULL;
    }

    return &objListeners;
}

PUBLIC
void SystemListenerHolder::AddListener(IN IMS_UINT32 nCategory, IN ISystemListener* piListener)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objListenerMap.GetIndexOfKey(nCategory);

    if (nIndex < 0)
    {
        ImsList<ISystemListener*> objListeners;

        if (!objListeners.Append(piListener))
        {
            IMS_TRACE_E(0, "Adding a listener failed", 0, 0, 0);
            return;
        }

        m_objListenerMap.Add(nCategory, objListeners);
        return;
    }

    ImsList<ISystemListener*>& objListeners = m_objListenerMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISystemListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piTmpListener == piListener)
        {
            IMS_TRACE_D("Listener is already added", 0, 0, 0);
            return;
        }
    }

    if (!objListeners.Append(piListener))
    {
        IMS_TRACE_E(0, "Adding a listener failed", 0, 0, 0);
    }
}

PRIVATE
void SystemListenerHolder::RemoveListener(
        IN IMS_UINT32 nCategory, IN const ISystemListener* piListener)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objListenerMap.GetIndexOfKey(nCategory);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<ISystemListener*>& objListeners = m_objListenerMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISystemListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piTmpListener == piListener)
        {
            objListeners.RemoveAt(i);
            break;
        }
    }

    // Do not remove the item from the map;
    // Keep the list of listeners as an empty list.
    // if (objListeners.IsEmpty())
    //{
    //     objListenerMap.RemoveAt(nIndex);
    //}
}

PRIVATE GLOBAL const IMS_CHAR* SystemListenerHolder::CategoryToString(IN IMS_UINT32 nCategory)
{
    switch (nCategory)
    {
        case SystemConstants::CATEGORY_NETWORK:
            return "CATEGORY_NETWORK";
        case SystemConstants::CATEGORY_WIFI:
            return "CATEGORY_WIFI";
        case SystemConstants::CATEGORY_POWER:
            return "CATEGORY_POWER";
        case SystemConstants::CATEGORY_TIMER:
            return "CATEGORY_TIMER";
        case SystemConstants::CATEGORY_CONFIG:
            return "CATEGORY_CONFIG";
        case SystemConstants::CATEGORY_EVENT:
            return "CATEGORY_EVENT";
        case SystemConstants::CATEGORY_ISIM:
            return "CATEGORY_ISIM";
        case SystemConstants::CATEGORY_USIM:
            return "CATEGORY_USIM";
        case SystemConstants::CATEGORY_RADIO:
            return "CATEGORY_RADIO";
        case SystemConstants::CATEGORY_LOCATION:
            return "CATEGORY_LOCATION";
        default:
            return "__UNKNOWN__";
    }
}

class SystemPrivate
{
public:
    SystemPrivate();
    ~SystemPrivate();

    SystemPrivate(IN const SystemPrivate&) = delete;
    SystemPrivate& operator=(IN const SystemPrivate&) = delete;

public:
    inline SystemListenerHolder* GetListenerHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppListenerHolder[nSlotId];
    }

private:
    SystemListenerHolder** m_ppListenerHolder;
};

PUBLIC
SystemPrivate::SystemPrivate()
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppListenerHolder = new SystemListenerHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppListenerHolder[i] = new SystemListenerHolder();
    }
}

PUBLIC
SystemPrivate::~SystemPrivate()
{
    if (m_ppListenerHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppListenerHolder[i] != IMS_NULL)
            {
                delete m_ppListenerHolder[i];
            }
        }

        delete[] m_ppListenerHolder;
    }
}

PRIVATE
System::System() :
        m_pSystemP(new SystemPrivate()),
        m_pCallback(IMS_NULL)
{
}

PRIVATE
System::~System()
{
    if (m_pSystemP != IMS_NULL)
    {
        delete m_pSystemP;
    }
}

PUBLIC GLOBAL System* System::GetInstance()
{
    static System* s_pSystem = IMS_NULL;

    if (s_pSystem == IMS_NULL)
    {
        s_pSystem = new System();
    }

    return s_pSystem;
}

PUBLIC
void System::SetCallback(IN SystemCallback* pCallback)
{
    m_pCallback = pCallback;
}

/*
case SystemConstants::NOTIFY_SERVICE_STATE_CHANGED:
case SystemConstants::NOTIFY_DATA_CONNECTION_STATE_CHANGED:
    listener[network]->No....(cmd, par.readint(), 0);
    listener[eventreceiver]->No....(cmd, par.readint(), 0);
    break;
*/
PUBLIC
void System::NotifyData(IN const android::Parcel& in, OUT android::Parcel& out)
{
    // cppcheck-suppress duplicateAssignExpression
    IMS_SINT32 nSlotId = in.readInt32();
    IMS_SINT32 nCmd = in.readInt32();
    AString strLog;

    strLog.Sprintf("slotId=%d, cmd=%08X, category=%08X, sub-category=%d", nSlotId, nCmd,
            (nCmd & 0xFFFF0000), (nCmd & 0xFFFF));

    IMS_TRACE_I("NotifyData :: %s", strLog.GetStr(), 0, 0);

    if ((nCmd & SystemConstants::CATEGORY_NETWORK) == SystemConstants::CATEGORY_NETWORK)
    {
        NotifyNetworkCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_WIFI) == SystemConstants::CATEGORY_WIFI)
    {
        NotifyWifiCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_POWER) == SystemConstants::CATEGORY_POWER)
    {
        NotifyPowerCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_TIMER) == SystemConstants::CATEGORY_TIMER)
    {
        NotifyTimerCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_CONFIG) == SystemConstants::CATEGORY_CONFIG)
    {
        NotifyConfigCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_EVENT) == SystemConstants::CATEGORY_EVENT)
    {
        NotifyEventCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_ISIM) == SystemConstants::CATEGORY_ISIM)
    {
        NotifySimCategory(nSlotId, nCmd, SystemConstants::CATEGORY_ISIM, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_USIM) == SystemConstants::CATEGORY_USIM)
    {
        NotifySimCategory(nSlotId, nCmd, SystemConstants::CATEGORY_USIM, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_RADIO) == SystemConstants::CATEGORY_RADIO)
    {
        NotifyRadioCategory(nSlotId, nCmd, in);
    }
    else if ((nCmd & SystemConstants::CATEGORY_LOCATION) == SystemConstants::CATEGORY_LOCATION)
    {
        NotifyLocationCategory(nSlotId, nCmd, in);
    }
    else
    {
        out.writeInt32(0);
        return;
    }

    out.writeInt32(1);
}

PUBLIC
void System::AddListener(
        IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("AddListener :: category=0x%08x, listener=%p, slotId=%d", nCategory, piListener,
            nSlotId);

    if (piListener == IMS_NULL)
    {
        return;
    }

    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_NETWORK, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_WIFI, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_POWER, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_TIMER, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_CONFIG, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_EVENT, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_ISIM, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_USIM, piListener, nSlotId);
    AddListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_RADIO, piListener, nSlotId);
    AddListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_LOCATION, piListener, nSlotId);
}

PUBLIC
void System::RemoveListener(
        IN IMS_UINT32 nCategory, IN ISystemListener* piListener, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("RemoveListener :: category=0x%08x, listener=%p, slotId=%d", nCategory, piListener,
            nSlotId);

    if (piListener == IMS_NULL)
    {
        return;
    }

    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_NETWORK, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_WIFI, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_POWER, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_TIMER, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_CONFIG, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_EVENT, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_ISIM, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(nCategory, SystemConstants::CATEGORY_USIM, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_RADIO, piListener, nSlotId);
    RemoveListenerIfCategoryMatched(
            nCategory, SystemConstants::CATEGORY_LOCATION, piListener, nSlotId);
}

PUBLIC
void System::GetUuid(IN IMS_SINT32 nVersion, IN const AString& strName, OUT AString& strUuid)
{
    strUuid = AString::ConstNull();

    if (m_pCallback == IMS_NULL)
    {
        return;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::GET_UUID);
    in.writeInt32(nVersion);

    if (strName.GetLength() != 0)
    {
        String16 str16Name(strName.GetStr());
        in.writeString16(str16Name);
    }

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16 = out.readString16();
        String8 str8(str16);
        strUuid = str8.c_str();
    }
}

PUBLIC
IMS_SINT32 System::GetPowerLevel()
{
    return GetInt(SystemConstants::GET_BATTERY_LEVEL);
}

PUBLIC
IMS_SINT32 System::GetDeviceId(OUT AString& strDeviceId, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_DEVICE_ID, strDeviceId, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetDeviceSoftwareVersion(OUT AString& strSv, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_DEVICE_SOFTWARE_VERSION, strSv, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetExternalStoragePath(OUT AString& strExternalStoragePath)
{
    return GetString(SystemConstants::GET_EXTERNAL_STORAGE_PATH, strExternalStoragePath);
}

PUBLIC
IMS_SINT32 System::GetPhoneNumber(OUT AString& strPhoneNumber, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_PHONE_NUMBER, strPhoneNumber, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetSubscriberId(OUT AString& strImsi, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_SUBSCRIBER_ID, strImsi, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetSimMcc(OUT AString& strMcc, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_SIM_MCC, strMcc, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetSimMnc(OUT AString& strMnc, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_SIM_MNC, strMnc, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetSimCountryIso(OUT AString& strCountry, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_SIM_COUNTRY_ISO, strCountry, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetNetworkCountryIso(OUT AString& strCountry, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_NETWORK_COUNTRY_ISO, strCountry, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetNetworkOperator(OUT AString& strOperator, IN IMS_SINT32 nSlotId)
{
    return GetString(SystemConstants::GET_NETWORK_OPERATOR, strOperator, nSlotId);
}

PUBLIC
AString System::GetIsimState(IN IMS_SINT32 nSlotId)
{
    AString strIsimState(AString::ConstNull());

    GetString(SystemConstants::GET_ISIM_STATE, strIsimState, nSlotId);

    return strIsimState;
}

PUBLIC
AStringArray System::GetIsimRecord(IN IMS_SINT32 nFileId, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_ISIM_RECORD);
    in.writeInt32(nFileId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        AStringArray objRecords;
        IMS_SINT32 nCount = out.readInt32();

        for (IMS_SINT32 i = 0; i < nCount; ++i)
        {
            String16 str16Temp = out.readString16();
            String8 str8Temp(str16Temp);
            objRecords.AddElement(AString(str8Temp.c_str()));
        }

        return objRecords;
    }

    return AStringArray::ConstNull();
}

PUBLIC
IMS_SINT32 System::RequestIsimAuthentication(
        IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId)
{
    return RequestSimAuthentication(SystemConstants::REQUEST_ISIM_AUTH, strNonce, nOwner, nSlotId);
}

PUBLIC
IMS_SINT32 System::RequestUsimAuthentication(
        IN const AString& strNonce, IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId)
{
    return RequestSimAuthentication(SystemConstants::REQUEST_USIM_AUTH, strNonce, nOwner, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetCsCallState(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_CS_CALL_STATE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::IsEmergencyNumber(IN const AString& strNumber, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::IS_EMERGENCY_NUMBER);

    String16 str16Number(strNumber.GetStr());
    in.writeString16(str16Number);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::GetTtyMode(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_TTY_MODE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetRttMode(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_RTT_MODE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetCsCallStateInOtherSlot(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_CS_CALL_STATE_IN_OTHER_SLOT, 0, nSlotId);
}

PUBLIC
IMS_BOOL System::IsCrossSimRedialingAvailable(IN IMS_SINT32 nSlotId)
{
    return (GetInt2(SystemConstants::IS_CROSS_SIM_REDIALING_AVAILABLE, nSlotId) == 1);
}

PUBLIC
IMS_SINT32 System::GetDeviceName(OUT AString& strDeviceName)
{
    return GetString(SystemConstants::GET_DEVICE_NAME, strDeviceName);
}

PUBLIC
IMS_SINT32 System::GetNetworkType(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_NETWORK_TYPE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetVoiceNetworkType(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_VOICE_NETWORK_TYPE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetRoamingState(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_ROAMING_STATE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetVoiceRoamingType(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_VOICE_ROAMING_TYPE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetDataRoamingType(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_DATA_ROAMING_TYPE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetServiceState(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_SERVICE_STATE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetCellularServiceState(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_CELLULAR_SERVICE_STATE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetVoiceServiceState(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_VOICE_SERVICE_STATE, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetAccessNetworkInfo(IN IMS_SINT32 nDefaultNetworkType,
        OUT IMS_SINT32& nNetworkType, OUT AStringArray& objAccessNetInfo, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_ACCESS_NETWORK_INFO);
    in.writeInt32(nDefaultNetworkType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        nNetworkType = out.readInt32();

        IMS_SINT32 nCount = out.readInt32();

        for (IMS_SINT32 i = 0; i < nCount; i++)
        {
            String16 str16Address = out.readString16();
            String8 str8Address(str16Address);
            AString strAddress(str8Address.c_str());
            objAccessNetInfo.AddElement(strAddress);
        }
        return 1;
    }
    else
    {
        nNetworkType = nDefaultNetworkType;
    }

    return 0;
}

PUBLIC
AStringArray System::GetLastAccessNetworkInfo(IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_LAST_ACCESS_NETWORK_INFO);
    in.writeInt32(nNetworkType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        AStringArray objLanInfo;
        IMS_SINT32 nCount = out.readInt32();

        for (IMS_SINT32 i = 0; i < nCount; i++)
        {
            String16 str16Temp = out.readString16();
            String8 str8Temp(str16Temp);
            AString strAddress(str8Temp.c_str());
            objLanInfo.AddElement(strAddress);
        }

        return objLanInfo;
    }

    return AStringArray::ConstNull();
}

PUBLIC
IMS_SINT32 System::GetMocnPlmnInfo(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_MOCN_PLMN_INFO, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetMtu(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    return GetInt2(SystemConstants::GET_MTU, nApnType, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::BindSocket(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSockFd, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::BIND_SOCKET);
    in.writeInt32(nApnType);

    if (m_pCallback->SendDataToJava(in, out, nSockFd) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
IMS_BOOL System::IsIpv6Preferred(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    return (GetInt2(SystemConstants::IS_IPV6_PREFERRED, nApnType, 0, nSlotId) == 1);
}

PUBLIC
IMS_SINT32 System::GetNetworkRegistrationRejectCause(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_NETWORK_REGISTRATION_REJECT_CAUSE, 0, nSlotId);
}

PUBLIC
IMS_BOOL System::IsImsEmergencyCallSupported(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_IMS_EMERGENCY_CALL_SUPPORTED, 0, nSlotId) == 1);
}

PUBLIC
IMS_BOOL System::IsEmergencyOnly(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_EMERGENCY_ONLY, 0, nSlotId) == 1);
}

PUBLIC
IMS_BOOL System::IsEmergencyAttachSupported(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_EMERGENCY_ATTACH_SUPPORTED, 0, nSlotId) == 1);
}

PUBLIC
IMS_BOOL System::IsMobileDataEnabled(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_MOBILE_DATA_ENABLED, 0, nSlotId) == 1);
}

PUBLIC
IMS_BOOL System::IsImsVoiceCallSupported(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_IMS_VOICE_CALL_SUPPORTED, 0, nSlotId) == 1);
}

PUBLIC
IMS_SINT32 System::RequestNetwork(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::REQUEST_NETWORK);
    in.writeInt32(nApnType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::ReleaseNetwork(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::RELEASE_NETWORK);
    in.writeInt32(nApnType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
AString System::GetApnName(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AString::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_APN_NAME);
    in.writeInt32(nApnType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16Apn = out.readString16();
        String8 str8Apn(str16Apn);

        return AString(str8Apn.c_str());
    }

    return AString::ConstNull();
}

PUBLIC
IMS_SINT32 System::GetDataConnectionState(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    return GetInt2(SystemConstants::GET_DATA_CONNECTION_STATE, nApnType, 0, nSlotId);
}

PUBLIC
AStringArray System::GetHostByName(IN const AString& strHost, IN IMS_SINT32 nIpVersion,
        IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_HOST_BY_NAME);

    in.writeInt32(nApnType);
    in.writeInt32(nIpVersion);

    String16 str16Host(strHost.GetStr());
    in.writeString16(str16Host);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        AStringArray objIpAddrs;
        IMS_SINT32 nCount = out.readInt32();

        for (IMS_SINT32 i = 0; i < nCount; i++)
        {
            String16 str16Address = out.readString16();
            String8 str8Address(str16Address);
            objIpAddrs.AddElement(AString(str8Address.c_str()));
        }

        return objIpAddrs;
    }

    return AStringArray::ConstNull();
}

PUBLIC
IMS_SINT32 System::GetIfaceId(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    return GetInt2(SystemConstants::GET_IFACE_ID, nApnType, -1, nSlotId);
}

PUBLIC
AString System::GetIfaceName(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AString::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_IFACE_NAME);
    in.writeInt32(nApnType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16Iface = out.readString16();
        String8 str8Iface(str16Iface);
        return AString(str8Iface.c_str());
    }

    return AString::ConstNull();
}

PUBLIC
IMS_SINT32 System::GetIpcanCategory(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    // 0 - IIpcan::CATEGORY_MOBILE
    return GetInt2(SystemConstants::GET_IPCAN_CATEGORY, nApnType, 0, nSlotId);
}

PUBLIC
AString System::GetLocalAddress(
        IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AString::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_LOCAL_ADDRESS);

    in.writeInt32(nApnType);
    in.writeInt32(nIpVersion);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16Ip = out.readString16();
        String8 str8Ip(str16Ip);

        return AString(str8Ip.c_str());
    }

    return AString::ConstNull();
}

PUBLIC
AStringArray System::GetPcscfAddresses(
        IN IMS_SINT32 nApnType, IN IMS_SINT32 nIpVersion, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_PCSCF_ADDRESSES);

    in.writeInt32(nApnType);
    in.writeInt32(nIpVersion);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        AStringArray objPcscfAddresses;
        IMS_SINT32 nCount = out.readInt32();

        for (IMS_SINT32 i = 0; i < nCount; i++)
        {
            String16 str16Address = out.readString16();
            String8 str8Address(str16Address);
            objPcscfAddresses.AddElement(AString(str8Address.c_str()));
        }

        return objPcscfAddresses;
    }

    return AStringArray::ConstNull();
}

PUBLIC
AString System::GetWifiBssId()
{
    AString strWifiBssId(AString::ConstNull());

    GetString(SystemConstants::GET_WIFI_BSS_ID, strWifiBssId);

    return strWifiBssId;
}

PUBLIC
IMS_SINT32 System::GetWifiConnectionState()
{
    return GetInt(SystemConstants::GET_WIFI_CONNECTION_STATE);
}

PUBLIC
IMS_SINT32 System::GetWifiState()
{
    return GetInt(SystemConstants::GET_WIFI_STATE, 1 /*WIFI_STATE_DISABLED*/);
}

PUBLIC
AString System::GetWifiSsId()
{
    AString strWifiSsId(AString::ConstNull());

    GetString(SystemConstants::GET_WIFI_SSID, strWifiSsId);

    return strWifiSsId;
}

PUBLIC
IMS_SINT32 System::SetTimer(IN IMS_UINT32 nDuration, IN IMS_UINTP nTimerId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::SET_TIMER);

    in.writeInt32(nDuration);
    in.writeInt64(nTimerId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::KillTimer(IN IMS_UINTP nTimerId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::KILL_TIMER);

    in.writeInt64(nTimerId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::GetPreference(IN const AString& strFileName, IN const AString& strKey,
        IN IMS_SINT32 nSlotId, OUT AString& strValue)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::GET_PREFERENCE);

    String16 str16FileName(strFileName.GetStr());
    String16 str16Key(strKey.GetStr());

    in.writeString16(str16FileName);
    in.writeString16(str16Key);
    in.writeInt32(nSlotId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16Value = out.readString16();
        String8 str8Value(str16Value);

        strValue = AString(str8Value.c_str());

        return 1;
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::SetPreference(IN const AString& strFileName, IN const AString& strKey,
        IN const AString& strValue, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::SET_PREFERENCE);

    String16 str16FileName(strFileName.GetStr());
    String16 str16Key(strKey.GetStr());
    String16 str16Value(strValue.GetStr());

    in.writeString16(str16FileName);
    in.writeString16(str16Key);
    in.writeString16(str16Value);
    in.writeInt32(nSlotId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
AString System::GetPrivateProperty(
        IN IMS_BOOL bPersistent, IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return AString::ConstNull();
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::GET_PRIVATE_PROPERTY);

    in.writeInt32(bPersistent ? 1 : 0);
    in.writeString16(String16(strKey.GetStr()));
    in.writeInt32(nSlotId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16 = out.readString16();
        String8 str8(str16);

        return AString(str8.c_str());
    }

    return AString::ConstNull();
}

PUBLIC
IMS_SINT32 System::SetPrivateProperty(IN IMS_BOOL bPersistent, IN const AString& strKey,
        IN const AString& strValue, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::SET_PRIVATE_PROPERTY);

    in.writeInt32(bPersistent ? 1 : 0);
    in.writeString16(String16(strKey.GetStr()));
    in.writeString16(String16(strValue.GetStr()));
    in.writeInt32(nSlotId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return 1;
    }

    return 0;
}

PUBLIC
IMS_BOOL System::GetCarrierConfig(IN IMS_SINT32 nSlotId, OUT ImsParcel& objConfig)
{
    if (m_pCallback == IMS_NULL)
    {
        return IMS_FALSE;
    }

    OsParcel& objOsConfig = DYNAMIC_CAST(OsParcel&, objConfig);
    android::Parcel in;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_CARRIER_CONFIG);

    if (m_pCallback->SendDataToJava(in, objOsConfig.GetParcel()) == 1)
    {
        IMS_SINT32 nStatus = objOsConfig.GetParcel().readInt32();

        return nStatus == 1;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 System::SendEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::SEND_EVENT);

    in.writeInt32(nEvent);
    in.writeInt32(nWParam);
    in.writeInt32(nLParam);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::SetEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId)
{
    return GetInt2(SystemConstants::SET_EVENT, nEvent, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::ResetEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId)
{
    return GetInt2(SystemConstants::RESET_EVENT, nEvent, 0, nSlotId);
}

PUBLIC
IMS_BOOL System::IsWifiCallingEnabled(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_WFC_ENABLED, 0, nSlotId) == 1);
}

PUBLIC
IMS_SINT32 System::GetWifiCallingPreferences(IN IMS_SINT32 nSlotId)
{
    return GetInt(SystemConstants::GET_WFC_PREFERENCES, 0, nSlotId);
}

PUBLIC
IMS_BOOL System::IsWifiCallingProvisioned(IN IMS_SINT32 nSlotId)
{
    return (GetInt(SystemConstants::IS_WFC_PROVISIONED, 0, nSlotId) == 1);
}

PUBLIC
AString System::GetWifiCallingAddressId(IN IMS_SINT32 nSlotId)
{
    AString strAid(AString::ConstNull());

    GetString(SystemConstants::GET_WFC_ADDRESS_ID, strAid, nSlotId);

    return strAid;
}

PUBLIC
IMS_BOOL System::StartListeningForLocation(
        IN IMS_UINT32 nUpdateIntervalInSec, IN IMS_SINT32 nSlotId)
{
    return GetInt2(SystemConstants::START_LISTENING_FOR_LOCATION, nUpdateIntervalInSec, 0,
                   nSlotId) == 1;
}

PUBLIC
void System::StopListeningForLocation(IN IMS_SINT32 nSlotId)
{
    (void)GetInt(SystemConstants::STOP_LISTENING_FOR_LOCATION, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::GetLastKnownLocation(
        OUT AStringArray& objLocationInfo, IN IMS_SINT32 nType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::GET_LAST_KNOWN_LOCATION);
    in.writeInt32(nType);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        IMS_SINT32 nCount = out.readInt32();

        if (nCount == 0)
        {
            return 0;
        }

        for (IMS_SINT32 i = 0; i < nCount; i++)
        {
            String16 str16Address = out.readString16();
            String8 str8Address(str16Address);
            AString strLocationParam(str8Address.c_str());
            objLocationInfo.AddElement(strLocationParam);
        }
        return 1;
    }

    return 0;
}

PUBLIC
IMS_SINT32 System::RequestLocationUpdate(IN IMS_SINT32 nWaitTimeMs, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::REQUEST_LOCATION_UPDATE);
    in.writeInt32(nWaitTimeMs);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
void System::CancelLocationUpdate(IN IMS_SINT32 nId, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::CANCEL_LOCATION_UPDATE);
    in.writeInt32(nId);

    m_pCallback->SendDataToJava(in, out);
}

PUBLIC
IMS_SINT32 System::StartImsTraffic(IN IMS_UINT32 nId, IN IMS_UINT32 nTrafficType,
        IN IMS_UINT32 nAccessNetworkType, IN IMS_UINT32 nDirection, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return -1;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::START_IMS_TRAFFIC);
    in.writeInt32(nId);
    in.writeInt32(nTrafficType);
    in.writeInt32(nAccessNetworkType);
    in.writeInt32(nDirection);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return -1;
}

PUBLIC
void System::StopImsTraffic(IN IMS_UINT32 nId, IN IMS_SINT32 nSlotId)
{
    GetInt2(SystemConstants::STOP_IMS_TRAFFIC, nId, 0, nSlotId);
}

PUBLIC
IMS_SINT32 System::TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason, IN IMS_SINT32 nSlotId)
{
    return (GetInt2(SystemConstants::TRIGGER_EPS_FALLBACK, nEpsfbReason, -1, nSlotId));
}

PUBLIC
IMS_SINT32 System::SetTrafficPriority(IN IMS_UINT32 nPriorityType, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return -1;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(IMS_SLOT_0);
    in.writeInt32(SystemConstants::SET_TRAFFIC_PRIORITY);
    in.writeInt32(nPriorityType);
    in.writeInt32(nSlotId);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return -1;
}

PRIVATE
IMS_SINT32 System::GetInt(IN IMS_SINT32 nOperation, IN IMS_SINT32 nDefaultValue /*= 0*/,
        IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    if (m_pCallback == IMS_NULL)
    {
        return nDefaultValue;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(nOperation);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return nDefaultValue;
}

PRIVATE
IMS_SINT32 System::GetInt2(IN IMS_SINT32 nOperation, IN IMS_SINT32 nParam,
        IN IMS_SINT32 nDefaultValue /*= 0*/, IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    if (m_pCallback == IMS_NULL)
    {
        return nDefaultValue;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(nOperation);
    in.writeInt32(nParam);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        return out.readInt32();
    }

    return nDefaultValue;
}

PRIVATE
IMS_SINT32 System::GetString(
        IN IMS_SINT32 nOperation, OUT AString& strValue, IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(nOperation);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        String16 str16 = out.readString16();
        String8 str8(str16);
        strValue = AString(str8.c_str());
        return 1;
    }

    return 0;
}

PRIVATE
IMS_SINT32 System::RequestSimAuthentication(IN IMS_SINT32 nOperation, IN const AString& strNonce,
        IN IMS_SINTP nOwner, IN IMS_SINT32 nSlotId)
{
    IMS_SINT32 nResult = IMS_FAILURE;

    if (m_pCallback == IMS_NULL)
    {
        return nResult;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(nOperation);
    in.writeString16(android::String16(strNonce.GetStr()));
    in.writeInt64(nOwner);

    if (m_pCallback->SendDataToJava(in, out) == 1)
    {
        nResult = IMS_SUCCESS;
    }

    return nResult;
}

PUBLIC
IMS_SINT32 System::AddIpSecSaParameter(IN const IpSecSaParameter& objSaParam, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::ADD_IPSEC_SA_PARAMETER);

    in.writeInt32(objSaParam.GetIpSecId());
    in.writeInt32(objSaParam.GetSecurityProtocol());

    in.writeInt32(objSaParam.GetIntegrityAlgorithm());
    const ByteArray& objIk = objSaParam.GetIk();
    in.writeInt32(objIk.GetLength());
    in.writeByteArray(objIk.GetLength(), static_cast<const uint8_t*>(objIk.GetData()));

    in.writeInt32(objSaParam.GetEncryptionAlgorithm());
    const ByteArray& objCk = objSaParam.GetCk();
    in.writeInt32(objCk.GetLength());
    in.writeByteArray(objCk.GetLength(), static_cast<const uint8_t*>(objCk.GetData()));

    const ImsList<IpSecSaParameter::Policy>& objPolicys = objSaParam.GetPolicys();

    in.writeInt32(objPolicys.GetSize());

    for (IMS_SINT32 i = 0; i < objPolicys.GetSize(); i++)
    {
        const IpSecSaParameter::Policy& objPolicy = objPolicys.GetAt(i);

        in.writeInt32(objPolicy.GetSpi());
        in.writeInt32(objPolicy.GetDirection());
        in.writeInt32(objPolicy.GetMode());

        const SocketAddress& objLocalAddr = objPolicy.GetLocalAddress();
        String16 strLocalIpAddress(objLocalAddr.GetAddress().ToString().GetStr());
        in.writeString16(strLocalIpAddress);
        // in.writeInt32(objLocalAddr.GetPort());

        const SocketAddress& objRemoteAddr = objPolicy.GetRemoteAddress();
        String16 strRemoteIpAddress(objRemoteAddr.GetAddress().ToString().GetStr());
        in.writeString16(strRemoteIpAddress);
        // in.writeInt32(objRemoteAddr.GetPort());
    }

    if (m_pCallback->SendDataToJava(in, out) != 0)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
void System::RemoveIpSecSaParameter(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::REMOVE_IPSEC_SA_PARAMETER);

    in.writeInt32(nIpSecId);

    if (m_pCallback->SendDataToJava(in, out) != 0)
    {
        return;
    }
}

PUBLIC
IMS_SINT32 System::ApplyIpSecSa(
        IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return 0;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::APPLY_IPSEC_SA);

    in.writeInt32(nIpSecId);
    in.writeInt32(nSpi);
    in.writeInt32(nSocketFd);

    if (m_pCallback->SendDataToJava(in, out, nSocketFd) != 0)
    {
        return out.readInt32();
    }

    return 0;
}

PUBLIC
void System::RemoveIpSecSa(
        IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSpi, IN IMS_SINT32 nSocketFd, IN IMS_SINT32 nSlotId)
{
    if (m_pCallback == IMS_NULL)
    {
        return;
    }

    android::Parcel in;
    android::Parcel out;

    in.writeInt32(nSlotId);
    in.writeInt32(SystemConstants::REMOVE_IPSEC_SA);

    in.writeInt32(nIpSecId);
    in.writeInt32(nSpi);
    in.writeInt32(nSocketFd);

    if (m_pCallback->SendDataToJava(in, out, nSocketFd) != 0)
    {
        return;
    }
}

PRIVATE
void System::AddListenerIfCategoryMatched(IN IMS_UINT32 nCategories, IN IMS_UINT32 nCategory,
        IN ISystemListener* piListener, IN IMS_SINT32 nSlotId)
{
    if ((nCategories & nCategory) == nCategory)
    {
        SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
        pHolder->AddListener(nCategory, piListener);
    }
}

PRIVATE
void System::RemoveListenerIfCategoryMatched(IN IMS_UINT32 nCategories, IN IMS_UINT32 nCategory,
        IN ISystemListener* piListener, IN IMS_SINT32 nSlotId)
{
    if ((nCategories & nCategory) == nCategory)
    {
        SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
        pHolder->RemoveListener(nCategory, piListener);
    }
}

PRIVATE
void System::NotifyNetworkCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in)
{
    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
    ImsList<ISystemListener*>* pListeners =
            pHolder->GetListeners(SystemConstants::CATEGORY_NETWORK);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nEvent = IMS_SYSTEM_INVALID;
    IMS_UINTP nWParam = 0;
    IMS_UINTP nLParam = 0;

    if (nCmd == SystemConstants::NOTIFY_SERVICE_STATE_CHANGED)
    {
        nEvent = IMS_SYSTEM_SERVICE_STATE_CHANGED;
        nWParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_DATA_CONNECTION_STATE_CHANGED)
    {
        nEvent = IMS_SYSTEM_DATACONNECTION_STATE_CHANGED;
        nWParam = in.readInt32();
        nLParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_DATA_CONNECTION_IPCAN_CHANGED)
    {
        nEvent = IMS_SYSTEM_DATACONNECTION_IPCAN_CHANGED;
        nWParam = in.readInt32();
        nLParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_AIRPLANE_MODE_CHANGED)
    {
        nEvent = IMS_SYSTEM_AIRPLANE_MODE_CHANGED;
        nWParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_NETWORK_TYPE_CHANGED)
    {
        nEvent = IMS_SYSTEM_RADIOTECH_STATE_CHANGED;
        nWParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_VOICE_NETWORK_TYPE_CHANGED)
    {
        nEvent = IMS_SYSTEM_VOICE_RADIOTECH_STATE_CHANGED;
        nWParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_DATA_CONNECTION_FAILED)
    {
        nEvent = IMS_SYSTEM_DATACONNECTION_FAILED;
        nWParam = in.readInt32();
    }
    else
    {
        IMS_TRACE_D("CATEGORY_NETWORK :: Cmd (%u) is not handled", nCmd, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->System_NotifyEvent(nEvent, nWParam, nLParam);
        }
    }
}

PRIVATE
void System::NotifyWifiCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in)
{
    (void)nSlotId;

    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(IMS_SLOT_0);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(SystemConstants::CATEGORY_WIFI);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nEvent = IMS_SYSTEM_INVALID;
    IMS_UINTP nWParam = 0;
    IMS_UINTP nLParam = 0;

    if (nCmd == SystemConstants::NOTIFY_WIFI_STATE_CHANGED)
    {
        nEvent = IMS_SYSTEM_WIFI_STATE_CHANGED;
        nWParam = in.readInt32();
    }
    else if (nCmd == SystemConstants::NOTIFY_WIFI_CONNECTION_STATE_CHANGED)
    {
        nEvent = IMS_SYSTEM_WIFI_CONNECTION_STATE_CHANGED;
        nWParam = in.readInt32();
    }
    else
    {
        IMS_TRACE_D("CATEGORY_WIFI :: Cmd (%u) is not handled", nCmd, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->System_NotifyEvent(nEvent, nWParam, nLParam);
        }
    }
}

PRIVATE
void System::NotifyPowerCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in)
{
    (void)nSlotId;

    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(IMS_SLOT_0);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(SystemConstants::CATEGORY_POWER);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nEvent = IMS_SYSTEM_INVALID;
    IMS_UINTP nWParam = 0;
    IMS_UINTP nLParam = 0;

    if (nCmd == SystemConstants::NOTIFY_BATTERY_LEVEL_CHANGED)
    {
        nEvent = IMS_SYSTEM_BATTERY_CHANGED;
        nWParam = in.readInt32();
    }
    else
    {
        IMS_TRACE_D("CATEGORY_POWER :: Cmd (%u) is not handled", nCmd, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->System_NotifyEvent(nEvent, nWParam, nLParam);
        }
    }
}

PRIVATE
void System::NotifyTimerCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in)
{
    (void)nSlotId;

    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(IMS_SLOT_0);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(SystemConstants::CATEGORY_TIMER);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nEvent = IMS_SYSTEM_INVALID;
    IMS_UINTP nWParam = 0;
    IMS_UINTP nLParam = 0;

    if (nCmd == SystemConstants::NOTIFY_TIMER_EXPIRED)
    {
        nEvent = IMS_SYSTEM_TIMER_EXPIRED;
        nWParam = INT64_TO_UINTP(in.readInt64());
    }
    else
    {
        IMS_TRACE_D("CATEGORY_TIMER ::  Cmd (%u) is not handled", nCmd, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->System_NotifyEvent(nEvent, nWParam, nLParam);
        }
    }
}

PRIVATE
void System::NotifyConfigCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in)
{
    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(SystemConstants::CATEGORY_CONFIG);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nEvent = IMS_SYSTEM_INVALID;
    IMS_UINTP nWParam = 0;
    IMS_UINTP nLParam = 0;

    if (nCmd == SystemConstants::NOTIFY_CONFIGURATION_CHANGED)
    {
        nEvent = IMS_SYSTEM_CONFIGURATION_CHANGED;
        nWParam = in.readInt32();
    }
    else
    {
        IMS_TRACE_D("CATEGORY_CONFIG :: Cmd (%u) is not handled", nCmd, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->System_NotifyEvent(nEvent, nWParam, nLParam);
        }
    }
}

PRIVATE
void System::NotifyEventCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nCmd, IN const android::Parcel& in)
{
    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(SystemConstants::CATEGORY_EVENT);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nEvent = IMS_SYSTEM_INVALID;
    IMS_UINTP nWParam = 0;
    IMS_UINTP nLParam = 0;

    if (nCmd == SystemConstants::NOTIFY_EVENT)
    {
        nEvent = in.readInt32();
        nWParam = in.readInt32();
        nLParam = in.readInt32();
    }
    else
    {
        IMS_TRACE_D("CATEGORY_EVENT :: Cmd (%u) is not handled", nCmd, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->System_NotifyEvent(nEvent, nWParam, nLParam);
        }
    }
}

PRIVATE
void System::NotifySimCategory(IN IMS_SINT32 nSlotId, IN IMS_UINT32 /*nCmd*/,
        IN IMS_UINT32 nCategory, IN const android::Parcel& in)
{
    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(nCategory);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    android::Parcel& objParcel = const_cast<android::Parcel&>(in);

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nEvent = objParcel.readInt32();

        piListener->System_NotifyEvent(nEvent, 0, reinterpret_cast<IMS_UINTP>(&objParcel));

        objParcel.setDataPosition(0);

        // Consume a slot id
        objParcel.readInt32();
        // Consume a command
        objParcel.readInt32();
    }
}

PRIVATE
void System::NotifyRadioCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 /*nCmd*/, IN const android::Parcel& in)
{
    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
    ImsList<ISystemListener*>* pListeners = pHolder->GetListeners(SystemConstants::CATEGORY_RADIO);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    android::Parcel& objParcel = const_cast<android::Parcel&>(in);

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nEvent = objParcel.readInt32();

        piListener->System_NotifyEvent(nEvent, 0, reinterpret_cast<IMS_UINTP>(&objParcel));

        objParcel.setDataPosition(0);

        // Consume a slot id
        objParcel.readInt32();
        // Consume a command
        objParcel.readInt32();
    }
}

PRIVATE
void System::NotifyLocationCategory(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 /*nCmd*/, IN const android::Parcel& in)
{
    SystemListenerHolder* pHolder = m_pSystemP->GetListenerHolder(nSlotId);
    ImsList<ISystemListener*>* pListeners =
            pHolder->GetListeners(SystemConstants::CATEGORY_LOCATION);

    if (pListeners == IMS_NULL)
    {
        return;
    }

    android::Parcel& objParcel = const_cast<android::Parcel&>(in);

    for (IMS_UINT32 i = 0; i < pListeners->GetSize(); ++i)
    {
        ISystemListener* piListener = pListeners->GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nEvent = objParcel.readInt32();

        piListener->System_NotifyEvent(nEvent, 0, reinterpret_cast<IMS_UINTP>(&objParcel));

        objParcel.setDataPosition(0);

        // Consume a slot id
        objParcel.readInt32();
        // Consume a command
        objParcel.readInt32();
    }
}
