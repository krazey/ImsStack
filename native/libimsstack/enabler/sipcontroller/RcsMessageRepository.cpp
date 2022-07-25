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
#include "RcsMessageRepository.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "RcsMessageTracker.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC
RcsMessageRepository::RcsMessageRepository()
{
    IMS_TRACE_MEM("SNC_MSG", "IM_M : RcsMessageRepository = %" PFLS_u, sizeof(RcsMessageRepository),
            0, 0);
}

PUBLIC
RcsMessageRepository::~RcsMessageRepository()
{
    IMS_TRACE_MEM("SNC_MSG", "IM_F : RcsMessageRepository = %" PFLS_u, sizeof(RcsMessageRepository),
            0, 0);
    Clear();
}

PUBLIC
IMS_BOOL RcsMessageRepository::Add(IN RcsMessageTracker* pRcsMessage)
{
    if (pRcsMessage != IMS_NULL)
    {
        objRcsMessageTrackers.Append(pRcsMessage);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL RcsMessageRepository::Remove(IN IMS_UINTP nSessionId)
{
    RcsMessageTracker* pRcsMessage = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < GetSize(); nIndex++)
    {
        pRcsMessage = objRcsMessageTrackers.GetAt(nIndex);

        if (pRcsMessage == IMS_NULL)
        {
            IMS_TRACE_E(0, "Remove : Getting an Rcs object failed", 0, 0, 0);
            continue;
        }

        if (pRcsMessage->GetSessionId() == nSessionId)
        {
            objRcsMessageTrackers.RemoveAt(nIndex);

            IMS_TRACE_D("Remove : Removed! Session-ID[%" PFLS_u "]", nSessionId, 0, 0);

            return IMS_TRUE;
        }
    }

    IMS_TRACE_E(0, "Remove : Failed! Session-ID[%" PFLS_u "]", nSessionId, 0, 0);

    return IMS_FALSE;
}

PUBLIC
RcsMessageTracker* RcsMessageRepository::Get(IN IMS_UINTP nSessionId)
{
    RcsMessageTracker* pRcsMessage = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < GetSize(); nIndex++)
    {
        pRcsMessage = objRcsMessageTrackers.GetAt(nIndex);

        if (pRcsMessage == IMS_NULL)
        {
            IMS_TRACE_E(0, "Get : Getting an Rcs object failed", 0, 0, 0);
            continue;
        }

        if (pRcsMessage->GetSessionId() == nSessionId)
        {
            return pRcsMessage;
        }
    }

    return IMS_NULL;
}

PUBLIC
RcsMessageTracker* RcsMessageRepository::GetAt(IN IMS_UINT32 nIndex)
{
    return objRcsMessageTrackers.GetAt(nIndex);
}

PUBLIC
IMS_UINT32 RcsMessageRepository::GetSize() const
{
    return objRcsMessageTrackers.GetSize();
}

PUBLIC
void RcsMessageRepository::Clear()
{
    RcsMessageTracker* pRcsMessage = IMS_NULL;

    while (!objRcsMessageTrackers.IsEmpty())
    {
        pRcsMessage = objRcsMessageTrackers.GetAt(0);

        if (pRcsMessage != IMS_NULL)
        {
            delete pRcsMessage;
            pRcsMessage = IMS_NULL;
        }
        objRcsMessageTrackers.RemoveAt(0);
    }
}
