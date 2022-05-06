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
#include "IIpSecPolicyListener.h"
#include "IIpSecSa.h"
#include "IIpSecSp.h"
#include "ITimer.h"
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "network/OsIpSecPolicy.h"
#include "network/OsIpSecSa.h"
#include "network/OsIpSecSp.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsIpSecPolicy::OsIpSecPolicy(IN IMS_SINT32 nId) :
        m_nId(nId),
        m_objIpSecSps(IMSList<OsIpSecSp*>()),
        m_objIpSecSas(IMSList<OsIpSecSa*>()),
        m_piListener(IMS_NULL),
        m_piTimer(IMS_NULL)
{
}

PUBLIC VIRTUAL OsIpSecPolicy::~OsIpSecPolicy()
{
    StopTimer();
    DestroyAllSas();
}

PUBLIC VIRTUAL IMS_SINT32 OsIpSecPolicy::GetId() const
{
    return m_nId;
}

PUBLIC VIRTUAL IIpSecSp* OsIpSecPolicy::CreateSp()
{
    OsIpSecSp* pIpSecSp = new OsIpSecSp();

    IMS_TRACE_D("CreateSP(%p)", pIpSecSp, 0, 0);

    m_objIpSecSps.Append(pIpSecSp);

    return pIpSecSp;
}

PUBLIC VIRTUAL void OsIpSecPolicy::DestroySp(IN IIpSecSp* piSp)
{
    IMS_TRACE_D("DestroySP(%p)", piSp, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objIpSecSps.GetSize(); i++)
    {
        OsIpSecSp* pIpSecSp = m_objIpSecSps.GetAt(i);

        if (pIpSecSp == piSp)
        {
            IMS_TRACE_I("DestroySP :: SP(%p) removed", piSp, 0, 0);
            delete pIpSecSp;
            m_objIpSecSps.RemoveAt(i);
            break;
        }
    }
}

PUBLIC VIRTUAL IIpSecSa* OsIpSecPolicy::CreateSa()
{
    OsIpSecSa* pIpSecSa = new OsIpSecSa();

    IMS_TRACE_D("CreateSA(%p)", pIpSecSa, 0, 0);

    m_objIpSecSas.Append(pIpSecSa);

    return pIpSecSa;
}

PUBLIC VIRTUAL void OsIpSecPolicy::DestroySa(IN IIpSecSa* piSa)
{
    IMS_TRACE_D("DestroySA(%p)", piSa, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objIpSecSas.GetSize(); i++)
    {
        OsIpSecSa* pIpSecSa = m_objIpSecSas.GetAt(i);

        if (pIpSecSa == piSa)
        {
            IMS_TRACE_I("DestroySA :: SA(%p) removed", piSa, 0, 0);
            delete pIpSecSa;
            m_objIpSecSas.RemoveAt(i);
            break;
        }
    }
}

PUBLIC VIRTUAL void OsIpSecPolicy::ManageLifetime(IMS_UINT32 nDuration)
{
    StopTimer();
    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    IMS_UINTP nTid = m_piTimer->SetTimer(nDuration, this);

    IMS_TRACE_I("SA lifetime - timer(%" PFLS_u ") started", nTid, 0, 0);
}

PUBLIC VIRTUAL void OsIpSecPolicy::SetListener(IN IIpSecPolicyListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
void OsIpSecPolicy::DestroyAllSas()
{
    IMS_TRACE_I("DestroyAllSAs - SP-size(%d), SA-size(%d)", m_objIpSecSps.GetSize(),
            m_objIpSecSas.GetSize(), 0);

    for (IMS_UINT32 i = 0; i < m_objIpSecSps.GetSize(); i++)
    {
        OsIpSecSp* pIpSecSp = m_objIpSecSps.GetAt(i);
        delete pIpSecSp;
    }

    m_objIpSecSps.Clear();

    for (IMS_UINT32 i = 0; i < m_objIpSecSas.GetSize(); i++)
    {
        OsIpSecSa* pIpSecSa = m_objIpSecSas.GetAt(i);
        delete pIpSecSa;
    }

    m_objIpSecSas.Clear();
}

PUBLIC
const IMSList<OsIpSecSp*>& OsIpSecPolicy::GetSPs() const
{
    return m_objIpSecSps;
}

PUBLIC
const IMSList<OsIpSecSa*>& OsIpSecPolicy::GetSAs() const
{
    return m_objIpSecSas;
}

PUBLIC
OsIpSecSp* OsIpSecPolicy::FindSp(IN IMS_UINT32 nSpi)
{
    IMS_TRACE_D("FindSp -- starts", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objIpSecSps.GetSize(); i++)
    {
        OsIpSecSp* pIpSecSp = m_objIpSecSps.GetAt(i);

        if (pIpSecSp->GetSpi() == nSpi)
        {
            IMS_TRACE_I("FindSP - SP=%p", pIpSecSp, 0, 0);
            return pIpSecSp;
        }
    }

    IMS_TRACE_D("FindSp -- ends", 0, 0, 0);

    return IMS_NULL;
}

PUBLIC
OsIpSecSa* OsIpSecPolicy::FindSa(IN IMS_UINT32 nSpi)
{
    IMS_TRACE_D("FindSa -- starts", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objIpSecSas.GetSize(); i++)
    {
        OsIpSecSa* pIpSecSa = m_objIpSecSas.GetAt(i);

        if (pIpSecSa->GetSpi() == nSpi)
        {
            IMS_TRACE_I("FindSA - SA=%p", pIpSecSa, 0, 0);
            return pIpSecSa;
        }
    }

    IMS_TRACE_D("FindSa -- ends", 0, 0, 0);

    return IMS_NULL;
}

PRIVATE VIRTUAL void OsIpSecPolicy::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piTimer)
    {
        IMS_TRACE_I("SA lifetime expired", 0, 0, 0);
        StopTimer();

        // Expired SA Life Time, Notify to Listener
        m_piListener->IpSecPolicy_OnSecurityAssociationExpired(this);
    }
    else
    {
        IMS_TRACE_E(0, "Invalid timer", 0, 0, 0);
    }
}

PRIVATE
void OsIpSecPolicy::StopTimer()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
    }
}
