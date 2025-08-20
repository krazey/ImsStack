/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "AString.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "utility/SuppServiceUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL void SuppServiceUtils::Add(IN ImsList<SuppService*>& objSuppServices,
        IN IMS_SINT32 nSuppType, IN const AString& strValue)
{
    SuppService* pService = Get(objSuppServices, nSuppType);
    if (pService)
    {
        pService->strValue = strValue;
    }
    else
    {
        SuppService* pNewService = new SuppService();
        pNewService->nType = nSuppType;
        pNewService->strValue = strValue;
        objSuppServices.Append(pNewService);
    }
}

PUBLIC GLOBAL void SuppServiceUtils::Add(
        IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType, IN IMS_SINT32 nValue)
{
    SuppService* pService = Get(objSuppServices, nSuppType);
    if (pService)
    {
        pService->nValue = nValue;
    }
    else
    {
        SuppService* pNewService = new SuppService();
        pNewService->nType = nSuppType;
        pNewService->nValue = nValue;
        objSuppServices.Append(pNewService);
    }
}

PUBLIC GLOBAL void SuppServiceUtils::Add(
        IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType, IN IMS_BOOL bValue)
{
    SuppService* pService = Get(objSuppServices, nSuppType);
    if (pService)
    {
        pService->bValue = bValue;
    }
    else
    {
        SuppService* pNewService = new SuppService();
        pNewService->nType = nSuppType;
        pNewService->bValue = bValue;
        objSuppServices.Append(pNewService);
    }
}

PUBLIC GLOBAL void SuppServiceUtils::Delete(
        IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType)
{
    SuppService* pTargetSupp = Get(objSuppServices, nSuppType);
    if (pTargetSupp)
    {
        objSuppServices.Remove(pTargetSupp);
        delete pTargetSupp;
        IMS_TRACE_I("Delete : size[%d] type[%d]", objSuppServices.GetSize(), nSuppType, 0);
        return;
    }

    IMS_TRACE_I("Delete : type[%d] not found", nSuppType, 0, 0);
}

PUBLIC GLOBAL void SuppServiceUtils::DeleteServices(IN ImsList<SuppService*>& objSuppServices)
{
    IMS_UINT32 nSize = objSuppServices.GetSize();

    IMS_TRACE_I("DeleteAll : size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        SuppService* pService = objSuppServices.GetValueAt(index);
        delete pService;
    }

    objSuppServices.Clear();
}

PUBLIC GLOBAL SuppService* SuppServiceUtils::Get(
        IN const ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType)
{
    for (IMS_UINT32 index = 0; index < objSuppServices.GetSize(); index++)
    {
        if (objSuppServices.GetAt(index)->nType == nSuppType)
        {
            return objSuppServices.GetAt(index);
        }
    }

    IMS_TRACE_I("Get : NoT Matched, Size[%d]", objSuppServices.GetSize(), 0, 0);
    return IMS_NULL;
}

PUBLIC GLOBAL IMS_BOOL SuppServiceUtils::IsSameSuppServices(
        IN const ImsList<SuppService*>& objSuppServicesA,
        IN const ImsList<SuppService*>& objSuppServicesB)
{
    if (objSuppServicesA.GetSize() != objSuppServicesB.GetSize())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 indexA = 0; indexA < objSuppServicesA.GetSize(); indexA++)
    {
        SuppService* service = Get(objSuppServicesB, objSuppServicesA.GetAt(indexA)->nType);
        if (!service || *service != *objSuppServicesA.GetAt(indexA))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL ImsList<SuppService*> SuppServiceUtils::Clone(
        IN const ImsList<SuppService*>& objSuppServices)
{
    ImsList<SuppService*> objCloneSuppServices;

    for (IMS_UINT32 index = 0; index < objSuppServices.GetSize(); index++)
    {
        objCloneSuppServices.Append(new SuppService(*objSuppServices.GetAt(index)));
    }

    return objCloneSuppServices;
}
