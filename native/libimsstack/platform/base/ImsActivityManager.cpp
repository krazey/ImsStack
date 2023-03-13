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
#include "ImsActivityManager.h"
#include "ServiceMemory.h"

PUBLIC
ImsActivityManager::ImsActivityManager() :
        m_nMajorId(0),
        m_nMinorId(0),
        m_objActivities(ImsList<ImsActivity*>())
{
}

PUBLIC
IMS_BOOL ImsActivityManager::Attach(IN ImsActivity* pActivity)
{
    if (pActivity == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_objActivities.Append(pActivity);
}

PUBLIC
void ImsActivityManager::Detach(IN const ImsActivity* pActivity)
{
    for (IMS_UINT32 i = 0; i < m_objActivities.GetSize(); ++i)
    {
        ImsActivity* pTempActivity = m_objActivities.GetAt(i);

        if (pTempActivity == pActivity)
        {
            m_objActivities.RemoveAt(i);

            if (m_objActivities.IsEmpty())
            {
                m_nMajorId = 0;
                m_nMinorId = 0;
            }
            break;
        }
    }
}

PUBLIC
ImsActivity* ImsActivityManager::Get(IN const AString& strActivityName)
{
    for (IMS_UINT32 i = 0; i < m_objActivities.GetSize(); ++i)
    {
        ImsActivity* pActivity = m_objActivities.GetAt(i);

        if (pActivity->GetName().Equals(strActivityName))
        {
            return pActivity;
        }
    }

    return IMS_NULL;
}

PUBLIC
AString ImsActivityManager::GenerateName(IN const AString& strThreadName, IN const AString& strName)
{
    AString strNewName;

    if (strName.GetLength() > 0)
    {
        strNewName.Sprintf("%s.%s", strThreadName.GetStr(), strName.GetStr());
    }
    else
    {
        strNewName.Sprintf("%s.ATVT%X_%X", strThreadName.GetStr(), m_nMajorId, m_nMinorId++);

        if (m_nMinorId == 0xFFFFFFFF)
        {
            m_nMajorId++;
            m_nMinorId = 0;
        }
    }

    return strNewName;
}

PUBLIC
IMS_BOOL ImsActivityManager::HandleMessage(IN ImsMessage& objMsg)
{
    AString strTartgetName = objMsg.GetTargetName();
    AString strTartgetActivityName = GetActivityNameFromMsg(strTartgetName);

    ImsActivity* pActivity = Get(strTartgetActivityName);

    if (pActivity != IMS_NULL)
    {
        return pActivity->DispatchMessage(objMsg);
    }

    return IMS_FALSE;
}

PUBLIC
IImsActivityController* ImsActivityManager::GetController(IN const AString& strControllerName)
{
    ImsActivity* pActivity = Get(strControllerName);

    if (pActivity != IMS_NULL)
    {
        return pActivity->GetController();
    }

    return IMS_NULL;
}

PRIVATE
AString ImsActivityManager::GetActivityNameFromMsg(IN const AString& strTargetName)
{
    IMS_SINT32 nOffset = strTargetName.GetIndexOf('.');
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.', nOffset + 1);

    return strTargetName.GetSubStr(0, nIndex);
}
