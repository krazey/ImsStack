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
#ifndef INTERFACE_IMS_SUBSCRIBER_INFO_H_
#define INTERFACE_IMS_SUBSCRIBER_INFO_H_

#include "AStringArray.h"
#include "Credential.h"

class IImsSubscriberInfo
{
protected:
    virtual ~IImsSubscriberInfo() = default;

public:
    /**
     * @brief Returns the credential of an IMS subscriber.
     *
     * @return The credential information of an IMS subscriber.
     */
    virtual const Credential& GetCredential() const = 0;

    /**
     * @brief Returns the home domain name of an IMS subscriber.
     *
     * @return The home domain name of an IMS subscriber.
     */
    virtual const AString& GetHomeDomainName() const = 0;

    /**
     * @brief Returns the reference index of a primary public user identity.
     *
     * @return A reference index to identify a primary public user identity.
     */
    virtual IMS_SINT32 GetIndexOfPrimaryPublicUserId() const = 0;

    /**
     * @brief Returns the domain name to be used in the phone-context parameter.
     *
     * @return The domain name for phone-context parameter.
     */
    virtual const AString& GetPhoneContext() const = 0;

    /**
     * @brief Returns the private user identity of an IMS subscriber.
     *
     * @return A private user identity.
     */
    virtual const AString& GetPrivateUserId() const = 0;

    /**
     * @brief Returns the public user identity of an IMS subscriber at the specified index.
     *
     * @param nImpuType The IMPU type to be retrieved.
     *                  #IMPU_REF_INDEX\n
     *                  #IMPU_SIP\n
     *                  #IMPU_TEL
     * @return A public user identity.
     */
    virtual const AString& GetPublicUserId(
            IN IMS_SINT32 nImpuType = IImsSubscriberInfo::IMPU_REF_INDEX) const = 0;

    /**
     * @brief Returns all the public user identities of an IMS subscriber.
     *
     * @return List of public user identity.
     */
    virtual const AStringArray& GetPublicUserIds() const = 0;

public:
    /// Type of public user identity to be retrieved
    enum
    {
        /// Public user identity from the primary reference index
        IMPU_REF_INDEX = 0,
        /// Public user identity with SIP URI format (first SIP URI among public user identities)
        IMPU_SIP,
        /// Public user identity with TEL URI format (first TEL URI among public user identities)
        IMPU_TEL
    };
};

#endif
