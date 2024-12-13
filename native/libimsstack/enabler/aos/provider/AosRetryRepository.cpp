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
#include "ServicePhoneInfo.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRetryRepository.h"
#include "provider/AosProvider.h"
#include "provider/AosRetryRepository.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosRetryRepository::AosRetryRepository(IN IMS_SINT32 nSlotId) :
        m_nRetryCount(0),
        m_nEmergencyRetryCount(0),
        m_nSlotId(nSlotId)
{
    m_strTag.Sprintf("%d", m_nSlotId);

    IMS_TRACE_D("AosRetryRepository [slot_%d]", m_nSlotId, 0, 0);
}

PUBLIC VIRTUAL AosRetryRepository::~AosRetryRepository()
{
    IMS_TRACE_D("~AosRetryRepository()", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL AosRetryRepository::IncreaseRetryCount(
        IN IMS_UINT32 nType /* = TYPE_NORMAL */)
{
    IMS_SINT32 nMaxCount = GET_N_CONFIG(m_nSlotId)->GetExtraRegErrMaxCount();
    A_IMS_TRACE_I(AOSTAG, "retryCount: %d, emergencyRetryCount: %d, maxCount: %d", m_nRetryCount,
            m_nEmergencyRetryCount, nMaxCount);

    if (nType == TYPE_NORMAL)
    {
        m_nRetryCount++;

        if (m_nRetryCount < nMaxCount)
        {
            return IMS_TRUE;
        }
        else
        {
            ResetRetryCount(TYPE_NORMAL);
            return IMS_FALSE;
        }
    }
    else
    {
        m_nEmergencyRetryCount++;

        if (m_nEmergencyRetryCount < nMaxCount)
        {
            return IMS_TRUE;
        }
        else
        {
            ResetRetryCount(TYPE_EMERGENCY);
            return IMS_FALSE;
        }
    }
}

PUBLIC VIRTUAL void AosRetryRepository::ResetRetryCount(IN IMS_UINT32 nType /* = TYPE_NORMAL */)
{
    if (nType == TYPE_NORMAL)
    {
        m_nRetryCount = 0;
    }
    else
    {
        m_nEmergencyRetryCount = 0;
    }
}
