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

#include "ISipMessageTrackerListener.h"
#include "SipMessageTracker.h"

PUBLIC
void SipMessageTracker::NotifyMessageReceived(
        IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN const AString& strCallId)
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    if (m_objIncomingFilters.IsEmpty())
    {
        m_piListener->MessageTracker_NotifyMessageReceived(objMethod, nStatusCode, strCallId);
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objIncomingFilters.GetSize(); ++i)
        {
            MessageFilter* pFilter = m_objIncomingFilters.GetAt(i);

            if (pFilter != IMS_NULL)
            {
                if (pFilter->Equals(objMethod, nStatusCode))
                {
                    m_piListener->MessageTracker_NotifyMessageReceived(
                            objMethod, nStatusCode, strCallId);
                    break;
                }
            }
        }
    }
}

PUBLIC
void SipMessageTracker::NotifyMessageSent(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
        IN const AString& strCallId, IN IMS_SINT32 nErrorCode /*= 0*/)
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    if (m_objOutgoingFilters.IsEmpty())
    {
        if (nErrorCode == 0)
        {
            m_piListener->MessageTracker_NotifyMessageSent(objMethod, nStatusCode, strCallId);
        }
        else
        {
            m_piListener->MessageTracker_NotifyMessageSentFailed(objMethod, nStatusCode, strCallId);
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objOutgoingFilters.GetSize(); ++i)
        {
            MessageFilter* pFilter = m_objOutgoingFilters.GetAt(i);

            if (pFilter != IMS_NULL)
            {
                if (pFilter->Equals(objMethod, nStatusCode))
                {
                    if (nErrorCode == 0)
                    {
                        m_piListener->MessageTracker_NotifyMessageSent(
                                objMethod, nStatusCode, strCallId);
                    }
                    else
                    {
                        m_piListener->MessageTracker_NotifyMessageSentFailed(
                                objMethod, nStatusCode, strCallId);
                    }
                    break;
                }
            }
        }
    }
}

PRIVATE VIRTUAL IMS_BOOL SipMessageTracker::AddFilter(
        IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN IMS_BOOL bOutgoing)
{
    MessageFilter* pFilter = new MessageFilter(objMethod, nStatusCode);

    if (pFilter == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (bOutgoing)
    {
        if (!m_objOutgoingFilters.Append(pFilter))
        {
            delete pFilter;
            return IMS_FALSE;
        }
    }
    else
    {
        if (!m_objIncomingFilters.Append(pFilter))
        {
            delete pFilter;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SipMessageTracker::RemoveFilter(IN const SipMethod& objMethod)
{
    for (IMS_UINT32 i = 0; i < m_objOutgoingFilters.GetSize();)
    {
        MessageFilter* pFilter = m_objOutgoingFilters.GetAt(i);

        if (pFilter == IMS_NULL)
        {
            ++i;
            continue;
        }

        if (pFilter->GetMethod().Equals(objMethod))
        {
            delete pFilter;

            m_objOutgoingFilters.RemoveAt(i);
            continue;
        }

        ++i;
    }

    for (IMS_UINT32 i = 0; i < m_objIncomingFilters.GetSize();)
    {
        MessageFilter* pFilter = m_objIncomingFilters.GetAt(i);

        if (pFilter == IMS_NULL)
        {
            ++i;
            continue;
        }

        if (pFilter->GetMethod().Equals(objMethod))
        {
            delete pFilter;

            m_objIncomingFilters.RemoveAt(i);
            continue;
        }

        ++i;
    }
}

PRIVATE VIRTUAL void SipMessageTracker::RemoveFilter(
        IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN IMS_BOOL bOutgoing)
{
    if (bOutgoing)
    {
        for (IMS_UINT32 i = 0; i < m_objOutgoingFilters.GetSize();)
        {
            MessageFilter* pFilter = m_objOutgoingFilters.GetAt(i);

            if (pFilter == IMS_NULL)
            {
                ++i;
                continue;
            }

            if (pFilter->Equals(objMethod, nStatusCode))
            {
                delete pFilter;

                m_objOutgoingFilters.RemoveAt(i);
                continue;
            }

            ++i;
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objIncomingFilters.GetSize();)
        {
            MessageFilter* pFilter = m_objIncomingFilters.GetAt(i);

            if (pFilter == IMS_NULL)
            {
                ++i;
                continue;
            }

            if (pFilter->Equals(objMethod, nStatusCode))
            {
                delete pFilter;

                m_objIncomingFilters.RemoveAt(i);
                continue;
            }

            ++i;
        }
    }
}

PRIVATE VIRTUAL void SipMessageTracker::RemoveAllFilters()
{
    for (IMS_UINT32 i = 0; i < m_objIncomingFilters.GetSize(); ++i)
    {
        MessageFilter* pFilter = m_objIncomingFilters.GetAt(i);

        if (pFilter != IMS_NULL)
        {
            delete pFilter;
        }
    }

    m_objIncomingFilters.Clear();

    for (IMS_UINT32 i = 0; i < m_objOutgoingFilters.GetSize(); ++i)
    {
        MessageFilter* pFilter = m_objOutgoingFilters.GetAt(i);

        if (pFilter != IMS_NULL)
        {
            delete pFilter;
        }
    }

    m_objOutgoingFilters.Clear();
}
