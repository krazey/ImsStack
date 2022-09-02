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
#include "ImsMessageDef.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "device/OsImsRadio.h"

__IMS_TRACE_TAG_ADAPT__;

class OsImsRadioParam
{
public:
    inline OsImsRadioParam(IN IMS_UINT32 nEvent) :
            m_nEvent(nEvent)
    {
    }
    inline virtual ~OsImsRadioParam() {}

public:
    enum
    {
        EVENT_CONNECTION_FAILED = 1,
        EVENT_CONNECTION_SETUP_PREPARED = 2
    };

    IMS_UINT32 m_nEvent;
};

class ConnectionSetupPreparedParam : public OsImsRadioParam
{
public:
    inline ConnectionSetupPreparedParam() :
            OsImsRadioParam(EVENT_CONNECTION_SETUP_PREPARED),
            m_nImsTrafficType(0)
    {
    }
    inline virtual ~ConnectionSetupPreparedParam() {}

    IMS_UINT32 m_nImsTrafficType;
};

LOCAL
void osImsRadio_SendMessage(IN IThread* piThread, IN IMS_SINT32 nSlotId, IN OsImsRadioParam* pParam)
{
    if (piThread == IMS_NULL)
    {
        delete pParam;
        return;
    }

    if (!piThread->PostMessageI(IMS_MSG_RADIO, nSlotId, reinterpret_cast<IMS_UINTP>(pParam)))
    {
        delete pParam;
    }
}

PUBLIC
OsImsRadio::OsImsRadio(IN IMS_SINT32 nSlotId) :
        ImsRadio(nSlotId),
        m_piOwnerThread(IMS_NULL),
        m_objConnectionListeners(ImsMap<IMS_UINT32, IImsRadioConnectionListener*>()),
        m_objTrafficPriorityListeners(ImsList<IImsRadioTrafficPriorityListener*>())
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
}

PUBLIC VIRTUAL OsImsRadio::~OsImsRadio() {}

PUBLIC VIRTUAL IMS_BOOL OsImsRadio::IsImsTrafficAllowed(IN IMS_UINT32 /* nTrafficType */)
{
    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsImsRadio::StartImsTraffic(IN IMS_UINT32 nTrafficType,
        IN IMS_UINT32 /* nAccessNetworkType */, IN IImsRadioConnectionListener* piListener)
{
    if (piListener != IMS_NULL)
    {
        IMS_SLONG nIndex = m_objConnectionListeners.GetIndexOfKey(nTrafficType);

        if (nIndex < 0)
        {
            m_objConnectionListeners.Add(nTrafficType, piListener);
        }
        else
        {
            m_objConnectionListeners.SetValueAt(nTrafficType, piListener);
        }
    }

    ConnectionSetupPreparedParam* pParam = new ConnectionSetupPreparedParam();
    pParam->m_nImsTrafficType = nTrafficType;

    osImsRadio_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
}

PUBLIC VIRTUAL void OsImsRadio::StopImsTraffic(IN IMS_UINT32 /* nTrafficType */) {}

PUBLIC VIRTUAL void OsImsRadio::TriggerEpsFallback(IN IMS_UINT32 /* nEpsfbReason */) {}

PUBLIC VIRTUAL void OsImsRadio::AddListenerForTrafficPriority(
        IN IImsRadioTrafficPriorityListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objTrafficPriorityListeners.GetSize(); ++i)
    {
        IImsRadioTrafficPriorityListener* pTmpListener = m_objTrafficPriorityListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            return;
        }
    }

    m_objTrafficPriorityListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsImsRadio::RemoveListenerForTrafficPriority(
        IN IImsRadioTrafficPriorityListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objTrafficPriorityListeners.GetSize(); ++i)
    {
        IImsRadioTrafficPriorityListener* pTmpListener = m_objTrafficPriorityListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objTrafficPriorityListeners.RemoveAt(i);
            return;
        }
    }
}

PROTECTED VIRTUAL void OsImsRadio::DispatchServiceMessage(IN IMS_UINTP /* nWparam */
        ,
        IN IMS_UINTP nLparam)
{
    OsImsRadioParam* pImsRadioParam = reinterpret_cast<OsImsRadioParam*>(nLparam);

    if (pImsRadioParam != IMS_NULL)
    {
        IMS_TRACE_D("OsImsRadio :: DispatchServiceMessage - slotId=%d, event=%s, lp=%" PFLS_u,
                GetSlotId(), EventToString(pImsRadioParam->m_nEvent), nLparam);

        if (pImsRadioParam->m_nEvent == OsImsRadioParam::EVENT_CONNECTION_SETUP_PREPARED)
        {
            ConnectionSetupPreparedParam* pParam =
                    reinterpret_cast<ConnectionSetupPreparedParam*>(pImsRadioParam);

            NotifyConnectionSetupPrepared(pParam->m_nImsTrafficType);
        }

        delete pImsRadioParam;
    }
}

PRIVATE void OsImsRadio::NotifyConnectionSetupPrepared(IN IMS_UINT32 nImsTrafficType)
{
    IMS_SLONG nIndex = m_objConnectionListeners.GetIndexOfKey(nImsTrafficType);

    if (nIndex >= 0)
    {
        IImsRadioConnectionListener* piListener = m_objConnectionListeners.GetValueAt(nIndex);
        if (piListener != IMS_NULL)
        {
            piListener->ImsRadio_OnConnectionSetupPrepared();
        }
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsImsRadio::EventToString(IN IMS_UINT32 nEvent)
{
    switch (nEvent)
    {
        case OsImsRadioParam::EVENT_CONNECTION_FAILED:
            return "EVENT_CONNECTION_FAILED";

        case OsImsRadioParam::EVENT_CONNECTION_SETUP_PREPARED:
            return "EVENT_CONNECTION_SETUP_PREPARED";

        default:
            return "__INVALID__";
    }
}