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
#ifndef INTERFACE_PHONE_INFO_POWER_H_
#define INTERFACE_PHONE_INFO_POWER_H_

#include "ImsList.h"
#include "ImsMessageDef.h"
#include "ServiceMessage.h"
#include "ServiceThread.h"

enum POWERLEVEL_ENTYPE
{
    POWERLEVEL_LOW = 0,
    POWERLEVEL_HIGH,

    POWERLEVEL_OFF,
    POWERLEVEL_VALUE
};

class IPowerInfo;

class IPowerInfoListener
{
public:
    virtual void PowerInfo_NotifyPowerLevel(IN IPowerInfo* piPowerInfo) = 0;
};

class IPowerInfo
{
public:
    virtual POWERLEVEL_ENTYPE GetPowerLevel() = 0;
    virtual IMS_UINT32 GetPowerValue() = 0;

public:
    inline void RegisterObserver(IN IPowerInfoListener* piListener)
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

    inline void RemoveObserver(IN IPowerInfoListener* piListener)
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
                    IPowerInfoListener* piTmpListener = pObserverList->m_objListeners.GetAt(j);

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

    inline void PostMsgRegisteredThread(IN IMS_UINT32 nEvent)
    {
        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); ++i)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(
                    pObserverList->m_piOwnerThread, IMS_MSG_BATTERY, 0, nEvent);
        }
    }

    inline void PostMsgRegisteredThread(IN POWERLEVEL_ENTYPE enPowerLevel)
    {
        for (IMS_UINT32 i = 0; i < m_objObserverLists.GetSize(); ++i)
        {
            ObserverList* pObserverList = m_objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(
                    pObserverList->m_piOwnerThread, IMS_MSG_BATTERY, 0, enPowerLevel);
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
                    IPowerInfoListener* piListener = pObserverList->m_objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->PowerInfo_NotifyPowerLevel(this);
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
        inline ObserverList(IN IPowerInfoListener* piListener)
        {
            m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
            m_objListeners.Append(piListener);
        }

        inline IMS_BOOL operator==(IN IThread* piThread) { return piThread == m_piOwnerThread; }

    public:
        IThread* m_piOwnerThread;
        IMSList<IPowerInfoListener*> m_objListeners;
    };

private:
    IMSList<ObserverList*> m_objObserverLists;
};

#endif
