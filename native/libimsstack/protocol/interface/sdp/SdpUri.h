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
#ifndef SDP_URI_H_
#define SDP_URI_H_

#include "SdpLine.h"

class SdpUri : public SdpLine
{
public:
    SdpUri();
    SdpUri(IN const SdpUri& other);
    ~SdpUri() override;

public:
    SdpUri& operator=(IN const SdpUri& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the uri line ("u=") in the session description.
     *        The strValue contains a full uri line without "u=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the uri line ("u=") in the session description.
     *        The returned value contains a full uri line with "u=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full uri line without "u=".
     */
    inline AString GetValue() const override { return m_strUri; }

private:
    // u=<uri>
    AString m_strUri;
};

#endif
