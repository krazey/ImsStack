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

#include "SipPrivate.h"

PRIVATE GLOBAL IMS_SINT32* SipPrivate::s_pnErrorCode = IMS_NULL;

PRIVATE GLOBAL IMS_SINT32* SipPrivate::s_pnEncodingOptions = IMS_NULL;

PUBLIC GLOBAL void SipPrivate::Init()
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    s_pnErrorCode = new IMS_SINT32[nSimCount];
    s_pnEncodingOptions = new IMS_SINT32[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        Init(i, OPTIONS_E | OPT_E_FULLFORM);
    }
}

PUBLIC GLOBAL void SipPrivate::Init(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nEncodingOptions)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (s_pnErrorCode != IMS_NULL)
    {
        s_pnErrorCode[nSlotId] = 0;
    }

    if (s_pnEncodingOptions != IMS_NULL)
    {
        s_pnEncodingOptions[nSlotId] = nEncodingOptions;
    }
}

PUBLIC GLOBAL void SipPrivate::SetLastError(IN IMS_SINT32 nErrorCode)
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    SetLastError(nErrorCode, nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 SipPrivate::GetLastError()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return GetLastError(nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 SipPrivate::GetEncodingOptions()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return GetEncodingOptions(nSlotId);
}

PRIVATE GLOBAL void SipPrivate::SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId)
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

PRIVATE GLOBAL IMS_SINT32 SipPrivate::GetLastError(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return 0;
    }

    return (s_pnErrorCode != IMS_NULL) ? s_pnErrorCode[nSlotId] : 0;
}

PRIVATE GLOBAL IMS_SINT32 SipPrivate::GetEncodingOptions(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return (OPTIONS_E | OPT_E_FULLFORM);
    }

    return (s_pnEncodingOptions != IMS_NULL) ? s_pnEncodingOptions[nSlotId]
                                             : (OPTIONS_E | OPT_E_FULLFORM);
}
