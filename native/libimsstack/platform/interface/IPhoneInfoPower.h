/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_PHONE_INFO_POWER_H_
#define _INTERFACE_IMS_PHONE_INFO_POWER_H_

#include "ImsTypeDef.h"
#include "IMSList.h"
#include "ImsMessageDef.h"

#include "ServiceThread.h"
#include "ServiceMessage.h"

typedef enum
{
    POWERLEVEL_LOW = 0,
    POWERLEVEL_HIGH,

    POWERLEVEL_OFF,
    POWERLEVEL_VALUE
} POWERLEVEL_ENTYPE;

class IPowerInfoListener
{
public:
    virtual void NotifyPowerLevel(IN class IPowerInfo *piPowerInfo) = 0;

};

class IPowerInfo
{
public:
    virtual POWERLEVEL_ENTYPE GetPowerLevel() = 0;
    virtual IMS_UINT32 GetPowerValue() = 0;

public:
    inline void RegisterObserver(IN IPowerInfoListener *piListener)
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
                pObserverList->objListeners.Append(piListener);
                return;
            }
        }

        objObserverLists.Append(new ObserverList(piListener));
    }

    inline void RemoveObserver(IN IPowerInfoListener *piListener)
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
                    IPowerInfoListener *objListener = pObserverList->objListeners.GetAt(j);

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

    inline void PostMsgRegisteredThread(IN IMS_UINT32 eEvt)
    {
        (void)eEvt;

        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(
                    pObserverList->piOwnerThread, IMS_MSG_BATTERY, 0, eEvt);
        }
    }

    inline void PostMsgRegisteredThread(IN POWERLEVEL_ENTYPE eEvt)
    {
        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(
                    pObserverList->piOwnerThread, IMS_MSG_BATTERY, 0, eEvt);
        }
    }

public:
    inline void ProcessNotify(IN ImsMessage &objMSG)
    {
        IThread *piThread = ThreadService::GetThreadService()->GetCurrentThread();
        (void)objMSG;

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
                    IPowerInfoListener* piListener = pObserverList->objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->NotifyPowerLevel(this);
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
            inline ObserverList(IN IPowerInfoListener *piListener)
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
            IMSList<IPowerInfoListener*> objListeners;
    };

private:
    IMSList<ObserverList*> objObserverLists;
};

#endif // _INTERFACE_IMS_PHONE_INFO_POWER_H_
