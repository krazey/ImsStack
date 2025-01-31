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

#ifndef INTERFACE_SILENT_REDIAL_HELPER_H_
#define INTERFACE_SILENT_REDIAL_HELPER_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"

class ISilentRedialHelper
{
public:
    virtual ~ISilentRedialHelper() {}

    /**
     * @brief Redials the previous call.
     *
     * @param nIntervalInMillis The interval in milliseconds to wait before redialing.
     *                          If set to `INTERVAL_BY_TYPE`, the interval is determined by the
     *                          redial type.
     *                          Defaults to `INTERVAL_BY_TYPE`.
     * @return The CallReasonInfo containing the result of the redial attempt.
     *         If the `nCode` field of the returned object is `CallReasonInfo.CODE_NONE`, the redial
     *         was successful.
     *         Otherwise, the `nCode` field indicates the reason for failure.
     */
    virtual CallReasonInfo Redial(IN IMS_SINT32 nIntervalInMillis = INTERVAL_BY_TYPE) = 0;

    /**
     * Gets the type of redial.
     *
     * This method retrieves the type of redial used by the implementation.
     * This information is intended for unit testing purposes only and should not be used in
     * production code.
     *
     * @return An integer value representing the redial type.
     */
    virtual IMS_UINT32 GetType() = 0;

    static const IMS_SINT32 INTERVAL_BY_TYPE = -1;
};

#endif
