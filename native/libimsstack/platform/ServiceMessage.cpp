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
#include "ImsActivity.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceThread.h"

__IMS_TRACE_TAG_BASE__;

PRIVATE GLOBAL AString MessageService::GetThreadName(IN const AString& strTargetName)
{
    // threadname.activitiname
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.');

    return strTargetName.GetSubStr(0, nIndex);
}

PUBLIC GLOBAL IMS_BOOL MessageService::PostMessage(
        IN const AString& strTarget, IN ImsMessage& objMsg)
{
    AString strThreadName = GetThreadName(strTarget);
    ThreadService* pThreadService = ThreadService::GetThreadService();

    pThreadService->LockThreadPool();

    IThread* piTargetThread = pThreadService->GetThread(strThreadName);
    IMS_BOOL bResult = IMS_FALSE;

    if (piTargetThread != IMS_NULL)
    {
        objMsg.SetTarget(strTarget.GetStr());

        bResult = piTargetThread->PostMessageI(objMsg);

        if (!bResult)
        {
            IMS_TRACE_D("PostMessage(failed) :: target=%s, msg=%d", strTarget.GetStr(),
                    objMsg.GetName(), 0);
        }
    }

    pThreadService->UnlockThreadPool();

    return bResult;
}

PUBLIC GLOBAL IMS_BOOL MessageService::PostMessageThread(
        IN IThread* piTargetThread, IN ImsMessage& objMsg)
{
    if (piTargetThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ThreadService* pThreadService = ThreadService::GetThreadService();
    IMS_BOOL bResult = IMS_FALSE;

    pThreadService->LockThreadPool();

    if (pThreadService->Contains(piTargetThread))
    {
        bResult = piTargetThread->PostMessageI(objMsg);

        if (!bResult)
        {
            IMS_TRACE_D("PostMessage(failed) :: msg=%d", objMsg.GetName(), 0, 0);
        }
    }

    pThreadService->UnlockThreadPool();

    return bResult;
}

PUBLIC GLOBAL IMS_BOOL MessageService::PostMessageActivity(
        IN ImsActivity* pTargetActivity, IN ImsMessage& objMsg)
{
    if (pTargetActivity == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objMsg.SetTarget(pTargetActivity->GetName().GetStr());

    return pTargetActivity->PostMessage(objMsg);
}
