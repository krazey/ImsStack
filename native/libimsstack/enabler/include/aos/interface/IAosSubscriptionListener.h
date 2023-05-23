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
#ifndef INTERFACE_AOS_SUBSCRIPTION_LISTENER_H_
#define INTERFACE_AOS_SUBSCRIPTION_LISTENER_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provies a listener interface for registration.
 */
class IAosSubscriptionListener
{
public:
    virtual ~IAosSubscriptionListener(){};

    /**
     * @brief Notifies the application that the state is changed.
     *
     * @param nState Value of the AosSubscription state
     * @param nReason The reason the state of AosSubscription changed
     */
    virtual void Subscription_StateChanged(IN IMS_SINT32 nState, IN IMS_SINT32 nReason = 0) = 0;

    /**
     * @brief Notifies the application that the SUBSCRIBE message can be transmitted.
     *
     * Then, the application can request that the SUBSCRIBE message is sent later.
     *
     * @return IMS_BOOL
     */
    virtual IMS_BOOL Subscription_CanBeTransmitted() = 0;

    /**
     * @brief Notifies the application when the NOTIFY message is received.
     *
     * @param nEvent The event information of NOTIFY message
     */
    virtual void Subscription_NotifyReceived(IN IMS_SINT32 nEvent) = 0;

    /**
     * @brief Notifies the application when it needs to request AosRegistration
     *
     * @param nCommand Request to AosRegistration
     * @param nRetryAfter Time to wait before the above request is proceed
     * @param bAwt Actual Waiting Time
     */
    virtual void Subscription_Request(IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter = 0,
            IN IMS_BOOL bAwt = IMS_FALSE) = 0;
};

#endif  // INTERFACE_AOS_SUBSCRIPTION_LISTENER_H_
