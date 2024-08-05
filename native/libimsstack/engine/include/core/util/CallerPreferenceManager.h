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
#ifndef CALLER_PREFERENCE_MANAGER_H_
#define CALLER_PREFERENCE_MANAGER_H_

#include "AString.h"
#include "ImsMap.h"

class CallerPreferenceManager
{
public:
    CallerPreferenceManager();
    virtual ~CallerPreferenceManager() = default;

    CallerPreferenceManager(IN const CallerPreferenceManager&) = delete;
    CallerPreferenceManager& operator=(IN const CallerPreferenceManager&) = delete;

public:
    IMS_BOOL CreatePreferenceWrapper(IN const AString& strName, IN const AString& strDialogId);
    void DestroyPreferenceWrapper(IN const AString& strName);
    const ImsList<AString>& GetAcceptContacts(IN const AString& strDialogId) const;
    const ImsList<AString>& GetAcceptContactsByName(IN const AString& strName) const;
    void UpdateAcceptContacts(
            IN const AString& strName, IN const ImsList<AString>& objAcceptContacts);
    void UpdateDialogId(IN const AString& strName, IN const AString& strDialogId);

private:
    class PreferenceWrapper
    {
    public:
        inline PreferenceWrapper() :
                m_strDialogId(AString::ConstNull()),
                m_objAcceptContacts(ImsList<AString>())
        {
        }

        inline PreferenceWrapper(IN const PreferenceWrapper& other) :
                m_strDialogId(other.m_strDialogId),
                m_objAcceptContacts(other.m_objAcceptContacts)
        {
        }

        inline ~PreferenceWrapper() = default;

    public:
        inline PreferenceWrapper& operator=(IN const PreferenceWrapper& other)
        {
            if (this != &other)
            {
                m_strDialogId = other.m_strDialogId;
                m_objAcceptContacts = other.m_objAcceptContacts;
            }

            return (*this);
        }

    public:
        inline const ImsList<AString>& GetAcceptContacts() const { return m_objAcceptContacts; }
        inline const AString& GetDialogId() const { return m_strDialogId; }
        inline void SetAcceptContacts(IN const ImsList<AString>& objAcceptContacts)
        {
            m_objAcceptContacts = objAcceptContacts;
        }
        inline void SetDialogId(IN const AString& strDialogId) { m_strDialogId = strDialogId; }

    public:
        AString m_strDialogId;
        ImsList<AString> m_objAcceptContacts;
    };

private:
    // Empty PreferenceWrapper
    PreferenceWrapper m_objEmptyPreferenceWrapper;
    // Name (identifier) / AcceptContactWrapper
    ImsMap<AString, PreferenceWrapper> m_objPreferenceWrappers;
};

#endif
