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
#include "ServiceTrace.h"

#include "manager/AosMngrAdaptor.h"

#include "manager/AosMngr.h"
#include "handle/AosHandle.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosMngrAdaptor::AosMngrAdaptor(IN const AString& strAppName, IN IMS_SINT32 nSlotId) :
        ImsAosManager(strAppName),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosMngrAdaptor = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosMngrAdaptor), this);

    m_pAdaptee = new AosMngr(m_nSlotId);
}

PUBLIC VIRTUAL AosMngrAdaptor::~AosMngrAdaptor()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosMngrAdaptor = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosMngrAdaptor), this);

    if (m_pAdaptee != IMS_NULL)
    {
        delete m_pAdaptee;
        m_pAdaptee = IMS_NULL;
    }
}

PUBLIC VIRTUAL IImsAos* AosMngrAdaptor::GetImsAos(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    IMS_TRACE_D("GetImsAos :: [%s/%s]", strAppId.GetStr(), strServiceId.GetStr(), 0);

    IAosHandle* piHandle = m_pAdaptee->GetAosHandle(strAppId, strServiceId);
    AosHandle* pHandle = DYNAMIC_CAST(AosHandle*, piHandle);
    return DYNAMIC_CAST(IImsAos*, pHandle);
}

PUBLIC VIRTUAL ImsList<IImsAos*> AosMngrAdaptor::GetImsAosList(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    IMS_TRACE_D("GetImsAosList :: [%s/%s]", strAppId.GetStr(), strServiceId.GetStr(), 0);

    ImsList<IImsAos*> objImsAos;
    ImsList<IAosHandle*> objHandles = m_pAdaptee->GetAllAosHandles(strAppId, strServiceId);

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); i++)
    {
        IAosHandle* piHandle = objHandles.GetAt(i);

        AosHandle* pAosHandle = DYNAMIC_CAST(AosHandle*, piHandle);
        objImsAos.Append(DYNAMIC_CAST(IImsAos*, pAosHandle));
    }

    return objImsAos;
}

PUBLIC VIRTUAL ImsList<IImsAos*> AosMngrAdaptor::GetImsAosList(IN const AString& strAppId)
{
    IMS_TRACE_D("GetImsAosList :: AppId(%s)", strAppId.GetStr(), 0, 0);

    ImsList<IImsAos*> objImsAos;
    ImsList<IAosHandle*> objHandles = m_pAdaptee->GetAllAosHandles(strAppId);

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); i++)
    {
        IAosHandle* piHandle = objHandles.GetAt(i);

        AosHandle* pAosHandle = DYNAMIC_CAST(AosHandle*, piHandle);
        objImsAos.Append(DYNAMIC_CAST(IImsAos*, pAosHandle));
    }

    return objImsAos;
}
