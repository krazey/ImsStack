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
#ifndef INTERFACE_REG_INFO_CONTACT_H_
#define INTERFACE_REG_INFO_CONTACT_H_

#include "IMSMap.h"

#include "SipAddress.h"

/**
 * @brief This class provides an interface to access the contact element
 *        of the "reginfo" XML document.
 */
class IRegInfoContact
{
public:
    /**
     * @brief Returns the 'cseq' information of contact.
     *
     * @return CSeq (command sequence) number.
     */
    virtual IMS_UINT32 GetCSeq() const = 0;

    /**
     * @brief Returns the display name of contact.
     *
     * @return Display name of contact.
     */
    virtual const AString& GetDisplayName() const = 0;

    /**
     * @brief Returns the event of contact.
     *
     * @return Event value.\n
     *         #EVENT_REGISTERED\n
     *         #EVENT_CREATED\n
     *         #EVENT_REFRESHED\n
     *         #EVENT_SHORTENED\n
     *         #EVENT_EXPIRED\n
     *         #EVENT_DEACTIVATED\n
     *         #EVENT_PROBATION\n
     *         #EVENT_UNREGISTERED\n
     *         #EVENT_REJECTED
     */
    virtual IMS_SINT32 GetEvent() const = 0;

    /**
     * @brief Returns the expiration value of contact.
     *
     * @return Expires value.
     */
    virtual IMS_UINT32 GetExpiresValue() const = 0;

    /**
     * @brief Returns the 'first-cseq' information of contact.
     *
     * @return First CSeq(command sequence) number.
     */
    virtual IMS_UINT32 GetFirstCSeq() const = 0;

    /**
     * @brief Returns the public GRUU of contact.
     *
     * @return Public GRUU of contact.
     */
    virtual const AString& GetPublicGruu() const = 0;

    /**
     * @brief Returns the temporary GRUU of contact.
     *
     * @return Temporary GRUU of contact.
     */
    virtual const AString& GetTemporaryGruu() const = 0;

    /**
     * @brief Returns the q-value of contact.
     *
     * @return Q-Value.
     */
    virtual const AString& GetQValue() const = 0;

    /**
     * @brief Returns the retry-after of contact.
     *
     * @return Retry-After value.
     */
    virtual IMS_UINT32 GetRetryAfterValue() const = 0;

    /**
     * @brief Returns the state of contact.
     *
     * @return State of contact.\n
     *         #STATE_CREATED\n
     *         #STATE_ACTIVE\n
     *         #STATE_TERMINATED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Returns the specified unknown parameter of contact.
     *
     * @param strName the name to be retrieved
     * @return Value of the specified parameter name\n
     *         - AString#ConstNull : parameter does not exist
     *         - AString#ConstEmepty : parameter exists, but empty value (ex. feature-tag, ...)
     */
    virtual const AString& GetUnknownParameter(IN const AString& strName) const = 0;

    /**
     * @brief Returns the unknown parameters of contact.
     *
     * @return Unknown parameters of contact - list of <key,value> pair.
     */
    virtual const IMSMap<AString, AString>& GetUnknownParameters() const = 0;

    /**
     * @brief Returns the URI of contact.
     *
     * @return URI of contact.
     */
    virtual const SipAddress& GetUri() const = 0;

public:
    /// State of Contact
    enum
    {
        STATE_CREATED = 0,
        /// When there is at least one contact bound to the AOR(address-of-record)
        STATE_ACTIVE = 1,
        /// When the last contact expires or is removed
        STATE_TERMINATED = 2
    };

    /// Event which caused the contact state machine to go into its current state
    enum
    {
        /// When the contact is successfully registered
        EVENT_REGISTERED,
        /// When the contact is just created
        EVENT_CREATED,
        /// When the contact is refreshed by re-registration
        EVENT_REFRESHED,
        /// When the contact decreases the refresh interval for explicit re-authorization
        EVENT_SHORTENED,
        /// When the registration timer is expired; re-registration is not performed
        EVENT_EXPIRED,
        /// When the contact is deactivated by some network node's problem
        EVENT_DEACTIVATED,
        /// When the contact is termiated and it can an initial registration after some times
        EVENT_PROBATION,
        /// When the de-registration is performed
        EVENT_UNREGISTERED,
        /// When the contact is rejected due to change in authorization policy
        EVENT_REJECTED
    };
};

#endif
