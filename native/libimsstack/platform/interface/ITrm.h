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
#ifndef INTERFACE_TRM_H_
#define INTERFACE_TRM_H_

#include "ImsTypeDef.h"
#include "IMSList.h"
#include "ImsMessageDef.h"

#include "ServiceThread.h"
#include "ServiceMessage.h"

class ITrmListener
{
public:
    /*
        Notifies the application that the service priority is changed.
    */
    virtual void Trm_NotifyServicePriorityChanged() = 0;
};

class TrmStateParam
{
public:
    IMS_SINT32 nSlotId;
    IMS_UINT32 nServiceType;
    IMS_UINT32 nMode;
};

class ITrm
{
public:
    enum
    {
        SERVICE_NONE            = 0,
        SERVICE_UT              = (0x0001),
        SERVICE_REG             = (0x0002),
        SERVICE_SMS             = (0x0004),
        SERVICE_VOLTE           = (0x0008)
    };

    enum
    {
        MODE_END = 0,
        MODE_START = 1
    };

    enum
    {
        SERVICE_CHANGED = 100
    };

    /*
        Enable TRM
    */
    virtual void Enable(IN IMS_UINT32 nSlotId) = 0;

    /*
        Disable TRM
    */
    virtual void Disable(IN IMS_UINT32 nSlotId) = 0;

    /*
        Check if service is available or not
    */
    virtual IMS_BOOL IsServiceAvailable(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType) = 0;

    /*
        Check if TRM is supported or not
    */
    virtual IMS_BOOL IsTrmSupported() = 0;

    /*
        Set the emergency service start or end state
    */
    virtual void SetEmergencyService(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType,
        IN IMS_UINT32 nMode) = 0;

    /*
        Set the IPCAN category (CATEGORY_MOBILE, CATEGORY_WLAN)
    */
    virtual void SetIpcan(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nCategory) = 0;

    /*
        Set the service start or end state
    */
    virtual IMS_BOOL SetService(IN IMS_UINT32 nSlotId,
            IN IMS_UINT32 nType, IN IMS_UINT32 nMode) = 0;

public:
    /*
        Register the listener for observing the call priority
    */
    inline void RegisterObserver(IN ITrmListener* piListener)
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

    /*
        Remove the listener for observing the call priority
    */
    inline void RemoveObserver(IN ITrmListener* piListener)
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
                    ITrmListener* objListener = pObserverList->objListeners.GetAt(j);

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

public:
    inline void ProcessNotify(IN ImsMessage& objMSG)
    {
        IThread *piThread = ThreadService::GetThreadService()->GetCurrentThread();

        TrmStateParam* pParam = IMS_NULL;
        if ((IMS_SINT32)objMSG.nWparam == SERVICE_CHANGED)
        {
            pParam = (TrmStateParam*)objMSG.nLparam;
        }

        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (*pObserverList == piThread)
            {
                if ((IMS_SINT32)objMSG.nWparam == SERVICE_CHANGED)
                {
                    if (pParam != IMS_NULL)
                    {
                        SetService(pParam->nSlotId, pParam->nServiceType, pParam->nMode);
                    }
                    break;
                }

                for (IMS_UINT32 j = 0; j < pObserverList->objListeners.GetSize(); ++j)
                {
                    ITrmListener* piListener = pObserverList->objListeners.GetAt(j);

                    if (piListener != IMS_NULL)
                    {
                        piListener->Trm_NotifyServicePriorityChanged();
                    }
                }
                break;
            }
        }

        if (pParam != IMS_NULL)
        {
            delete pParam;
        }
    }

protected:
    inline void PostMsgRegisteredThread()
    {
        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            IMS_MSG_CreateNPostThreadMessage(pObserverList->piOwnerThread,
                    IMS_MSG_TRM_PRIORITY_STATUS, 0, 0);
        }
    }

    inline void PostMsgEnablerThread(IN TrmStateParam* pParam)
    {
        for (IMS_UINT32 i = 0; i < objObserverLists.GetSize(); ++i)
        {
            ObserverList *pObserverList = objObserverLists.GetAt(i);

            if (pObserverList == IMS_NULL)
            {
                continue;
            }

            if (pObserverList->piOwnerThread != IMS_NULL)
            {
                if (pObserverList->piOwnerThread->GetSlotId() == pParam->nSlotId)
                {
                    IMS_MSG_CreateNPostThreadMessage(pObserverList->piOwnerThread,
                        IMS_MSG_TRM_PRIORITY_STATUS, SERVICE_CHANGED, pParam);
                    return;
                }

            }
        }

        if (pParam != IMS_NULL)
        {
            delete pParam;
        }
    }

private:
    class ObserverList
    {
        public:
            inline ObserverList(IN ITrmListener* piListener)
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
            IMSList<ITrmListener*> objListeners;
    };

private:
    IMSList<ObserverList*> objObserverLists;
};

#endif // INTERFACE_TRM_H_
