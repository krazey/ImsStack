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
#ifndef SDP_LINE_H_
#define SDP_LINE_H_

#include "ImsTypeDef.h"

class AString;

class SdpLine
{
public:
    SdpLine() = default;
    SdpLine(IN const SdpLine& other) = default;
    virtual ~SdpLine() = 0;

public:
    SdpLine& operator=(IN const SdpLine& other) = default;

public:
    /**
     * @brief Decodes the SDP line in the session description.
     */
    virtual IMS_BOOL Decode(IN const AString& strValue);

    /**
     * @brief Encodes the SDP line (with the prefix; "a=", "b=", "i=", ...)
     *        in the session description.
     */
    virtual AString Encode() const;

    /**
     * @brief Returns the full SDP line (without the prefix; "a=", "b=", "i=", ...)
     *        in the session description.
     */
    virtual AString GetValue() const;

protected:
    /**
     * @brief Check if the specified address format is valid.
     */
    static IMS_BOOL CheckValidityForAddress(IN const AString& strAddress, IN IMS_SINT32 nAddrType);
};

#endif
