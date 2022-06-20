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
#include "ServiceThread.h"
#include "SystemConfig.h"

#include "base/Ims.h"

PRIVATE GLOBAL IMS_SINT32* Ims::s_pnErrorCode = IMS_NULL;

PUBLIC GLOBAL void Ims::Init()
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    s_pnErrorCode = new IMS_SINT32[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        Init(i);
    }
}

PUBLIC GLOBAL void Ims::Init(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (s_pnErrorCode != IMS_NULL)
    {
        s_pnErrorCode[nSlotId] = ImsError::NO_ERROR;
    }
}

PUBLIC GLOBAL void Ims::SetLastError(IN IMS_SINT32 nErrorCode)
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    SetLastError(nErrorCode, nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 Ims::GetLastError()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return GetLastError(nSlotId);
}

PUBLIC GLOBAL void Ims::SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (s_pnErrorCode != IMS_NULL)
    {
        s_pnErrorCode[nSlotId] = nErrorCode;
    }
}

PUBLIC GLOBAL IMS_SINT32 Ims::GetLastError(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return ImsError::NO_ERROR;
    }

    return (s_pnErrorCode != IMS_NULL) ? s_pnErrorCode[nSlotId] : ImsError::NO_ERROR;
}
