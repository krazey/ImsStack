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
#ifndef INTERFACE_SUBSCRIPTION_LISTENER_H_
#define INTERFACE_SUBSCRIPTION_LISTENER_H_

#include "ImsTypeDef.h"

class IMessage;
class ISubscription;

/**
 * @brief This class provides a listener interface to notify the application
 *        about subscription status and the event state of the subscribed event package.
 *
 * @see ISubscription, IMessage
 */
class ISubscriptionListener
{
public:
    /**
     * @brief Notifies the application that the event notification
     *        (to the forked SUBSCRIBE request) received.
     *
     * If the subscription is durative, the state remains at STATE_ACTIVE,
     * otherwise the state will transit to STATE_INACTIVE.
     *
     * @param piSubscription The concerned ISubscription
     * @param piForkedSubscription The forked ISubscription
     */
    virtual void SubscriptionForkedNotify(
            IN ISubscription* piSubscription, IN ISubscription* piForkedSubscription) = 0;

    /**
     * @brief Notifies the application that the event notification received.
     *
     * If the subscription is durative, the state remains at STATE_ACTIVE,
     * otherwise the state will transit to STATE_INACTIVE.
     *
     * @param piSubscription The concerned ISubscription
     * @param piNotify Pointer to IMessage which has an event state
     *                 of the subscribed event package
     */
    virtual void SubscriptionNotify(IN ISubscription* piSubscription, IN IMessage* piNotify) = 0;

    /**
     * @brief Notifies the application that the durative subscription was successfully
     *        started or updated.
     *
     * @param piSubscription The concerned ISubscription
     */
    virtual void SubscriptionStarted(IN ISubscription* piSubscription) = 0;

    /**
     * @brief Notifies the application that the durative subscription failed to start or update.
     *
     * @param piSubscription The concerned ISubscription
     */
    virtual void SubscriptionStartFailed(IN ISubscription* piSubscription) = 0;

    /**
     * @brief Notifies the application that the subscription was terminated.
     *
     * @param piSubscription The concerned ISubscription
     */
    virtual void SubscriptionTerminated(IN ISubscription* piSubscription) = 0;
};

#endif
