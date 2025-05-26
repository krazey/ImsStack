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

#ifndef INTERFACE_MTS_ERROR_HANDLER_H_
#define INTERFACE_MTS_ERROR_HANDLER_H_

#include "ImsTypeDef.h"

class IMessage;
class IMtsDynamicLoader;
class IMtsService;

class IMtsErrorHandler
{
public:
    virtual ~IMtsErrorHandler() {}

    /**
     * @brief Handles an error encountered during SIP transaction.
     *
     * This method processes the error and takes appropriate actions, such as
     * attempting recovery and handling retry mechanisms.
     *
     * @param objMtsService A reference to the IMtsService object.
     * @param objMtsDynamicLoader A reference to the IMtsDynamicLoader object.
     * @param piMessage A pointer to the IMessage object associated with the error, if available.
     * @param nMti The Message Type of the failed MO SMS.
     *
     * @return A status code indicating the outcome of the error handling process.
     */
    virtual IMS_SINT32 Handle(IN const IMtsService& objMtsService,
            IN const IMtsDynamicLoader& objMtsDynamicLoader,
            IN const IMessage* piMessage = IMS_NULL,
            IMS_SINT32 nMti = SMS_3GPP_MTI_RP_DATA_FROM_MS) = 0;

    /**
     * @brief Retrieves the Retry-After value.
     *
     * This method returns the currently stored Retry-After value,
     * which is typically obtained from a Retry-After header in a SIP response.
     *
     * @return The Retry-After value in seconds.
     */
    virtual IMS_SINT32 GetRetryAfterValue() const = 0;

    /**
     * @brief Resets the Retry-After value and related member variables.
     *
     */
    virtual void ResetRetryAfterStatus() = 0;
};

#endif
