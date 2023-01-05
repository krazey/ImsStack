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
#include "ServiceThread.h"

#include "FakeRegistration.h"
#include "RegInfoManager.h"
#include "RegKey.h"
#include "Registration.h"
#include "RegistrationManager.h"
#include "SipConfigProxy.h"

class RegistrationManagerPrivate
{
public:
    RegistrationManagerPrivate();
    ~RegistrationManagerPrivate();

    RegistrationManagerPrivate(IN const RegistrationManagerPrivate&) = delete;
    RegistrationManagerPrivate& operator=(IN const RegistrationManagerPrivate&) = delete;

public:
    IMS_BOOL CreateRegistration(IN IMS_UINT32 nFlowId, IN const SipAddress& objAor,
            IN IMS_BOOL bFake, IN const AString& strSubsId, IN SipProfile* pProfile);
    void DestroyRegistration(IN IRegistration* piReg, IN IMS_BOOL bByForce = IMS_FALSE);
    IRegistration* GetRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const;

private:
    void ClearRegistrations();

private:
    IMutex* m_piLock;
    // List of registration (bindings): < AOR (IMPU) + Contacts >
    IMSMap<RegKey, IRegistration*> m_objRegistrations;
};

PUBLIC
RegistrationManagerPrivate::RegistrationManagerPrivate()
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
    RegInfoManager::GetInstance()->Initialize();
}

PUBLIC
RegistrationManagerPrivate::~RegistrationManagerPrivate()
{
    ClearRegistrations();

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
IMS_BOOL RegistrationManagerPrivate::CreateRegistration(IN IMS_UINT32 nFlowId,
        IN const SipAddress& objAor, IN IMS_BOOL bFake, IN const AString& strSubsId,
        IN SipProfile* pProfile)
{
    // Fine a proper registration flow, but in this time, it uses a default registration flow.

    if (bFake)
    {
        FakeRegistration* pFakeReg = new FakeRegistration();

        if (pFakeReg == IMS_NULL)
        {
            return IMS_FALSE;
        }

        // MULTI_SUBS
        // MULTI_REG_SIP_PROFILE
        if (!pFakeReg->Create(nFlowId, objAor, strSubsId, pProfile))
        {
            delete pFakeReg;
            return IMS_FALSE;
        }

        LockGuard objLock(m_piLock);

        if (!m_objRegistrations.Add(RegKey(pFakeReg->GetSlotId(), nFlowId), pFakeReg))
        {
            delete pFakeReg;
            return IMS_FALSE;
        }
    }
    else
    {
        Registration* pReg = new Registration();

        if (pReg == IMS_NULL)
        {
            return IMS_FALSE;
        }

        // MULTI_SUBS
        // MULTI_REG_SIP_PROFILE
        if (!pReg->Create(nFlowId, objAor, strSubsId, pProfile))
        {
            delete pReg;
            return IMS_FALSE;
        }

        LockGuard objLock(m_piLock);

        if (!m_objRegistrations.Add(RegKey(pReg->GetSlotId(), nFlowId), pReg))
        {
            delete pReg;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void RegistrationManagerPrivate::DestroyRegistration(
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
        IRegistration* piTmpReg = m_objRegistrations.GetValueAt(i);

        if (piTmpReg->IsNetworkInterworkingRequired() != piReg->IsNetworkInterworkingRequired())
        {
            continue;
        }

        if (piTmpReg->Equals(piReg))
        {
            if (piReg->IsNetworkInterworkingRequired())
            {
                Registration* pReg = DYNAMIC_CAST(Registration*, piReg);

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
                FakeRegistration* pFakeReg = DYNAMIC_CAST(FakeRegistration*, piReg);

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

PUBLIC
IRegistration* RegistrationManagerPrivate::GetRegistration(
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
void RegistrationManagerPrivate::ClearRegistrations()
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
                Registration* pReg = DYNAMIC_CAST(Registration*, piReg);

                if (pReg != IMS_NULL)
                {
                    delete pReg;
                }
            }
            else
            {
                FakeRegistration* pFakeReg = DYNAMIC_CAST(FakeRegistration*, piReg);

                if (pFakeReg != IMS_NULL)
                {
                    delete pFakeReg;
                }
            }
        }
    }

    m_objRegistrations.Clear();
}

PRIVATE
RegistrationManager::RegistrationManager() :
        m_pRegMngrPrivate(new RegistrationManagerPrivate())
{
}

PUBLIC
RegistrationManager::~RegistrationManager()
{
    if (m_pRegMngrPrivate != IMS_NULL)
    {
        delete m_pRegMngrPrivate;
    }
}

PUBLIC
IMS_BOOL RegistrationManager::CreateRegistration(IN IMS_UINT32 nFlowId, IN const AString& strAor,
        IN IMS_BOOL bFake /*= IMS_FALSE*/, IN const AString& strSubsId /*= AString::ConstNull()*/,
        IN SipProfile* pProfile /*= IMS_NULL*/)
{
    SipAddress objAor;

    if (!objAor.Create(strAor))
    {
        return IMS_FALSE;
    }

    return CreateRegistration(nFlowId, objAor, bFake, strSubsId, pProfile);
}

PUBLIC
IMS_BOOL RegistrationManager::CreateRegistration(IN IMS_UINT32 nFlowId, IN const SipAddress& objAor,
        IN IMS_BOOL bFake /*= IMS_FALSE*/, IN const AString& strSubsId /*= AString::ConstNull()*/,
        IN SipProfile* pProfile /*= IMS_NULL*/)
{
    IMS_SINT32 nSlotId = ThreadService::GetCurrentSlotId();

    if (m_pRegMngrPrivate->GetRegistration(nSlotId, nFlowId) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    return m_pRegMngrPrivate->CreateRegistration(nFlowId, objAor, bFake, strSubsId, pProfile);
}

PUBLIC
void RegistrationManager::DestroyRegistration(
        IN IRegistration* piReg, IN IMS_BOOL bByForce /*= IMS_FALSE*/)
{
    m_pRegMngrPrivate->DestroyRegistration(piReg, bByForce);
}

PUBLIC
IMS_BOOL RegistrationManager::IsRegSubscriptionSupported(
        IN IMS_SINT32 nSlotId, IN SipProfile* pProfile /*= IMS_NULL*/) const
{
    return SipConfigProxy::IsRegSubscriptionConfigured(nSlotId, pProfile);
}

PUBLIC
IRegistration* RegistrationManager::GetRegistration(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const
{
    return m_pRegMngrPrivate->GetRegistration(nSlotId, nFlowId);
}

PUBLIC GLOBAL RegistrationManager* RegistrationManager::GetInstance()
{
    static RegistrationManager* s_pRegMngr = IMS_NULL;

    if (s_pRegMngr == IMS_NULL)
    {
        s_pRegMngr = new RegistrationManager();
    }

    return s_pRegMngr;
}
