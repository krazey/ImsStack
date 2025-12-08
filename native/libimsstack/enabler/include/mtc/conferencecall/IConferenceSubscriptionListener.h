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

#ifndef INTERFACE_CONFERENCE_SUBSCRIPTION_LISTENER_H_
#define INTERFACE_CONFERENCE_SUBSCRIPTION_LISTENER_H_

/**
 * @brief Defines the types of updates for a conference subscription.
 */
enum class SubscriptionUpdateType
{
    /** The subscription request was successful. */
    SUCCEEDED = 0,
    /** The subscription request failed. */
    FAILED,
    /** The unsubscription request was successful. */
    UNSUBSCRIBED,
    /** The subscription has been terminated. */
    TERMINATED,
    /** A NOTIFY message with conference event package has been received. */
    NOTIFY_RECEIVED
};

/**
 * @brief Listener for events from a {@link ConferenceSubscription}.
 *
 * This interface allows for receiving notifications about the lifecycle of a
 * conference subscription, such as success, failure, or termination.
 */
class IConferenceSubscriptionListener
{
public:
    virtual ~IConferenceSubscriptionListener() = default;

    /**
     * @brief Notifies of an update to the conference subscription.
     *
     * This callback is invoked when the state of the conference subscription changes.
     *
     * @param eType The type of the subscription update. See {@link SubscriptionUpdateType}.
     */
    virtual void OnSubscriptionUpdated(IN SubscriptionUpdateType eType) = 0;
};

#endif
