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
#include "ImsProcess.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"

PUBLIC
ImsActivity::ImsActivity(IN const AString& strName /*= AString::ConstNull()*/) :
        m_strName(AString::ConstNull()),
        m_piThread(IMS_NULL)
{
    m_piThread = ThreadService::GetThreadService()->GetCurrentThread();

    const AString& strThreadName =
            (m_piThread != IMS_NULL) ? m_piThread->GetName() : AString::ConstEmpty();

    ImsAppThread* pAppThread = ImsProcess::GetInstance()->GetApplicationThread(strThreadName);

    if (pAppThread != IMS_NULL)
    {
        ImsActivityManager* pActivityManager = pAppThread->GetActivityManager();

        m_strName = pActivityManager->GenerateName(strThreadName, strName);
        pActivityManager->Attach(this);
    }
}

PUBLIC VIRTUAL ImsActivity::~ImsActivity()
{
    AString strThreadName = GetOwnerThreadName(m_strName);
    ImsAppThread* pAppThread = ImsProcess::GetInstance()->GetApplicationThread(strThreadName);

    if (pAppThread != IMS_NULL)
    {
        pAppThread->GetActivityManager()->Detach(this);
    }
}

PUBLIC
IMS_BOOL ImsActivity::PostMessage(IN ImsMessage& objMsg)
{
    if (m_piThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objMsg.SetTarget(m_strName.GetStr());

    return m_piThread->PostMessageI(objMsg);
}

PUBLIC
IMS_BOOL ImsActivity::PostMessage(IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    ImsMessage objMsg(nMsg, nWparam, nLparam);
    return PostMessage(objMsg);
}

PRIVATE GLOBAL AString ImsActivity::GetOwnerThreadName(const AString& strTargetName)
{
    // [threadname.activityname]
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.');
    return strTargetName.GetSubStr(0, nIndex);
}
