/**
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

#ifndef INTERFACE_MEDIA_MANAGER_H_
#define INTERFACE_MEDIA_MANAGER_H_

#include "IMSList.h"
#include "IMSTypeDef.h"

class IMediaManager
{
public:
    virtual ~IMediaManager() {};
    virtual void SetJniMediaSessionThread(IN IMS_SINTP nCallKey,
            IN JniMediaSessionThread* pThread) = 0;

    virtual void OnResponse(IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam);
    virtual void OnVideoMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam);
};

#endif
