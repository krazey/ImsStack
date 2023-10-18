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
#ifndef INTERFACE_REASON_INFO_H_
#define INTERFACE_REASON_INFO_H_

#include "AString.h"

/**
 * @brief This class provides an interface to enable an application to get details on
 *        why a method call failed.
 */
class IReasonInfo
{
protected:
    virtual ~IReasonInfo() = default;

public:
    /**
     * @brief Returns the IMS reason phrase.
     *
     * @return An IMS reason phrase.
     */
    virtual const AString& GetReasonPhrase() const = 0;

    /**
     * @brief Returns the IMS reason type.
     *
     * @return An IMS reason type.
     */
    virtual IMS_SINT32 GetReasonType() const = 0;

    /**
     * @brief Returns the IMS reason code.
     *
     * @return An IMS reason code.
     */
    virtual IMS_SINT32 GetStatusCode() const = 0;

public:
    /// Enum for reason types
    enum
    {
        /// Reason type is unspecified
        REASON_TYPE_NONE = (-1),
        /// Reason type from SIP response
        REASON_TYPE_RESPONSE = 0,
        /// Reason type from user's(application's) action
        REASON_TYPE_USER_ACTION = 1,
        /// Reason type from the local timer
        REASON_TYPE_LOCAL_TIMEOUT = 2,
        /// Reason type from inconsistent application registry (@ref IAppConfig)
        REASON_TYPE_INCONSISTENT_REGISTRY = 3
    };
};

#endif
