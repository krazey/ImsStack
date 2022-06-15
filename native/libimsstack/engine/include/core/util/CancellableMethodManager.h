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
#ifndef CANCELLABLE_METHOD_MANAGER_H_
#define CANCELLABLE_METHOD_MANAGER_H_

#include "AString.h"
#include "IMSMap.h"

class ICancellableMethod;
class IMutex;
class ISipServerConnection;

class CancellableMethodManager
{
private:
    CancellableMethodManager();
    ~CancellableMethodManager();

public:
    CancellableMethodManager(IN const CancellableMethodManager&) = delete;
    CancellableMethodManager& operator=(IN const CancellableMethodManager&) = delete;

public:
    IMS_BOOL AddMethod(IN const AString& strName, IN ICancellableMethod* piMethod);
    void RemoveMethod(IN const AString& strName);

    static CancellableMethodManager* GetInstance();

private:
    IMS_BOOL HandleCancelRequest(IN ISipServerConnection* piSsc);

private:
    friend class SipConnectionNotifierManagerPrivate;

    IMutex* m_piLock;
    // Name (identifier), Pointer of ICancellableMethod
    IMSMap<AString, ICancellableMethod*> m_objCancellableMethods;
};

#endif
