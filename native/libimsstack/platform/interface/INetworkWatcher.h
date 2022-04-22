/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_NET_WATCHER_H_
#define _INTERFACE_IMS_NET_WATCHER_H_

#include "ImsTypeDef.h"
#include "IMSList.h"
#include "ImsMessageDef.h"

#include "ServiceThread.h"
#include "ServiceMessage.h"
#include "ServiceNetworkPolicy.h"

class INetWatcherListener
{
public:
    virtual void NotifyNetWatcherStatus(IN class INetWatcherInfo *piNetWatcherInfo) = 0;
};

typedef enum
{
    NW_REPORT_RADIO_INVALID     = (0x00000000),
    NW_REPORT_RADIO_NOSRV       = (0x00010000),
    NW_REPORT_RADIO_AMPS        = (0x00020000),
    NW_REPORT_RADIO_CDMA        = (0x00040000),
    NW_REPORT_RADIO_GSM         = (0x00080000),
    NW_REPORT_RADIO_HDR         = (0x00100000),
    NW_REPORT_RADIO_WCDMA       = (0x00200000),
    NW_REPORT_RADIO_GPS         = (0x00400000),
    NW_REPORT_RADIO_EDGE        = (0x00800000),
    NW_REPORT_RADIO_WLAN        = (0x01000000),
    NW_REPORT_RADIO_EVDODO      = (0x04000000),
    NW_REPORT_RADIO_EHRPD       = (0x10000000),
    NW_REPORT_RADIO_LTE         = (0x20000000),
    NW_REPORT_RADIO_HSPA        = (0x40000000),
    NW_REPORT_RADIO_NR          = (0x80000000),
    NW_REPORT_RADIO_MAX
} NETRADIO_ENTYPE;

typedef enum
{
    NW_REPORT_SRV_INVALID           = -1,
    NW_REPORT_SRV_NOSRV             = (0x00000001),
    NW_REPORT_SRV_LIMITED           = (0x00000002),
    NW_REPORT_SRV_SRV               = (0x00000004),
    NW_REPORT_SRV_LIMITEDREGION     = (0x00000008),
    NW_REPORT_SRV_PWRSAVE           = (0x00000010),
    NW_REPORT_SRV_MAX
} NETSERVICE_ENTYPE;

typedef enum
{
    NW_REPORT_DOMAIN_INVALID        = -1,
    NW_REPORT_DOMAIN_NOSRV          = (0x00000100),
    NW_REPORT_DOMAIN_CS             = (0x00000200),
    NW_REPORT_DOMAIN_PS             = (0x00000400),
    NW_REPORT_DOMAIN_CSPS           = (0x00000800),
    NW_REPORT_DOMAIN_CAMPED         = (0x00001000),
    NW_REPORT_DOMAIN_MAX
} NETDOMAIN_ENTYPE;

class INetWatcherInfo
{
public:
    // Same as OsNetworkConstants.h
    enum
    {
        RADIOTECH_TYPE_INVALID        = -1,
        RADIOTECH_TYPE_UNKNOWN        = 0,
        RADIOTECH_TYPE_GPRS           = 1,
        RADIOTECH_TYPE_EDGE           = 2,
        RADIOTECH_TYPE_UMTS           = 3,
        RADIOTECH_TYPE_CDMA           = 4, // Not used
        RADIOTECH_TYPE_EVDO_0         = 5, // Not used
        RADIOTECH_TYPE_EVDO_A         = 6, // Not used
        RADIOTECH_TYPE_1xRTT          = 7, // Not used
        RADIOTECH_TYPE_HSDPA          = 8,
        RADIOTECH_TYPE_HSUPA          = 9,
        RADIOTECH_TYPE_HSPA           = 10,
        RADIOTECH_TYPE_IDEN           = 11,
        RADIOTECH_TYPE_EVDO_B         = 12, // Not used
        RADIOTECH_TYPE_LTE            = 13,
        RADIOTECH_TYPE_EHRPD          = 14,
        RADIOTECH_TYPE_HSPAP          = 15,
        RADIOTECH_TYPE_GSM            = 16,
        RADIOTECH_TYPE_TD_SCDMA       = 17,
        RADIOTECH_TYPE_IWLAN          = 18,
        RADIOTECH_TYPE_LTE_CA         = 19,
        RADIOTECH_TYPE_NR             = 20,
        RADIOTECH_TYPE_MAX,
    };

public:
    virtual IMS_UINT32 GetNetworkStatus(IN const AString& strProfile) = 0;
    virtual NETRADIO_ENTYPE GetNetRadioTechType(IN const AString& strProfile,
            IN IMS_SINT32 nApnType = NetworkPolicy::APN_NONE) = 0;
    virtual NETRADIO_ENTYPE GetNetRadioTechType() = 0;
    virtual NETRADIO_ENTYPE GetNetVoiceRadioTechType() = 0;
    virtual NETSERVICE_ENTYPE GetNetServiceType(IN const AString& strProfile,
            IN IMS_SINT32 nApnType = NetworkPolicy::APN_NONE) = 0;
    virtual NETSERVICE_ENTYPE GetNetServiceType() = 0;
    virtual NETSERVICE_ENTYPE GetNetVoiceServiceType() = 0;
    virtual NETDOMAIN_ENTYPE GetNetDomainType() = 0;

    // 20120227 jungwpn82.kang@ - for direct access network type
    virtual IMS_SINT32 GetNetworkType()= 0;

    virtual IMS_SINT32 GetRoamingState() = 0;

    virtual IMS_SINT32 GetVoiceRoamingType() = 0;

    virtual IMS_SINT32 GetDataRoamingType() = 0;

    virtual IMS_BOOL IsImsEmergencyCallSupported() = 0;

    virtual IMS_BOOL IsImsVoiceCallSupported() = 0;

    virtual IMS_SINT32 GetLTERsrpStrength() = 0;

    virtual IMS_BOOL IsLteEmergencyOnly() = 0;

    virtual IMS_BOOL IsEmergencyAttachSupported() = 0;

    virtual IMS_SINT32 GetMocnPlmnInfo() = 0;

public:
    inline void RegisterObserver(IN INetWatcherListener *piListener)
    {
        IThread *piThread = ThreadService::GetThreadService()->GetCurrentThread();

        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); i++)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                pObserverList->objListeners.Append( piListener );
                return;
            }
        }

        objObserverLists.Append( new ObserverList( piListener ) );
    }

    inline void RemoveObserver(IN INetWatcherListener *piListener)
    {
        IThread *piThread = ThreadService::GetThreadService()->GetCurrentThread();

        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); i++)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                for (IMS_UINT32 j = 0; j < pObserverList->objListeners.GetSize(); j++)
                {
                    INetWatcherListener *objListener = pObserverList->objListeners.GetAt(j);

                    if (piListener == objListener)
                    {
                        pObserverList->objListeners.RemoveAt(j);
                        break;
                    }
                }
                break;
            }
        }
    }
    inline void PostMsgRegisteredThread(IN IMS_SINT32 nSlotId)
    {
        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(pObserverList->piOwnerThread,
                    IMS_MSG_NETWORK_STATUS, nSlotId, 0);
        }
    }

public:
    inline void ProcessNotify(IN IMSMSG &objMSG)
    {
        (void)objMSG;
        IThread *piThread = ThreadService::GetThreadService()->GetCurrentThread();

        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                for (IMS_UINT32 j = 0; j < pObserverList->objListeners.GetSize(); ++j)
                {
                    INetWatcherListener* piListener = pObserverList->objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->NotifyNetWatcherStatus(this);
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
            inline ObserverList(IN INetWatcherListener *piListener)
            {
                piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

                objListeners.Append(piListener);
            }

            inline IMS_BOOL operator==(IN IThread *piThread)
            {
                return piThread == piOwnerThread;
            }

        public:
            IThread *piOwnerThread;
            IMSList<INetWatcherListener*> objListeners;
    };

private:
    IMSList<ObserverList*> objObserverLists;
};

#endif // _INTERFACE_IMS_NET_WATCHER_H_
