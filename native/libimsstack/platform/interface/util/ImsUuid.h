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
#ifndef IMS_UUID_H_
#define IMS_UUID_H_

#include "AString.h"

class ImsUuid
{
public:
    enum
    {
        /// Time-based version (based on platform)
        VERSION_1 = 1,
        /// Name-based version (MD5 hashing)
        VERSION_3 = 3,
        /// Randomly or pseudo-randomly generated version
        VERSION_4 = 4
    };

public:
    ImsUuid() = delete;

public:
    /**
     * @brief Returns the generated UUID string based on the specified version and name.
     *
     * @param nVersion The UUID type
     * @param strName The name to be used to construct UUID string.
     *                This is a mandatory field for version 3.
     * @return A generated UUID string.
     */
    static AString GetUuid(
            IN IMS_SINT32 nVersion, IN const AString& strName = AString::ConstNull());
};

#endif
