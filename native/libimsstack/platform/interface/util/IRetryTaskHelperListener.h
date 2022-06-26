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
#ifndef INTERFACE_RETRY_TASK_HELPER_LISTENER_H_
#define INTERFACE_RETRY_TASK_HELPER_LISTENER_H_

#include "ImsTypeDef.h"

class RetryCmd;
class RetryTaskHelper;

class IRetryTaskHelperListener
{
public:
    /**
     * @brief Notify to the user that the retry task is executed succefully or error occurred.
     *
     * When this method is invoked, the state of RetryTask is in STATE_INACTIVE.
     *
     * @param pTaskHelper The RetryTaskHelper object
     * @param pCmd The completed RetryCmd object
     * @param nCode The result code; error code, status code
     */
    virtual void RetryTaskHelper_OnCompleted(
            IN RetryTaskHelper* pTaskHelper, IN RetryCmd* pCmd, IN IMS_SINT32 nCode = 0) = 0;
};

#endif
