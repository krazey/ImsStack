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
#ifndef INTERFACE_RETRY_TIMER_LISTENER_H_
#define INTERFACE_RETRY_TIMER_LISTENER_H_

#include "ImsTypeDef.h"

class RetryTimer;

class IRetryTimerListener
{
protected:
    virtual ~IRetryTimerListener() = default;

public:
    /**
     * @brief Notify to the user that the interim retry timer is expired.
     *
     * NOTE: After this method returns, the retry timer will determine to start or not to start
     *       a timer according to the return value (RetryTimer.h).
     *
     * @param pTimer The RetryTimer object
     * @return The command result\n
     *         #RetryTimer#RESULT_CONTINUE\n
     *         #RetryTimer#RESULT_PENDING\n
     *         #RetryTimer#RESULT_STOP
     */
    virtual IMS_SINT32 RetryTimer_OnInterimExpired(IN RetryTimer* pTimer) = 0;

    /**
     * @brief Notify to the user that the final retry timer is expired.
     *
     * NOTE: After this method returns, the retry timer goes to the initial state.
     *
     * @param pTimer The RetryCmd object
     */
    virtual void RetryTimer_OnFinalExpired(IN RetryTimer* pTimer) = 0;
};

#endif
