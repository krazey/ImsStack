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
#ifndef INTERFACE_WIFI_WATCHER_H_
#define INTERFACE_WIFI_WATCHER_H_

#include "ImsList.h"
#include "ImsMessageDef.h"
#include "ServiceMessage.h"
#include "ServiceThread.h"

class IWifiWatcher;

class IWifiWatcherListener
{
public:
    /**
     * @brief Notifies the application that the Wi-Fi connection state is changed.
     *
     * @param piWifiWatcher The Wi-Fi watcher that notifies
     */
    virtual void WifiWatcher_NotifyStateChanged(IN IWifiWatcher* piWifiWatcher) = 0;
};

class IWifiWatcher
{
public:
    /// State of Wi-Fi connection
    enum
    {
        STATE_DISCONNECTED,
        STATE_CONNECTED,
    };

    /**
     * @brief Returns the state of Wi-Fi connection.
     *
     * @return The state of Wi-Fi connection.
     */
    virtual IMS_SINT32 GetState() = 0;

public:
    inline void RegisterObserver(IN IWifiWatcherListener* piListener)
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

    inline void RemoveObserver(IN IWifiWatcherListener* piListener)
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
                    IWifiWatcherListener* piTmpListener = pObserverList->m_objListeners.GetAt(j);

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

    inline void PostMsgRegisteredThread()
    {
        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); ++i)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(
                    pObserverList->m_piOwnerThread, IMS_MSG_WIFI_STATUS, 0, 0);
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
                    IWifiWatcherListener* piListener = pObserverList->m_objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->WifiWatcher_NotifyStateChanged(this);
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
        inline explicit ObserverList(IN IWifiWatcherListener* piListener)
        {
            m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
            m_objListeners.Append(piListener);
        }

        inline IMS_BOOL operator==(IN IThread* piThread) { return piThread == m_piOwnerThread; }

    public:
        IThread* m_piOwnerThread;
        IMSList<IWifiWatcherListener*> m_objListeners;
    };

private:
    IMSList<ObserverList*> m_objObserverLists;
};

#endif
