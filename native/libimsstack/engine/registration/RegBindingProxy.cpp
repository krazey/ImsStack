/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceResolver.h"
#include "RegBinding.h"
#include "RegBindingProxy.h"

__IMS_TRACE_TAG_REG__;

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL RegBindingProxy::CreateBinding(IN IMS_SINT32 nSlotId,
        IN const AString& strAppId, IN const AString& strServiceId, IN IRegistrationEx* piRegEx)
{
    RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding != IMS_NULL)
    {
        pRegBinding->Create(piRegEx);

        IMS_TRACE_D("REG (CREATE) :: AID=%s, SID=%s, REG=%p", strAppId.GetStr(),
                strServiceId.GetStr(), piRegEx);
        return IMS_TRUE;
    }

    pRegBinding = new RegBinding();

    if (pRegBinding == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ServiceResolver::SetRegBinding(nSlotId, strAppId, strServiceId, pRegBinding);

    pRegBinding->Create(piRegEx);

    IMS_TRACE_D("REG (CREATE) :: NEW - AID=%s, SID=%s, REG=%p", strAppId.GetStr(),
            strServiceId.GetStr(), piRegEx);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC GLOBAL void RegBindingProxy::DestroyBinding(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId)
{
    RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("REG (DESTROY) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return;
    }

    // The destructor will be invoked inside of Destroy method
    pRegBinding->Destroy();

    IMS_TRACE_D("REG (DESTROY) :: AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
            strServiceId.GetStr(), nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL void RegBindingProxy::DestroyBinding(
        IN IMS_SINT32 nSlotId, IN IRegistrationEx* piRegEx)
{
    IMSList<IRegBinding*> objIRegBindings = ServiceResolver::GetRegBindings(nSlotId);

    for (IMS_UINT32 i = 0; i < objIRegBindings.GetSize();)
    {
        RegBinding* pRegBinding = DYNAMIC_CAST(RegBinding*, objIRegBindings.GetAt(i));

        if (pRegBinding == IMS_NULL)
        {
            ++i;
            IMS_TRACE_D("REG (DESTROY) :: RegBinding is null", 0, 0, 0);
            continue;
        }

        if (pRegBinding->IsSameRegistration(piRegEx))
        {
            // The destructor will be invoked inside of Destroy method
            pRegBinding->Destroy();

            objIRegBindings.RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }

    IMS_TRACE_D("REG (DESTROY) :: COUNT (%d)", objIRegBindings.GetSize(), 0, 0);
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL RegBindingProxy::BindContact(IN IMS_SINT32 nSlotId,
        IN const AString& strAppId, IN const AString& strServiceId, IN IRegContact* piContact)
{
    RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("CONTACT (BIND) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return IMS_FALSE;
    }

    pRegBinding->UpdateContact(piContact);

    IMS_TRACE_D("CONTACT (BIND) :: AID=%s, SID=%s, CONTACT=%p", strAppId.GetStr(),
            strServiceId.GetStr(), piContact);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC GLOBAL void RegBindingProxy::UnbindContact(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId)
{
    RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("CONTACT (UNBIND) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return;
    }

    // Update the registration contact as NULL
    pRegBinding->UpdateContact(IMS_NULL);

    IMS_TRACE_D("CONTACT (UNBIND) :: AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
            strServiceId.GetStr(), nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL void RegBindingProxy::UnbindContact(IN IMS_SINT32 nSlotId, IN IRegContact* piContact)
{
    IMSList<IRegBinding*> objIRegBindings = ServiceResolver::GetRegBindings(nSlotId);

    for (IMS_UINT32 i = 0; i < objIRegBindings.GetSize(); ++i)
    {
        RegBinding* pRegBinding = DYNAMIC_CAST(RegBinding*, objIRegBindings.GetAt(i));

        if (pRegBinding != IMS_NULL)
        {
            if (pRegBinding->IsSameContact(piContact))
            {
                // Update the registration contact as NULL
                pRegBinding->UpdateContact(IMS_NULL);
            }
        }
    }
}

/*

Remarks

*/
PUBLIC GLOBAL void RegBindingProxy::QueryCapability(IN IMS_SINT32 nSlotId,
        IN const AString& strAppId, IN const AString& strServiceId,
        OUT CallerCapability*& pCapability)
{
    RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("CONTACT (CAP) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return;
    }

    pRegBinding->QueryCapability(pCapability);
}

/*

Remarks

*/
PUBLIC GLOBAL void RegBindingProxy::QueryRegistrationHeaders(IN IMS_SINT32 nSlotId,
        IN const AString& strAppId, IN const AString& strServiceId, OUT AStringArray& objHeaders)
{
    RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("REG (HEADER) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return;
    }

    pRegBinding->QueryRegistrationHeaders(objHeaders);
}
