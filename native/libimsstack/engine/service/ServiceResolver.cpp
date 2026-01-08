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

#include "ImsCoreContext.h"
#include "Service.h"
#include "ServiceManager.h"
#include "ServiceResolver.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL IRegBinding* ServiceResolver::GetRegBinding(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId)
{
    const Service* pService = ImsCoreContext::GetInstance()->GetServiceManager()->GetService(
            nSlotId, strAppId, strServiceId);

    if (pService != IMS_NULL)
    {
        return pService->GetRegBinding();
    }

    IMS_TRACE_E(0, "Can't find a service (%d, %s, %s)", nSlotId, strAppId.GetStr(),
            strServiceId.GetStr());

    return IMS_NULL;
}

PUBLIC GLOBAL ImsList<IRegBinding*> ServiceResolver::GetRegBindings(IN IMS_SINT32 nSlotId)
{
    ImsList<Service*> objServices =
            ImsCoreContext::GetInstance()->GetServiceManager()->GetServices(nSlotId);
    ImsList<IRegBinding*> objRegBindings;

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service* pService = objServices.GetAt(i);

        if (pService != IMS_NULL)
        {
            objRegBindings.Append(pService->GetRegBinding());
        }
    }

    IMS_TRACE_D("Count of RegBinding (%d)", objRegBindings.GetSize(), 0, 0);

    return objRegBindings;
}

PUBLIC GLOBAL void ServiceResolver::SetRegBinding(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
        IN const AString& strServiceId, IN IRegBinding* piRegBinding)
{
    Service* pService = ImsCoreContext::GetInstance()->GetServiceManager()->GetService(
            nSlotId, strAppId, strServiceId);

    if (pService != IMS_NULL)
    {
        pService->SetRegBinding(piRegBinding);
        return;
    }

    IMS_TRACE_E(0, "Can't find a service (%d, %s, %s)", nSlotId, strAppId.GetStr(),
            strServiceId.GetStr());
}
