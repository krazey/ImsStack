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
#ifndef INTERFACE_REG_SUBSCRIPTION_H_
#define INTERFACE_REG_SUBSCRIPTION_H_

#include "IRegBase.h"
#include "IRegInfo.h"

class IRegSubscriptionListener;

/**
 * @brief This class provides an interface to access and control "reg" event package subscription.
 */
class IRegSubscription : public IRegBase
{
protected:
    ~IRegSubscription() override = default;

public:
    /**
     * @brief Destroys the subscription for 'reg' event package.
     */
    virtual void DestroyEx() = 0;

    /**
     * @brief Disables the features to manage the "reg" subscription.
     *
     * @param nFeatures Features to be disabled
     * @return All the current features excluding the specified features.
     */
    virtual IMS_SINT32 DisableFeatures(IN IMS_SINT32 nFeatures) = 0;

    /**
     * @brief Enables the features to manage the "reg" subscription.
     *
     * @param nFeatures Features to be enabled
     * @return All the current features including the specified features.
     */
    virtual IMS_SINT32 EnableFeatures(IN IMS_SINT32 nFeatures) = 0;

    /**
     * @brief Returns the expiration value for this subscription.
     *
     * @return Expires value.
     */
    virtual IMS_UINT32 GetExpires() const = 0;

    /**
     * @brief Returns the interface to read the content of the registration info.
     *
     * @return Pointer to IRegInfo.
     */
    virtual const IRegInfo* GetRegInfo() const = 0;

    /**
     * @brief Returns the state of this subscription.
     *
     * @return State of subscription.\n
     *         #STATE_INACTIVE\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Sets the contact header parameter on the confirmed state.
     *
     * @param strParameter Header parameter
     * @param nOperation Operations
     *                   #CONTACT_PARAM_OP_ADD
     *                   #CONTACT_PARAM_OP_REMOVE
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @note CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
     */
    virtual IMS_RESULT SetContactParameter(
            IN const AString& strParameter, IN IMS_SINT32 nOperation = CONTACT_PARAM_OP_ADD) = 0;

    /**
     * @brief Sets the expiration value for this subscription.
     *
     * When the specified expires value is 0, the subscription will process a fetch operation.\n
     * If the application does not set this value, it will automatically calculated from
     * the expires value of registration. This method only works in the STATE_INACTIVE state.
     *
     * @param nExpires Expires value
     */
    virtual void SetExpires(IN IMS_UINT32 nExpires) = 0;

    /**
     * @brief Sets the listener for this subscription.
     *
     * @param piListener Listener interface to be set
     */
    virtual void SetListener(IN IRegSubscriptionListener* piListener) = 0;

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

    /**
     * @brief Starts or updates a durative subscription.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Subscribe() = 0;

    /**
     * @brief Terminates this durative subscription.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Unsubscribe() = 0;

public:
    /// States of IRegSubscription
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    /// Value of Subscription-State header
    enum
    {
        /// The subscription has been accepted and (in general) has been authorized.
        SUB_STATE_ACTIVE = 1,
        /// The subscription has been received by the notifier,
        /// but there is insufficient policy information to grant or deny the subscription yet.
        SUB_STATE_PENDING,
        /// The subscription terminated.
        SUB_STATE_TERMINATED
    };

    /// "reason" parameter value of Subscription-State header
    enum
    {
        PARAM_REASON_NONE = 0,

        /// The subscription has been terminated, but the subscriber
        /// SHOULD retry immediately with a new subscription.\n
        /// One primary use of such a status code is to allow migration
        /// of subscriptions between nodes.
        PARAM_REASON_DEACTIVATED,
        /// The subscription has been terminated, but the client
        /// SHOULD retry at some later time.\n
        /// If a "retry-after" parameter is also present,
        /// the client SHOULD wait at least the number of seconds specified by
        /// that parameter before attempting to re-subscribe.
        PARAM_REASON_PROBATION,
        /// The subscription has been terminated due to change in authorization policy.\n
        /// Clients SHOULD NOT attempt to re-subscribe.
        PARAM_REASON_REJECTED,
        /// The subscription has been terminated because it was not refreshed
        /// before it expired.\n
        /// Clients MAY re-subscribe immediately.
        PARAM_REASON_TIMEOUT,
        /// The subscription has been terminated because the notifier
        /// could not obtain authorization in a timely fashion.\n
        /// If a "retry-after" parameter is also present, the client SHOULD wait at least
        /// the number of seconds specified by that parameter before attempting to
        /// re-subscribe;\n
        /// otherwise, the client MAY retry immediately,
        /// but will likely get put back into pending state.
        PARAM_REASON_GIVEUP,
        /// The subscription has been terminated because the resource state
        /// which was being monitored no longer exists.\n
        /// Clients SHOULD NOT attempt to re-subscribe.
        PARAM_REASON_NORESOURCE,

        PARAM_REASON_MAX
    };

    /// Policy for subscription refresh
    enum
    {
        /// No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.\n
        ///    nCriteriaInterval : Criteria value for the refresh duration\n
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)\n
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        /// Set the remain time before it is expired\n
        ///    nCriteriaInterval : Criteria value for the refresh duration\n
        ///    nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval\n
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired\n
        ///    nCriteriaInterval : Criteria value for the refresh duration\n
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)\n
        ///    nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)\n
        /// Ex) Expires: 3600, Ratio: 10\n
        ///        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

    /// Reason codes which can be occurred on the subscription procedure
    enum
    {
        REASON_NONE = 0,
        REASON_STATUS_CODE,
        REASON_NO_EXPIRES,
        REASON_INTERNAL_ERROR,
        REASON_TRANSACTION_TIMEOUT,
        REASON_REFRESH_TIMEOUT,
        REASON_REFRESH_INTERNAL_ERROR,
        REASON_NOTIFY_TERMINATED
    };

    /// Runtime configuration features for RegSubscription
    enum
    {
        /// Indicates that the subscription remains as active
        /// even though the subscription refresh is failed by SIP transaction timeout.
        FEATURE_KEEP_ACTIVE_ON_REFRESH_TRANSACTION_TIMEOUT = 0x01,
        /// Indicates that the initial subscription will use TCP transport protocol
        /// regardless of SIP request's length.
        FEATURE_USE_TCP_TRANSPORT_ON_INITIAL_SUBSCRIPTION = 0x02
    };

    /// Operations for setting contact parameter
    enum
    {
        CONTACT_PARAM_OP_ADD = 0,
        CONTACT_PARAM_OP_REMOVE = 1
    };
};

#endif
