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
#ifndef INTERFACE_NETWORK_WATCHER_H_
#define INTERFACE_NETWORK_WATCHER_H_

#include "ImsList.h"
#include "ImsMessageDef.h"
#include "ServiceMessage.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceThread.h"

class INetworkWatcher;

class INetworkWatcherListener
{
protected:
    virtual ~INetworkWatcherListener() = default;

public:
    virtual void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetworkWatcher) = 0;
};

typedef enum
{
    NW_REPORT_RADIO_INVALID = (0x00000000),
    NW_REPORT_RADIO_NOSRV = (0x00010000),
    NW_REPORT_RADIO_AMPS = (0x00020000),
    NW_REPORT_RADIO_CDMA = (0x00040000),
    NW_REPORT_RADIO_GSM = (0x00080000),
    NW_REPORT_RADIO_HDR = (0x00100000),
    NW_REPORT_RADIO_WCDMA = (0x00200000),
    NW_REPORT_RADIO_GPS = (0x00400000),
    NW_REPORT_RADIO_EDGE = (0x00800000),
    NW_REPORT_RADIO_WLAN = (0x01000000),
    NW_REPORT_RADIO_EVDODO = (0x04000000),
    NW_REPORT_RADIO_EHRPD = (0x10000000),
    NW_REPORT_RADIO_LTE = (0x20000000),
    NW_REPORT_RADIO_HSPA = (0x40000000),
    NW_REPORT_RADIO_NR = (0x80000000),
    NW_REPORT_RADIO_MAX
} NETRADIO_ENTYPE;

typedef enum
{
    NW_REPORT_SRV_NOSRV = (0x00000001),
    NW_REPORT_SRV_LIMITED = (0x00000002),
    NW_REPORT_SRV_SRV = (0x00000004),
    NW_REPORT_SRV_LIMITEDREGION = (0x00000008),
    NW_REPORT_SRV_PWRSAVE = (0x00000010),
    NW_REPORT_SRV_MAX
} NETSERVICE_ENTYPE;

typedef enum
{
    NW_REPORT_DOMAIN_NOSRV = (0x00000100),
    NW_REPORT_DOMAIN_CS = (0x00000200),
    NW_REPORT_DOMAIN_PS = (0x00000400),
    NW_REPORT_DOMAIN_CSPS = (0x00000800),
    NW_REPORT_DOMAIN_CAMPED = (0x00001000),
    NW_REPORT_DOMAIN_MAX
} NETDOMAIN_ENTYPE;

class INetworkWatcher
{
protected:
    virtual ~INetworkWatcher() = default;

public:
    // Same as OsNetworkConstants.h
    enum
    {
        RADIOTECH_TYPE_INVALID = -1,
        RADIOTECH_TYPE_UNKNOWN = 0,
        RADIOTECH_TYPE_GPRS = 1,
        RADIOTECH_TYPE_EDGE = 2,
        RADIOTECH_TYPE_UMTS = 3,
        RADIOTECH_TYPE_CDMA = 4,    // Not used
        RADIOTECH_TYPE_EVDO_0 = 5,  // Not used
        RADIOTECH_TYPE_EVDO_A = 6,  // Not used
        RADIOTECH_TYPE_1xRTT = 7,   // Not used
        RADIOTECH_TYPE_HSDPA = 8,
        RADIOTECH_TYPE_HSUPA = 9,
        RADIOTECH_TYPE_HSPA = 10,
        RADIOTECH_TYPE_IDEN = 11,
        RADIOTECH_TYPE_EVDO_B = 12,  // Not used
        RADIOTECH_TYPE_LTE = 13,
        RADIOTECH_TYPE_EHRPD = 14,
        RADIOTECH_TYPE_HSPAP = 15,
        RADIOTECH_TYPE_GSM = 16,
        RADIOTECH_TYPE_TD_SCDMA = 17,
        RADIOTECH_TYPE_IWLAN = 18,
        RADIOTECH_TYPE_LTE_CA = 19,
        RADIOTECH_TYPE_NR = 20,
        RADIOTECH_TYPE_MAX,
    };

public:
    virtual IMS_UINT32 GetNetworkStatus(IN const AString& strProfile) = 0;
    virtual NETRADIO_ENTYPE GetNetRadioTechType(
            IN const AString& strProfile, IN IMS_SINT32 nApnType = NetworkPolicy::APN_NONE) = 0;
    virtual NETRADIO_ENTYPE GetNetRadioTechType() = 0;
    virtual NETRADIO_ENTYPE GetNetVoiceRadioTechType() = 0;
    virtual NETSERVICE_ENTYPE GetNetServiceType(
            IN const AString& strProfile, IN IMS_SINT32 nApnType = NetworkPolicy::APN_NONE) = 0;
    virtual NETSERVICE_ENTYPE GetNetServiceType() = 0;
    virtual NETSERVICE_ENTYPE GetNetVoiceServiceType() = 0;
    virtual NETDOMAIN_ENTYPE GetNetDomainType() = 0;

    // 20120227 jungwpn82.kang@ - for direct access network type
    virtual IMS_SINT32 GetNetworkType() = 0;

    virtual IMS_SINT32 GetRoamingState() = 0;

    virtual IMS_SINT32 GetVoiceRoamingType() = 0;

    virtual IMS_SINT32 GetDataRoamingType() = 0;

    virtual IMS_BOOL IsImsEmergencyCallSupported() = 0;

    virtual IMS_BOOL IsImsVoiceCallSupported() = 0;

    virtual IMS_BOOL IsEmergencyOnly() = 0;

    virtual IMS_BOOL IsEmergencyAttachSupported() = 0;

    virtual IMS_SINT32 GetMocnPlmnInfo() = 0;

public:
    inline void RegisterObserver(IN INetworkWatcherListener* piListener)
    {
        IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); i++)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                pObserverList->m_objListeners.Append(piListener);
                return;
            }
        }

        m_objObserverLists.Append(new ObserverList(piListener));
    }

    inline void RemoveObserver(IN const INetworkWatcherListener* piListener)
    {
        IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); i++)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                for (IMS_UINT32 j = 0; j < pObserverList->m_objListeners.GetSize(); j++)
                {
                    INetworkWatcherListener* piTmpListener = pObserverList->m_objListeners.GetAt(j);

                    if (piListener == piTmpListener)
                    {
                        pObserverList->m_objListeners.RemoveAt(j);
                        break;
                    }
                }
                break;
            }
        }
    }
    inline void PostMsgRegisteredThread(IN IMS_SINT32 nSlotId)
    {
        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); ++i)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(
                    pObserverList->m_piOwnerThread, IMS_MSG_NETWORK_STATUS, nSlotId, 0);
        }
    }

public:
    inline void ProcessNotify(IN ImsMessage& /*objMsg*/)
    {
        IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); ++i)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                for (IMS_UINT32 j = 0; j < pObserverList->m_objListeners.GetSize(); ++j)
                {
                    INetworkWatcherListener* piListener = pObserverList->m_objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->NetworkWatcher_NotifyStatus(this);
                    }
                }
                break;
            }
        }
    }

private:
    class ObserverList
    {
    public:
        inline explicit ObserverList(IN INetworkWatcherListener* piListener)
        {
            m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
            m_objListeners.Append(piListener);
        }

        inline IMS_BOOL operator==(IN const IThread* piThread) const
        {
            return piThread == m_piOwnerThread;
        }

    public:
        IThread* m_piOwnerThread;
        ImsList<INetworkWatcherListener*> m_objListeners;
    };

private:
    ImsList<ObserverList*> m_objObserverLists;
};

#endif
