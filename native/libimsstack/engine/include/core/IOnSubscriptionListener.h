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
#ifndef INTERFACE_ON_SUBSCRIPTION_LISTENER_H_
#define INTERFACE_ON_SUBSCRIPTION_LISTENER_H_

#include "ImsTypeDef.h"

class Message;
class Subscription;

/**
 * @brief This listener type is used to notify the application about subscription status
 *        and the event state of the subscribed event package.
 *
 * @see Subscription
 */
class IOnSubscriptionListener
{
public:
    /**
     * @brief Notifies the application that the forked NOTIFY request received.
     *
     * In this moment, the method just notifies that the forked subscription is created
     * by the forked SUBSCRIBE request.
     * The NOTIFY message will be delivered by the forked subscription (pForkedSubscription).
     *
     * @param pSubscription The Subscription object
     * @param pForkedSubscription The Subscription object that is forked
     * @return true if the forked subscription is successfully processed, false otherwise.
     */
    virtual IMS_BOOL OnSubscription_ForkedNotifyReceived(
            IN Subscription* pSubscription, IN Subscription* pForkedSubscription) = 0;

    /**
     * @brief Notifies the application that the event notification received.
     *
     * @param pSubscription The Subscription object
     * @param pNotify The Message object which has an event state of the subscribed event package
     * @param bDestroyNotify Flag to indicate if the NOTIFY message should be destroyed\n
     *                       or not after returning the method
     */
    virtual void OnSubscription_NotifyReceived(
            IN Subscription* pSubscription, IN Message* pNotify, OUT IMS_BOOL& bDestroyNotify) = 0;

    /**
     * @brief Notifies the application that the durative subscription was successfully started
     *        or updated.
     *
     * @param pSubscription The Subscription object
     */
    virtual void OnSubscription_Started(IN Subscription* pSubscription) = 0;

    /**
     * @brief Notifies the application that the durative subscription failed to start or update.
     *
     * @param pSubscription The Subscription object
     */
    virtual void OnSubscription_StartFailed(IN Subscription* pSubscription) = 0;

    /**
     * @brief Notifies the application that the subscription was terminated.
     *
     * @param pSubscription The Subscription object
     */
    virtual void OnSubscription_Terminated(IN Subscription* pSubscription) = 0;
};

#endif
