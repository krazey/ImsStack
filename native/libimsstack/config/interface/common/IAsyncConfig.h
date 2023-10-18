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
#ifndef INTERFACE_ASYNC_CONFIG_H_
#define INTERFACE_ASYNC_CONFIG_H_

class IAsyncConfig
{
protected:
    virtual ~IAsyncConfig() = default;

public:
    /**
     * @brief Dispatches the asynchronous message for the configuration.
     *
     * @param nMsg The message typ to be handled
     * @param nParam1 The item to be handled
     * @param nParam2 The value of the specified item (action)
     */
    virtual void HandleMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2) = 0;

public:
    enum
    {
        /// Initialization operation
        /// IAsyncConfig will register itself to the AsynConfigHelper
        /// Param1 : AsyncConfigHelper*
        ACMSG_START = 1,

        /// Base of user-defined Async Config Message
        ACMSG_USER = 10
    };
};

#endif
