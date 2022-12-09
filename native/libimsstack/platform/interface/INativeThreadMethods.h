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
#ifndef INTERFACE_NATIVE_THREAD_METHODS_H_
#define INTERFACE_NATIVE_THREAD_METHODS_H_

#include "ImsTypeDef.h"

class INativeThreadMethods
{
protected:
    virtual ~INativeThreadMethods() = default;

public:
    /**
     * @brief Attaches the native thread.
     */
    virtual void AttachNativeThread(IN const IMS_CHAR* pszName) = 0;

    /**
     * @brief Detaches the native thread.
     */
    virtual void DetachNativeThread() = 0;
};

#endif
