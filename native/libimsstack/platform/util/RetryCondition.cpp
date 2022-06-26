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
#include "RetryCondition.h"
#include "RetryRangeCode.h"
#include "RetrySingleCode.h"
#include "ServiceMemory.h"

/**
 * @brief Adds the single code value to this condition.
 */
PUBLIC
IMS_BOOL RetryCondition::Add(IN IMS_SINT32 nCode)
{
    RetrySingleCode* pCode = new RetrySingleCode(nCode);

    if (pCode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_objRetryCodes.Append(pCode))
    {
        delete pCode;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/**
 * @brief Adds the range code value (x > MIN && x < MAX) to this condition.
 */
PUBLIC
IMS_BOOL RetryCondition::Add(IN IMS_SINT32 nMinCode, IN IMS_SINT32 nMaxCode)
{
    RetryRangeCode* pCode = new RetryRangeCode(nMinCode, nMaxCode);

    if (pCode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_objRetryCodes.Append(pCode))
    {
        delete pCode;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/**
 * @brief Verify the condition value if the condition meets or not.
 */
PRIVATE
IMS_BOOL RetryCondition::Verify(IN IMS_SINT32 nCode) const
{
    for (IMS_UINT32 i = 0; i < m_objRetryCodes.GetSize(); ++i)
    {
        RetryCode* pCode = m_objRetryCodes.GetAt(i);

        if (pCode->IsIn(nCode))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
