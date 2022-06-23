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
#ifndef FORKED_DIALOG_METHOD_MANAGER_H_
#define FORKED_DIALOG_METHOD_MANAGER_H_

#include "AString.h"
#include "ImsMap.h"

class IForkedDialogMethod;
class IMutex;
class ISipDialog;
class ISipServerConnection;

class ForkedDialogMethodManager
{
private:
    ForkedDialogMethodManager();
    ~ForkedDialogMethodManager();

public:
    ForkedDialogMethodManager(IN const ForkedDialogMethodManager&) = delete;
    ForkedDialogMethodManager& operator=(IN const ForkedDialogMethodManager&) = delete;

public:
    IMS_BOOL AddMethod(IN const AString& strName, IN IForkedDialogMethod* piMethod);
    void RemoveMethod(IN const AString& strName);

    static ForkedDialogMethodManager* GetInstance();

private:
    IMS_BOOL HandleRequestWithinDialog(IN ISipServerConnection* piSsc, IN ISipDialog* piOrigDialog);

private:
    friend class SipConnectionNotifierManagerPrivate;

    IMutex* m_piLock;
    // Name (identifier), Pointer of IForkedDialogMethod
    IMSMap<AString, IForkedDialogMethod*> m_objDialogMethods;
};

#endif
