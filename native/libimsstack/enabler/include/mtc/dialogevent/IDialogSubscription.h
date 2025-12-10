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

/**
 * @brief Listener for dialog subscription events.
 */
class IDialogSubscriptionListener
{
public:
    virtual ~IDialogSubscriptionListener() = default;

    /** Notifies that the subscription has been successfully started. */
    virtual void OnSubscriptionStarted() = 0;

    /** Notifies that the subscription has failed to start. */
    virtual void OnSubscriptionStartFailed() = 0;

    /** Notifies that the subscription has been terminated. */
    virtual void OnSubscriptionTerminated() = 0;

    /**
     * @brief Notifies that a dialog event package has been received.
     *
     * @param strBody The XML body of the received NOTIFY request.
     */
    virtual void OnSubscriptionNotified(IN const AString& strBody) = 0;
};

/**
 * @brief An interface for managing a subscription to the dialog event package.
 */
class IDialogSubscription
{
public:
    virtual ~IDialogSubscription() = default;

    /**
     * @brief Subscribes to the dialog event package for a target URI.
     *
     * @return {@code IMS_SUCCESS} if the subscription request is sent successfully,
     *         {@code IMS_FAILURE} otherwise.
     */
    virtual IMS_RESULT Subscribe() = 0;

    /** Unsubscribes from the ongoing subscription. */
    virtual void Unsubscribe() = 0;
};

#endif
