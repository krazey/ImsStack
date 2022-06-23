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
#ifndef DIALOG_METHOD_MANAGER_H_
#define DIALOG_METHOD_MANAGER_H_

#include "AString.h"
#include "ImsMap.h"

class IDialogMethod;
class IMutex;
class ISipServerConnection;

class DialogMethodManager
{
private:
    DialogMethodManager();
    ~DialogMethodManager();

public:
    DialogMethodManager(IN const DialogMethodManager&) = delete;
    DialogMethodManager& operator=(IN const DialogMethodManager&) = delete;

public:
    IMS_BOOL AddMethod(IN const AString& strName, IN IDialogMethod* piMethod);
    void RemoveMethod(IN const AString& strName);
    IMS_BOOL IsEmpty() const;

    static DialogMethodManager* GetInstance();

private:
    IMS_BOOL HandleRequestWithinDialog(IN ISipServerConnection* piSsc);
    // In case of receiving a forked request ...
    IMS_BOOL HandleRequestWithinDialog(IN ISipServerConnection* piSsc, IN ISipDialog* piOrigDialog);

private:
    friend class SipConnectionNotifierManagerPrivate;

    IMutex* m_piLock;
    // Name (identifier), Pointer of IDialogMethod
    IMSMap<AString, IDialogMethod*> m_objDialogMethods;
};

#endif
