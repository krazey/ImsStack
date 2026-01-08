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
#include "ServiceTrace.h"

#include "ServiceFilterCriteria.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
ServiceFilterCriteria::ServiceFilterCriteria() :
        m_objCalleePreferences(ImsMap<IMS_SINT32, IMS_BOOL>()),
        m_nNextTriggerPointId(1),
        m_objTriggerPoints(ImsMap<IMS_UINT32, TriggerPoint*>())
{
    m_objCalleePreferences.Add(SipMethod::INVITE, IMS_FALSE);
    m_objCalleePreferences.Add(SipMethod::OPTIONS, IMS_FALSE);
    m_objCalleePreferences.Add(SipMethod::MESSAGE, IMS_FALSE);
    m_objCalleePreferences.Add(SipMethod::REFER, IMS_FALSE);
}

PUBLIC VIRTUAL ServiceFilterCriteria::~ServiceFilterCriteria()
{
    RemoveAllTriggerPoints();
}

PUBLIC VIRTUAL IMS_UINT32 ServiceFilterCriteria::AddTriggerPoint(
        IN const TriggerPoint& objTriggerPoint)
{
    TriggerPoint* pTriggerPoint = new TriggerPoint(objTriggerPoint);

    if (pTriggerPoint == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a TriggerPoint failed", 0, 0, 0);
        return 0;
    }

    m_objTriggerPoints.Add(m_nNextTriggerPointId, pTriggerPoint);
    ++m_nNextTriggerPointId;

    return (m_nNextTriggerPointId - 1);
}

PUBLIC VIRTUAL void ServiceFilterCriteria::RemoveTriggerPoint(IN IMS_SINT32 nTriggerPointId)
{
    IMS_SLONG nIndex = m_objTriggerPoints.GetIndexOfKey(nTriggerPointId);

    if (nIndex < 0)
    {
        IMS_TRACE_D("ServiceFilterCriteria :: No trigger point (%d)", nTriggerPointId, 0, 0);
        return;
    }

    TriggerPoint* pTriggerPoint = m_objTriggerPoints.GetValueAt(nIndex);

    if (pTriggerPoint != IMS_NULL)
    {
        delete pTriggerPoint;
    }

    m_objTriggerPoints.RemoveAt(nIndex);

    if (m_objTriggerPoints.IsEmpty())
    {
        m_nNextTriggerPointId = 1;
    }
}

PUBLIC VIRTUAL void ServiceFilterCriteria::RemoveAllTriggerPoints()
{
    for (IMS_UINT32 i = 0; i < m_objTriggerPoints.GetSize(); ++i)
    {
        TriggerPoint* pTriggerPoint = m_objTriggerPoints.GetValueAt(i);

        if (pTriggerPoint == IMS_NULL)
        {
            continue;
        }

        delete pTriggerPoint;
    }

    m_objTriggerPoints.Clear();

    m_nNextTriggerPointId = 1;
}

PUBLIC VIRTUAL void ServiceFilterCriteria::SetCalleePreference(
        IN const SipMethod& objMethod, IN IMS_BOOL bCalleePreference /*= IMS_TRUE*/)
{
    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::OPTIONS) &&
            !objMethod.Equals(SipMethod::MESSAGE) && !objMethod.Equals(SipMethod::REFER))
    {
        return;
    }

    m_objCalleePreferences.SetValue(objMethod.ToInt(), bCalleePreference);
}

PUBLIC
IMS_UINT32 ServiceFilterCriteria::Evaluate(IN const ISipMessage* piSipMsg) const
{
    for (IMS_UINT32 i = 0; i < m_objTriggerPoints.GetSize(); ++i)
    {
        const TriggerPoint* pTriggerPoint = m_objTriggerPoints.GetValueAt(i);

        if (pTriggerPoint == IMS_NULL)
        {
            continue;
        }

        if (pTriggerPoint->Evaluate(piSipMsg))
        {
            return pTriggerPoint->GetCount();
        }
    }

    return 0;
}

PUBLIC
IMS_BOOL ServiceFilterCriteria::IsCalleePreferenceSupported(IN const SipMethod& objMethod) const
{
    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::OPTIONS) &&
            !objMethod.Equals(SipMethod::MESSAGE) && !objMethod.Equals(SipMethod::REFER))
    {
        return IMS_FALSE;
    }

    return m_objCalleePreferences.GetValue(objMethod.ToInt());
}
