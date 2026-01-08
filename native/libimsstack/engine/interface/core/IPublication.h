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
#ifndef INTERFACE_PUBLICATION_H_
#define INTERFACE_PUBLICATION_H_

#include "ByteArray.h"

#include "IServiceMethod.h"

class IPublicationListener;
class IRefreshListener;

/**
 * @brief This class provides an interface for publishing event state to a remote endpoint.
 *
 * @see IServiceMethod, IPublicationListener
 */
class IPublication : public IServiceMethod
{
protected:
    ~IPublication() override = default;

public:
    /**
     * @brief Returns the event package corresponding to this IPublication.
     *
     * @return The event package name.
     */
    virtual const AString& GetEvent() const = 0;

    /**
     * @brief Returns the current state of the state machine of the IPublication.
     *
     * @return The current state of this IPublication.\n
     *         #STATE_INACTIVE\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Sends a publication request with an event state to the remote endpoint.
     *
     * @param objState Event state to be published
     * @param strContentType Content MIME type of an event state
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Publish(IN const ByteArray& objState, IN const AString& strContentType) = 0;

    /**
     * @brief Sets a listener for this IPublication, replacing any previous IPublicationListener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN IPublicationListener* piListener) = 0;

    /**
     * @brief Terminates this publication.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Unpublish() = 0;

    /**
     * @brief Sets a refresh listener for this IPublication, replacing any previous
     *        IRefreshListener.
     *
     * A null reference is allowed and has the effect of removing any existing listener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetRefreshListener(IN IRefreshListener* piListener) = 0;

    /**
     * @brief Sets the refresh policy for the publication refresh.
     *
     * This policy will be applied from when the refresh operation of the current publication
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
    /// States of IPublication
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    /// Policy for publication refresh
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
