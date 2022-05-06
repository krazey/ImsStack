/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceThread.h"
#include "SipConfigProxy.h"
#include "RegKey.h"
#include "FakeRegistration.h"
#include "Registration.h"
#include "RegInfoManager.h"
#include "RegistrationManager.h"

class RegistrationManagerPrivate
{
public:
    RegistrationManagerPrivate();
    ~RegistrationManagerPrivate();

private:
    RegistrationManagerPrivate(IN const RegistrationManagerPrivate& objRHS);
    RegistrationManagerPrivate& operator=(IN const RegistrationManagerPrivate& objRHS);

public:
    // MULTI_SUBS
    IMS_BOOL CreateRegistration(IN IMS_UINT32 nFlowId, IN CONST SipAddress& objAOR,
            IN IMS_BOOL bFake, IN CONST AString& strSubsId, IN SipProfile* pSIPProfile);
    void DestroyRegistration(IN IRegistration* piReg, IN IMS_BOOL bByForce = IMS_FALSE);
    IRegistration* GetRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const;

private:
    void ClearRegistrations();

private:
    IMutex* piLock;
    // List of registration (bindings): < AOR (IMPU) + Contacts >
    IMSMap<RegKey, IRegistration*> objRegistrations;
};

PUBLIC
RegistrationManagerPrivate::RegistrationManagerPrivate()
{
    piLock = MutexService::GetMutexService()->CreateMutex();
    RegInfoManager::GetInstance()->Initialize();
}

PUBLIC
RegistrationManagerPrivate::~RegistrationManagerPrivate()
{
    ClearRegistrations();

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

PUBLIC
IMS_BOOL RegistrationManagerPrivate::CreateRegistration(IN IMS_UINT32 nFlowId,
        IN CONST SipAddress& objAOR, IN IMS_BOOL bFake, IN CONST AString& strSubsId,
        IN SipProfile* pSIPProfile)
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
        if (!pFakeReg->Create(nFlowId, objAOR, strSubsId, pSIPProfile))
        {
            delete pFakeReg;
            return IMS_FALSE;
        }

        LockGuard objLock(piLock);

        if (!objRegistrations.Add(RegKey(pFakeReg->GetSlotId(), nFlowId), pFakeReg))
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
        if (!pReg->Create(nFlowId, objAOR, strSubsId, pSIPProfile))
        {
            delete pReg;
            return IMS_FALSE;
        }

        LockGuard objLock(piLock);

        if (!objRegistrations.Add(RegKey(pReg->GetSlotId(), nFlowId), pReg))
        {
            delete pReg;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void RegistrationManagerPrivate::DestroyRegistration(
        IN IRegistration* piReg, IN IMS_BOOL bByForce /* = IMS_FALSE */)
{
    if (piReg == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(piLock);

    if (objRegistrations.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objRegistrations.GetSize(); ++i)
    {
        IRegistration* piTmpReg = objRegistrations.GetValueAt(i);

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
                    objRegistrations.RemoveAt(i);

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
                    objRegistrations.RemoveAt(i);

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
    LockGuard objLock(piLock);

    if (objRegistrations.IsEmpty())
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = objRegistrations.GetIndexOfKey(RegKey(nSlotId, nFlowId));

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objRegistrations.GetValueAt(nIndex);
}

PRIVATE
void RegistrationManagerPrivate::ClearRegistrations()
{
    LockGuard objLock(piLock);

    if (objRegistrations.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objRegistrations.GetSize(); ++i)
    {
        IRegistration* piReg = objRegistrations.GetValueAt(i);

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

    objRegistrations.Clear();
}

PRIVATE
RegistrationManager::RegistrationManager() :
        pRegMngrP(new RegistrationManagerPrivate())
{
}

PUBLIC
RegistrationManager::~RegistrationManager()
{
    if (pRegMngrP != IMS_NULL)
    {
        delete pRegMngrP;
    }
}

/*

Remarks
 MULTI_SUBS
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
IMS_BOOL RegistrationManager::CreateRegistration(IN IMS_UINT32 nFlowId, IN CONST AString& strAOR,
        IN IMS_BOOL bFake /* = IMS_FALSE */,
        IN CONST AString& strSubsId /* = AString::ConstNull() */,
        IN SipProfile* pSIPProfile /* = IMS_NULL*/)
{
    SipAddress objAOR;

    if (!objAOR.Create(strAOR))
    {
        return IMS_FALSE;
    }

    return CreateRegistration(nFlowId, objAOR, bFake, strSubsId, pSIPProfile);
}

/*

Remarks
 MULTI_SUBS
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
IMS_BOOL RegistrationManager::CreateRegistration(IN IMS_UINT32 nFlowId, IN CONST SipAddress& objAOR,
        IN IMS_BOOL bFake /* = IMS_FALSE */,
        IN CONST AString& strSubsId /* = AString::ConstNull() */,
        IN SipProfile* pSIPProfile /* = IMS_NULL*/)
{
    IMS_SINT32 nSlotId = ThreadService::GetCurrentSlotId();

    if (pRegMngrP->GetRegistration(nSlotId, nFlowId) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    return pRegMngrP->CreateRegistration(nFlowId, objAOR, bFake, strSubsId, pSIPProfile);
}

/*

Remarks

*/
PUBLIC
void RegistrationManager::DestroyRegistration(
        IN IRegistration* piReg, IN IMS_BOOL bByForce /* = IMS_FALSE */)
{
    pRegMngrP->DestroyRegistration(piReg, bByForce);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegistrationManager::IsRegSubscriptionSupported(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/, IN SipProfile* pSIPProfile /* = IMS_NULL*/) const
{
    return SipConfigProxy::IsRegSubscriptionConfigured(nSlotId, pSIPProfile);
}

/*

Remarks

*/
PUBLIC
IRegistration* RegistrationManager::GetRegistration(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const
{
    return pRegMngrP->GetRegistration(nSlotId, nFlowId);
}

/*

Remarks

*/
PUBLIC GLOBAL RegistrationManager* RegistrationManager::GetInstance()
{
    static RegistrationManager* pRegMngr = IMS_NULL;

    if (pRegMngr == IMS_NULL)
    {
        pRegMngr = new RegistrationManager();
    }

    return pRegMngr;
}
