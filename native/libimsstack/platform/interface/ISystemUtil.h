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
#ifndef INTERFACE_SYSTEM_UTIL_H_
#define INTERFACE_SYSTEM_UTIL_H_

#include "AString.h"

class ISystemUtil
{
protected:
    virtual ~ISystemUtil() = default;

public:
    /**
     * @brief Gets Digest result using SHA1 algorithm.
     *
     * @param strIn The raw string to be hashed
     * @param strOut The digest result using SHA1 algorithm
     */
    virtual void DigestSha1(IN const AString& strIn, OUT AString& strOut) = 0;

    /**
     * @brief Gets the time or random based UUID (version 1).
     *        The time-based UUID will be prioritized.
     *
     * @param nOption The options for UUID generation; it's not used in the moment.
     *                The default is a time-based UUID.
     * @return A time or random based UUID string.
     */
    virtual AString GetUuid(IN IMS_SINT32 nOption = 0) = 0;
};

#endif
