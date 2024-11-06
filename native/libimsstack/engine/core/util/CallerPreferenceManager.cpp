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

#include "util/CallerPreferenceManager.h"

PUBLIC
CallerPreferenceManager::CallerPreferenceManager() :
        m_objEmptyPreferenceWrapper(PreferenceWrapper()),
        m_objPreferenceWrappers(ImsMap<AString, PreferenceWrapper>())
{
}

PUBLIC
IMS_BOOL CallerPreferenceManager::CreatePreferenceWrapper(
        IN const AString& strName, IN const AString& strDialogId)
{
    IMS_SLONG nIndex = m_objPreferenceWrappers.GetIndexOfKey(strName);

    if (nIndex >= 0)
    {
        return IMS_TRUE;
    }

    PreferenceWrapper objPreferenceWrapper;

    objPreferenceWrapper.SetDialogId(strDialogId);

    return m_objPreferenceWrappers.SetValue(strName, objPreferenceWrapper);
}

PUBLIC
void CallerPreferenceManager::DestroyPreferenceWrapper(IN const AString& strName)
{
    m_objPreferenceWrappers.Remove(strName);
}

PUBLIC
const ImsList<AString>& CallerPreferenceManager::GetAcceptContacts(
        IN const AString& strDialogId) const
{
    for (IMS_UINT32 i = 0; i < m_objPreferenceWrappers.GetSize(); ++i)
    {
        const PreferenceWrapper& objPreferenceWrapper = m_objPreferenceWrappers.GetValueAt(i);

        if (strDialogId.Equals(objPreferenceWrapper.GetDialogId()))
        {
            return objPreferenceWrapper.GetAcceptContacts();
        }
    }

    return m_objEmptyPreferenceWrapper.GetAcceptContacts();
}

PUBLIC
const ImsList<AString>& CallerPreferenceManager::GetAcceptContactsByName(
        IN const AString& strName) const
{
    IMS_SLONG nIndex = m_objPreferenceWrappers.GetIndexOfKey(strName);

    if (nIndex < 0)
    {
        return m_objEmptyPreferenceWrapper.GetAcceptContacts();
    }

    const PreferenceWrapper& objPreferenceWrapper = m_objPreferenceWrappers.GetValueAt(nIndex);

    return objPreferenceWrapper.GetAcceptContacts();
}

PUBLIC
void CallerPreferenceManager::UpdateAcceptContacts(
        IN const AString& strName, IN const ImsList<AString>& objAcceptContacts)
{
    IMS_SLONG nIndex = m_objPreferenceWrappers.GetIndexOfKey(strName);

    if (nIndex < 0)
    {
        return;
    }

    PreferenceWrapper& objPreferenceWrapper = m_objPreferenceWrappers.GetValueAt(nIndex);

    objPreferenceWrapper.SetAcceptContacts(objAcceptContacts);
}

PUBLIC
void CallerPreferenceManager::UpdateDialogId(
        IN const AString& strName, IN const AString& strDialogId)
{
    IMS_SLONG nIndex = m_objPreferenceWrappers.GetIndexOfKey(strName);

    if (nIndex < 0)
    {
        return;
    }

    PreferenceWrapper& objPreferenceWrapper = m_objPreferenceWrappers.GetValueAt(nIndex);

    objPreferenceWrapper.SetDialogId(strDialogId);
}
