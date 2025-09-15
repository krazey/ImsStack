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
#ifndef SDP_INFORMATION_H_
#define SDP_INFORMATION_H_

#include "SdpLine.h"

class SdpInformation : public SdpLine
{
public:
    SdpInformation();
    SdpInformation(IN const SdpInformation& other);
    ~SdpInformation() override;

public:
    SdpInformation& operator=(IN const SdpInformation& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the information line ("i=") in the session description.
     *        The strValue contains a full information line without "i=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the information line ("a=") in the session description.
     *        The returned value contains a full information line with "i=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full information line without "i=".
     */
    inline AString GetValue() const override { return m_strInformation; }

private:
    AString m_strInformation;
};

#endif
