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
#ifndef SDP_SESSION_NAME_H_
#define SDP_SESSION_NAME_H_

#include "SdpLine.h"

class SdpSessionName : public SdpLine
{
public:
    SdpSessionName();
    SdpSessionName(IN const SdpSessionName& other);
    ~SdpSessionName() override;

public:
    SdpSessionName& operator=(IN const SdpSessionName& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the session-name line ("s=") in the session description.
     *        The strValue contains a full session-name line without "s=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the session-name line ("s=") in the session description.
     *        The returned value contains a full session-name line with "s=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full session-name line without "s=".
     */
    inline AString GetValue() const override { return m_strName; }

private:
    AString m_strName;
};

#endif
