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

#include "RegBinding.h"
#include "RegBindingProxy.h"
#include "ServiceResolver.h"

__IMS_TRACE_TAG_REG__;

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

PUBLIC GLOBAL void RegBindingProxy::DestroyBinding(
        IN IMS_SINT32 nSlotId, IN const IRegistrationEx* piRegEx)
{
    ImsList<IRegBinding*> objRegBindings = ServiceResolver::GetRegBindings(nSlotId);

    for (IMS_UINT32 i = 0; i < objRegBindings.GetSize();)
    {
        RegBinding* pRegBinding = DYNAMIC_CAST(RegBinding*, objRegBindings.GetAt(i));

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

            objRegBindings.RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }

    IMS_TRACE_D("REG (DESTROY) :: COUNT (%d)", objRegBindings.GetSize(), 0, 0);
}

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

PUBLIC GLOBAL void RegBindingProxy::UnbindContact(
        IN IMS_SINT32 nSlotId, IN const IRegContact* piContact)
{
    ImsList<IRegBinding*> objRegBindings = ServiceResolver::GetRegBindings(nSlotId);

    for (IMS_UINT32 i = 0; i < objRegBindings.GetSize(); ++i)
    {
        RegBinding* pRegBinding = DYNAMIC_CAST(RegBinding*, objRegBindings.GetAt(i));

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

PUBLIC GLOBAL void RegBindingProxy::QueryCapability(IN IMS_SINT32 nSlotId,
        IN const AString& strAppId, IN const AString& strServiceId,
        OUT CallerCapability*& pCapability)
{
    const RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("CONTACT (CAP) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return;
    }

    pRegBinding->QueryCapability(pCapability);
}

PUBLIC GLOBAL void RegBindingProxy::QueryRegistrationHeaders(IN IMS_SINT32 nSlotId,
        IN const AString& strAppId, IN const AString& strServiceId, OUT AStringArray& objHeaders)
{
    const RegBinding* pRegBinding = DYNAMIC_CAST(
            RegBinding*, ServiceResolver::GetRegBinding(nSlotId, strAppId, strServiceId));

    if (pRegBinding == IMS_NULL)
    {
        IMS_TRACE_D("REG (HEADER) :: NOT FOUND - AID=%s, SID=%s, slotId=%d", strAppId.GetStr(),
                strServiceId.GetStr(), nSlotId);
        return;
    }

    pRegBinding->QueryRegistrationHeaders(objHeaders);
}
