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
#ifndef INTERFACE_SUBSCRIPTION_H_
#define INTERFACE_SUBSCRIPTION_H_

#include "IServiceMethod.h"

class IRefreshListener;
class ISubscriptionListener;

/**
 * @brief This class provides an interface for subscribing to events of the event package sent
 *        from the remote server endpoint.
 *
 * The application receives event notifications via callbacks to SubscriptionNotify(...) method
 * in ISubscriptionListener class while being subscribed.\n
 * There are two types of a subscription:
 *     - A durative subscription starts with a call to Subscribe(...)
 *         and ends with a call to Unsubscribe(...).
 *     - An instant subscription starts with a call to Poll(...)
 *         and ends when the first notification is received.
 *
 * @see IServiceMethod, ISubscriptionListener
 */
class ISubscription : public IServiceMethod
{
protected:
    ~ISubscription() override = default;

public:
    /**
     * @brief Returns the event package corresponding to this ISubscription.
     *
     * @return Event package name.
     */
    virtual const AString& GetEvent() const = 0;

    /**
     * @brief Returns the current state of the state machine of the ISubscription.
     *
     * @return The current state of Subscription.\n
     *         #STATE_INACTIVE\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Makes an instant subscription.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Poll() = 0;

    /**
     * @brief Sets a listener for this ISubscription, replacing any previous ISubscriptionListener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISubscriptionListener* piListener) = 0;

    /**
     * @brief Starts or updates a durative subscription.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Subscribe() = 0;

    /**
     * @brief Terminates this durative subscription.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Unsubscribe() = 0;

    /**
     * @brief Sets the flag if the mid-dialog request needs to be implicitly routed
     *        to the preloaded topmost route address.
     *
     * NOTE: It MUST be invoked when the route set (R-R headers) is not present, and the mid-dialog
     * request needs to be routed to the preloaded topmost address (i.e. P-CSCF or SBC address).
     * This option will be applied when no Route header exists in the request.
     *
     * @param bFlag Indicates the flag if the implicit routing is required or not
     * @note IMPLICIT_ROUTING_FOR_MID_DIALOG
     */
    virtual void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) = 0;

    /**
     * @brief Sets a refresh listener for this ISubscription,
     *        replacing any previous IRefreshListener.
     *
     * A null reference is allowed and has the effect of removing any existing listener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetRefreshListener(IN IRefreshListener* piListener) = 0;

    /**
     * @brief Sets the refresh policy for the subscription refresh.
     *
     * This policy will be applied from when the refresh operation of the current subscription
     * is invoked.
     *
     * @param nPolicy Refresh policy to be set\n
     *                #REFRESH_POLICY_NO_REFRESH\n
     *                #REFRESH_POLICY_SPEC\n
     *                #REFRESH_POLICY_REMAIN_TIME\n
     *                #REFRESH_POLICY_RATIO
     * @param nCriteriaInterval Criteria interval to determine the refresh interval
     * @param nValueEorLt Interval value when the refresh duration is equal or less
     *                    than the criteria interval
     * @param nValueGt Interval value when the refresh duration is greater
     *                 than the criteria interval
     */
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) = 0;

public:
    /// States of ISubscription
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    /// Policy for subscription refresh
    enum
    {
        /// No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.
        ///    - nCriteriaInterval : Criteria value for the refresh duration
        ///    - nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)
        ///    - nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        /// Set the remain time before it is expired
        ///    - nCriteriaInterval : Criteria value for the refresh duration
        ///    - nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval
        ///    - nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired
        ///    - nCriteriaInterval : Criteria value for the refresh duration
        ///    - nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)
        ///    - nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)
        /// Ex) Expires: 3600, Ratio: 10
        ///        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };
};

#endif
