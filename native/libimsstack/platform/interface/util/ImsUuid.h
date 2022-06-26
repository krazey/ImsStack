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

#include "AStringBuffer.h"

class ImsUuid
{
public:
    enum
    {
        /// Time-based version (based on platform)
        VERSION_1,
        /// DCE security version with embedded POSIX UIDs - NOT IMPLEMENTED
        VERSION_2,
        /// Name-based version (MD5 hashing)
        VERSION_3,
        /// Randomly or pseudo-randomly generated version - NOT IMPLEMENTED
        VERSION_4,
        /// Name-based version (SHA-1 hashing) - NOT IMPLEMENTED
        VERSION_5
    };

public:
    ImsUuid() = delete;

public:
    static AString GetUuid(
            IN IMS_SINT32 nVersion = VERSION_4, IN const AString& strName = AString::ConstNull());

private:
    static void GetUuidv3(IN const AString& strName, OUT AStringBuffer& objUuidStr);
    static void GetUuidv4(IN const AString& strRandom, OUT AStringBuffer& objUuidStr);
    static void GetUuidv5(IN const AString& strName, OUT AStringBuffer& objUuidStr);
};

#endif
