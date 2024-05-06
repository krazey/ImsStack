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

#ifndef INTERFACE_JNI_MEDIA_MANAGER_H_
#define INTERFACE_JNI_MEDIA_MANAGER_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "INativeEnabler.h"

class IJniMediaManager : public INativeEnabler
{
public:
    virtual ~IJniMediaManager(){};

    /**
     * @brief Sends a message from java layer through jni interface
     *
     * @param nMsg Enum of message defined in IJniMedia.h
     * @param nCallKey The key to identify the media session instance
     * @param pParam The message parameter, it is defined in IJniMedia.h
     *
     * @return IMS_BOOL return IMS_TRUE when SendMessage is successful, otherwise return IMS_FALSE.
     */
    virtual IMS_BOOL SendMessage(
            IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam) = 0;
};

#endif
