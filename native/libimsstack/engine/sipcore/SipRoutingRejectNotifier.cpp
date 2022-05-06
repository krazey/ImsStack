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
#include "ServiceMemory.h"

#include "ISipRoutingRejectListener.h"
#include "SipRoutingRejectNotifier.h"

PUBLIC
SipRoutingRejectNotifier::SipRoutingRejectNotifier() {}

PUBLIC VIRTUAL SipRoutingRejectNotifier::~SipRoutingRejectNotifier() {}

PUBLIC
IMS_BOOL SipRoutingRejectNotifier::IsNotificationRequired() const
{
    return !m_objListeners.IsEmpty();
}

PUBLIC
void SipRoutingRejectNotifier::NotifyRequestReject(
        IN ISipMessage* piSipMsg, IN_OUT SipStatusCode& objStatusCode)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISipRoutingRejectListener* piListener = m_objListeners.GetAt(i);

        if (piListener->RoutingReject_NotifyRequest(piSipMsg, objStatusCode))
        {
            return;
        }
    }
}

PUBLIC
void SipRoutingRejectNotifier::NotifyRequestReject(
        IN ISipServerConnection* piSsc, IN_OUT SipStatusCode& objStatusCode)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISipRoutingRejectListener* piListener = m_objListeners.GetAt(i);

        if (piListener->RoutingReject_NotifyRequest(piSsc, objStatusCode))
        {
            return;
        }
    }
}

PRIVATE VIRTUAL void SipRoutingRejectNotifier::AddListener(IN ISipRoutingRejectListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISipRoutingRejectListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PRIVATE VIRTUAL void SipRoutingRejectNotifier::RemoveListener(
        IN ISipRoutingRejectListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISipRoutingRejectListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}
