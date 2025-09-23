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
#include "ServiceMutex.h"

#include "FakeRegistration.h"
#include "Registration.h"
#include "RegistrationManager.h"
#include "SipConfigProxy.h"

PUBLIC
RegistrationManager::RegistrationManager() :
        m_piLock(IMS_NULL),
        m_objRegistrations(ImsMap<RegKey, IRegistration*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC
RegistrationManager::~RegistrationManager()
{
    ClearRegistrations();
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC VIRTUAL IMS_BOOL RegistrationManager::CreateRegistration(IN IMS_SINT32 nSlotId,
        IN IMS_UINT32 nFlowId, IN const AString& strAor, IN IMS_BOOL bEmergency,
        IN IMS_BOOL bFake /*= IMS_FALSE*/, IN const AString& strSubsId /*= AString::ConstNull()*/,
        IN SipProfile* pProfile /*= IMS_NULL*/)
{
    SipAddress objAor;

    if (!objAor.Create(strAor))
    {
        return IMS_FALSE;
    }

    return CreateRegistration(nSlotId, nFlowId, objAor, bEmergency, bFake, strSubsId, pProfile);
}

PUBLIC VIRTUAL IMS_BOOL RegistrationManager::CreateRegistration(IN IMS_SINT32 nSlotId,
        IN IMS_UINT32 nFlowId, IN const SipAddress& objAor, IN IMS_BOOL bEmergency,
        IN IMS_BOOL bFake /*= IMS_FALSE*/, IN const AString& strSubsId /*= AString::ConstNull()*/,
        IN SipProfile* pProfile /*= IMS_NULL*/)
{
    if (GetRegistration(nSlotId, nFlowId) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (bFake)
    {
        FakeRegistration* pFakeReg = new FakeRegistration();

        if (pFakeReg == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pFakeReg->Create(nFlowId, objAor, bEmergency, strSubsId, pProfile))
        {
            delete pFakeReg;
            return IMS_FALSE;
        }

        LockGuard objLock(m_piLock);

        m_objRegistrations.Add(RegKey(pFakeReg->GetSlotId(), nFlowId), pFakeReg);
    }
    else
    {
        Registration* pReg = new Registration();

        if (pReg == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pReg->Create(nFlowId, objAor, bEmergency, strSubsId, pProfile))
        {
            delete pReg;
            return IMS_FALSE;
        }

        LockGuard objLock(m_piLock);

        m_objRegistrations.Add(RegKey(pReg->GetSlotId(), nFlowId), pReg);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void RegistrationManager::DestroyRegistration(
        IN IRegistration* piReg, IN IMS_BOOL bByForce /*= IMS_FALSE*/)
{
    if (piReg == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(m_piLock);

    if (m_objRegistrations.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        const IRegistration* piTmpReg = m_objRegistrations.GetValueAt(i);

        if (piTmpReg->IsNetworkInterworkingRequired() != piReg->IsNetworkInterworkingRequired())
        {
            continue;
        }

        if (piTmpReg->Equals(piReg))
        {
            if (piReg->IsNetworkInterworkingRequired())
            {
                Registration* pReg = static_cast<Registration*>(piReg);

                if (bByForce || pReg->GetAllContactsEx().IsEmpty())
                {
                    m_objRegistrations.RemoveAt(i);

                    // If the registration can be destroyed in this moment,
                    // then remove the registration from the aggregation.
                    pReg->Destroy();
                    break;
                }
            }
            else
            {
                FakeRegistration* pFakeReg = static_cast<FakeRegistration*>(piReg);

                if (bByForce || pFakeReg->GetAllContactsEx().IsEmpty())
                {
                    m_objRegistrations.RemoveAt(i);

                    // If the registration can be destroyed in this moment,
                    // then remove the registration from the aggregation.
                    pFakeReg->Destroy();
                    break;
                }
            }
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL RegistrationManager::IsRegSubscriptionSupported(
        IN IMS_SINT32 nSlotId, IN SipProfile* pProfile /*= IMS_NULL*/) const
{
    return SipConfigProxy::IsRegSubscriptionConfigured(nSlotId, pProfile);
}

PUBLIC VIRTUAL IRegistration* RegistrationManager::GetRegistration(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const
{
    LockGuard objLock(m_piLock);

    if (m_objRegistrations.IsEmpty())
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = m_objRegistrations.GetIndexOfKey(RegKey(nSlotId, nFlowId));

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objRegistrations.GetValueAt(nIndex);
}

PRIVATE
void RegistrationManager::ClearRegistrations()
{
    LockGuard objLock(m_piLock);

    if (m_objRegistrations.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        IRegistration* piReg = m_objRegistrations.GetValueAt(i);

        if (piReg != IMS_NULL)
        {
            if (piReg->IsNetworkInterworkingRequired())
            {
                Registration* pReg = static_cast<Registration*>(piReg);

                if (pReg != IMS_NULL)
                {
                    delete pReg;
                }
            }
            else
            {
                FakeRegistration* pFakeReg = static_cast<FakeRegistration*>(piReg);

                if (pFakeReg != IMS_NULL)
                {
                    delete pFakeReg;
                }
            }
        }
    }

    m_objRegistrations.Clear();
}
