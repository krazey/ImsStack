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
#ifndef SDP_VERSION_H_
#define SDP_VERSION_H_

#include "SdpLine.h"

class SdpVersion : public SdpLine
{
public:
    SdpVersion();
    SdpVersion(IN const SdpVersion& other);
    ~SdpVersion() override;

public:
    SdpVersion& operator=(IN const SdpVersion& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the version line ("v=") in the session description.
     *        The strValue contains a full version line without "v=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the version line ("v=") in the session description.
     *        The returned value contains a full version line with "v=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full version line without "v=".
     */
    AString GetValue() const override;

    inline IMS_SINT32 GetVersion() const { return m_nVersion; }
    IMS_BOOL SetVersion(IN IMS_SINT32 nVersion = SDP_VERSION);

public:
    enum
    {
        SDP_VERSION = 0
    };

private:
    // v=<version>
    IMS_SINT32 m_nVersion;
};

#endif
