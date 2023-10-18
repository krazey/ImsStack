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
#ifndef INTERFACE_REG_SUBSCRIPTION_LISTENER_H_
#define INTERFACE_REG_SUBSCRIPTION_LISTENER_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provides a listener interface for "reg" event package subscription.
 */
class IRegSubscriptionListener
{
protected:
    virtual ~IRegSubscriptionListener() = default;

public:
    /**
     * @brief Notifies the application that the event notification received.
     *
     * @param nSubsState Value of Subscription-State header
     * @param nReasonParam "reason" parameter which is in the Subscription-State header
     * @param bHasBody Flag to indicate that the NOTIFY request has message body or not
     */
    virtual void RegSubscription_NotifyReceived(
            IN IMS_SINT32 nSubState, IN IMS_SINT32 nReasonParam, IN IMS_BOOL bHasBody) = 0;

    /**
     * @brief Notifies the application when the subscription's refresh timer is expired.
     *
     * The refresh of subscription will be handled by the application.
     */
    virtual void RegSubscription_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;

    /**
     * @brief Notifies the application that the initial subscription is successfully started.
     */
    virtual void RegSubscription_Started() = 0;

    /**
     * @brief Notifies the application that the initial subscription is failed.
     *
     * @param nReason Reason code (refer to IRegSubscription)
     */
    virtual void RegSubscription_StartFailed(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies the application that the refresh or modification of subscription
     *        is successfully updated.
     */
    virtual void RegSubscription_Updated() = 0;

    /**
     * @brief Notifies the application that the refresh or modification of subscription is failed.
     *
     * @param nReason Reason code (refer to IRegSubscription)
     */
    virtual void RegSubscription_UpdateFailed(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies the application that the subscription is successfully removed.
     */
    virtual void RegSubscription_Removed() = 0;

    /**
     * @brief Notifies the application that the subscription is terminated.
     *
     * @param nReason Reason code (refer to IRegSubscription)
     */
    virtual void RegSubscription_Terminated(IN IMS_SINT32 nReason) = 0;
};

#endif
