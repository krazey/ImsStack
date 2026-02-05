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
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "helper/MtcPermanentSupplementaryService.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcPermanentSupplementaryService::MtcPermanentSupplementaryService() :
        m_objPermanentSuppServices(ImsList<SuppService*>())
{
}

PUBLIC
MtcPermanentSupplementaryService::~MtcPermanentSupplementaryService()
{
    DeleteServices();
}

PUBLIC
void MtcPermanentSupplementaryService::UpdateServices(
        IN const ImsList<SuppService*>& objSuppServices)
{
    IMS_UINT32 nInServiceSize = objSuppServices.GetSize();
    IMS_TRACE_I("UpdateServices : ServiceSize[%d] InServiceSize[%d]",
            m_objPermanentSuppServices.GetSize(), nInServiceSize, 0);

    for (IMS_UINT32 i = 0; i < nInServiceSize; i++)
    {
        IMS_SINT32 eSuppType = objSuppServices.GetAt(i)->nType;
        if (SuppServiceUtils::Get(m_objPermanentSuppServices, eSuppType) != IMS_NULL)
        {
            Delete(static_cast<PermanentSuppType>(eSuppType));
        }

        IMS_TRACE_I("UpdateServices : Append[%d]", eSuppType, 0, 0);
        m_objPermanentSuppServices.Append(objSuppServices.GetAt(i));
    }
}

PUBLIC
IMS_BOOL MtcPermanentSupplementaryService::IsEnabled(IN PermanentSuppType ePermanentSuppType) const
{
    const SuppService* pSuppService = Get(ePermanentSuppType);
    if (pSuppService == IMS_NULL)
    {
        return ePermanentSuppType == PermanentSuppType::TB_CW;
    }

    return pSuppService->bValue;
}
