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
#ifndef INTERFACE_CAPABILITIES_H_
#define INTERFACE_CAPABILITIES_H_

#include "IServiceMethod.h"

class ICapabilitiesListener;

/**
 * @brief This class provides an interface for SIP OPTIONS transaction.
 *
 * The Capabilities is used to query a remote endpoint
 * whether it has matching capabilities matching the local ones.
 *
 * @see ICoreService, ICapabilitiesListener
 */
class ICapabilities : public IServiceMethod
{
protected:
    ~ICapabilities() override = default;

public:
    /**
     * @brief Returns an array of strings representing valid user identities
     *        for the remote endpoint.
     *
     * @return All SIP & TEL URIs which are in the Contact header.
     */
    virtual ImsList<AString> GetRemoteUserIdentities() const = 0;

    /**
     * @brief Returns the current state of this ICapabilities.
     *
     * @return State of this Capabilities.\n
     *         #STATE_INACTIVE\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief This method returns true if the remote endpoint is believed to be sufficiently
     *        capable of handling requests from a certain core service on the local endpoint.
     *
     * The core service is specified with a "imscore" Connector string.
     *
     * @param strConnection Connector ("imscore") string to be checked
     * @return If this ICapabilities has the given capabilities, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL HasCapabilities(IN const AString& strConnection) const = 0;

    /**
     * @brief Sends a capability request to a remote endpoint.
     *
     * @param nFlags Flags indicating that which information should be added
     *               or checked for a capability request\n
     *               The value is bitwise OR of
     *                   #FLAG_ADD_CONTACT_HEADER
     *                   #FLAG_ADD_ALL_REGISTERED_FEATURES_IN_CONTACT_HEADER
     *                   #FLAG_ADD_SDP_BODY_PART
     *                   #FLAG_CHECK_MEDIA_CAPABILITIES
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT QueryCapabilities(IN IMS_SINT32 nFlags = FLAG_REQUEST_DEFAULT) = 0;

    virtual IMS_RESULT QueryCapabilitiesEx()
            __IMS_DEPRECATED__("Use QueryCapabilities(IMS_SINT32) instead") = 0;

    /**
     * @brief Sets a listener for this ICapabilities, replacing any previous ICapabilitiesListener.
     *
     * A null reference is allowed and has the effect of removing any existing listener.
     *
     * @param piListener Pointer to the listener for receiving the result of capability query
     */
    virtual void SetListener(IN ICapabilitiesListener* piListener) = 0;

    /**
     * @brief Sends a successful final response to an incoming capability query
     *        from a remote endpoint.
     *
     * @param nFlags Flags indicating that which information should be added
     *               or checked for a successful response\n
     *               The value is bitwise OR of
     *                   #FLAG_ADD_CONTACT_HEADER
     *                   #FLAG_ADD_ALL_REGISTERED_FEATURES_IN_CONTACT_HEADER
     *                   #FLAG_ADD_SDP_BODY_PART
     *                   #FLAG_CHECK_MEDIA_CAPABILITIES
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Accept(IN IMS_SINT32 nFlags = FLAG_RESPONSE_DEFAULT) = 0;

    /**
     * @brief Sends a successful final response to an incoming capability query
     *        from a remote endpoint.
     *
     * The application SHOULD set the Contact header & message body
     * according to the implementation.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AcceptEx() __IMS_DEPRECATED__("Use Accept(IMS_SINT32) instead") = 0;

    /**
     * @brief Sends a failure final response to an incoming capability query
     *        from a remote endpoint.
     *
     * @param nStatusCode SIP status code
     * @param nRetryAfter Value for Retry-After header field (seconds unit)
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0) = 0;

public:
    /// States of ICapabilities
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    /// Optional flags for forming SIP message such as Contact header, SDP body parts, and so on.
    enum
    {
        FLAG_NONE = 0,
        /// Flag indicating whether or not a Contact header should be added.
        FLAG_ADD_CONTACT_HEADER = 1 << 0,
        /// Flag indicating whether or not a Contact header should contain all the feature tags
        /// which were used for the same IMS registration.
        /// This is available when FLAG_ADD_CONTACT_HEADER is set.
        FLAG_ADD_ALL_REGISTERED_FEATURES_IN_CONTACT_HEADER = 1 << 1,
        /// Flag indicating whether or not a SDP body part should be added.
        FLAG_ADD_SDP_BODY_PART = 1 << 2,
        /// Flag indicating whether or not the media capabilities should be checked
        /// while forming a SDP body part.
        /// This is available when FLAG_ADD_SDP_BODY_PART is set.
        FLAG_CHECK_MEDIA_CAPABILITIES = 1 << 3,

        FLAG_REQUEST_DEFAULT = FLAG_ADD_CONTACT_HEADER,
        FLAG_RESPONSE_DEFAULT =
                FLAG_ADD_CONTACT_HEADER | FLAG_ADD_ALL_REGISTERED_FEATURES_IN_CONTACT_HEADER,
    };
};

#endif
