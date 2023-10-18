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
#ifndef CALL_CONTROL_HELPER_H_
#define CALL_CONTROL_HELPER_H_

#include "ImsMap.h"

class ISipDialog;
class Replaces;

/**
 * @brief This class defines a 3rd party call control helper utilities.
 */
class CallControlHelper
{
private:
    CallControlHelper();
    ~CallControlHelper();

public:
    IMS_BOOL AddSession(IN const AString& strSessionId, IN Replaces* pReplaces);
    inline IMS_UINT32 GetSessionCount() const { return m_objSessions.GetSize(); }
    void RemoveSession(IN const AString& strSessionId);

    Replaces* GetReplacesFromSessionId(IN const AString& strSessionId);
    const AString& GetSessionIdFromReplaces(IN const Replaces* pReplaces);

    static Replaces* CreateReplaces(IN IMS_BOOL bMo, IN ISipDialog* piDialog);
    static const AString CreateSessionId();
    static CallControlHelper* GetInstance();

private:
    IMS_UINT32 m_nGlobalSessionId;
    // <session-id> , <replaces> pair
    ImsMap<AString, Replaces*> m_objSessions;
};

#endif
