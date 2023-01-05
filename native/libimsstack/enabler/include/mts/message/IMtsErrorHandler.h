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

#ifndef INTERFACE_MTS_ERROR_HANDLER_H_
#define INTERFACE_MTS_ERROR_HANDLER_H_

#include "ImsTypeDef.h"

class IMessage;
class IMtsErrorHandlerListener;

class IMtsErrorHandler
{
public:
    virtual ~IMtsErrorHandler() {}

    // MtsMessageController
    virtual IMS_SINT32 Handle(IN const IMessage* piMessage = IMS_NULL) = 0;
    virtual void SetListener(IN IMtsErrorHandlerListener* piListener) = 0;
    virtual IMtsErrorHandlerListener* GetListener() = 0;
};

#endif
