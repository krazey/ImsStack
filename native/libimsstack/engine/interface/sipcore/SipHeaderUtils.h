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
#ifndef SIP_HEADER_UTILS_H_
#define SIP_HEADER_UTILS_H_

#include "ImsTypeDef.h"

/**
 * @brief This class defines the warning error codes and to generate the retry-after field value.
 */
class SipHeaderUtils
{
public:
    SipHeaderUtils() = delete;

public:
    /**
     * @brief Generates the retry-after header field as second.
     *
     * @param nExtent The value for modular operation; maximum interval value
     * @return A second value for Retry-After header field.
     */
    static IMS_SINT32 GenerateRetryAfterSeconds(IN IMS_SINT32 nExtent = 0);

public:
    /// Warning codes that provides information supplemental to the status code
    /// in SIP response messages when the failure of the transaction results from a SDP problem.
    enum
    {
        WC_INVALID = 0,

        /// Incompatible Network Protocol
        WC_300 = 300,
        /// Incompatible Network Address Formats
        WC_301,
        /// Incompatible Transport Protocol
        WC_302,
        /// Incompatible Bandwidth Units
        WC_303,
        /// Media Type Not Available
        WC_304,
        /// Incompatible Media Format
        WC_305,
        /// Attribute Not Understand
        WC_306,
        /// Session Description Parameter Not Understood
        WC_307,
        /// Multicast Not Available
        WC_330 = 330,
        /// Unicast Not Available
        WC_331,
        /// Insufficient Bandwidth
        WC_370 = 370,
        /// Miscellaneous Warning
        WC_399 = 399,

        WC_MAX
    };
};

#endif
