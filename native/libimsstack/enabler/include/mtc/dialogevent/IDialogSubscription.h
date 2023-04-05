/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef INTERFACE_DIALOG_SUBSCRIPTION_H_
#define INTERFACE_DIALOG_SUBSCRIPTION_H_

#include "ImsTypeDef.h"

class AString;

class IDialogSubscriptionListener
{
public:
    virtual ~IDialogSubscriptionListener() = default;

    /**
     * @brief Notifies
     *
     */
    virtual void OnSubscriptionStarted() = 0;
    virtual void OnSubscriptionStartFailed() = 0;
    virtual void OnSubscriptionTerminated() = 0;
    virtual void OnSubscriptionNotified(IN const AString& strBody) = 0;
};

class IDialogSubscription
{
public:
    virtual ~IDialogSubscription() = default;

    /**
     * @brief Subscribes the dialog package of strTarget.
     *
     * @return {@code IMS_SUCCESS} if a subscription is succeeded.
     */
    virtual IMS_RESULT Subscribe() = 0;

    /**
     * @brief Unsubscribes the ongoing subscription.
     *
     */
    virtual void Unsubscribe() = 0;
};

#endif
