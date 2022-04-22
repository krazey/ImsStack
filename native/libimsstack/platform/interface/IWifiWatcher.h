/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110418  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_WIFI_WATCHER_H_
#define _INTERFACE_IMS_WIFI_WATCHER_H_

#include "ImsTypeDef.h"
#include "IMSList.h"
#include "ImsMessageDef.h"

#include "ServiceThread.h"
#include "ServiceMessage.h"

class IWifiWatcherListener
{
public:
    /*
     Notifies the application that the WiFi connection state is changed.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void NotifyStateChanged(IN class IWifiWatcher *pIWifiWatcher) = 0;
};

class IWifiWatcher
{
public:

    // State of WiFi connection
    enum
    {
        STATE_DISCONNECTED,
        STATE_CONNECTED,
    };

    /*
     Returns the state of WiFi connection.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              State of WiFi connection
    </table>
    */
    virtual IMS_SINT32 GetState() = 0;

public:
    inline void RegisterObserver(IN IWifiWatcherListener *piListener)
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

    inline void RemoveObserver(IN IWifiWatcherListener *piListener)
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
                    IWifiWatcherListener *objListener = pObserverList->objListeners.GetAt(j);

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

    inline void PostMsgRegisteredThread( )
    {
        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(pObserverList->piOwnerThread,
                    IMS_MSG_WIFI_STATUS, 0, 0);
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
                    IWifiWatcherListener* piListener = pObserverList->objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->NotifyStateChanged(this);
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
            inline ObserverList(IN IWifiWatcherListener *piListener)
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
            IMSList<IWifiWatcherListener*> objListeners;
    };

private:
    IMSList<ObserverList*> objObserverLists;
};

#endif // _INTERFACE_IMS_WIFI_WATCHER_H_
