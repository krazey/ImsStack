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
#ifndef SDP_ENCRYPTION_KEY_H_
#define SDP_ENCRYPTION_KEY_H_

#include "SdpLine.h"

class SdpEncryptionKey : public SdpLine
{
public:
    SdpEncryptionKey();
    SdpEncryptionKey(IN const SdpEncryptionKey& other);
    ~SdpEncryptionKey() override;

public:
    SdpEncryptionKey& operator=(IN const SdpEncryptionKey& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the encryption key line in the session description.
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the encryption key line with "k=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full encryption key line without "k=".
     */
    AString GetValue() const override;

    /**
     * @brief Returns the method which uses in the encryption.
     */
    inline IMS_SINT32 GetMethod() const { return m_nMethod; }

    /**
     * @brief Returns the key which uses in the encryption.
     */
    inline const AString& GetKey() const { return m_strKey; }

    /**
     * @brief Sets the parameters for the encryption key.
     */
    IMS_BOOL SetValue(IN IMS_SINT32 nMethod, IN const AString& strKey = AString::ConstNull());

public:
    enum
    {
        METHOD_INVALID = (-1),
        METHOD_PROMPT,  // no value
        METHOD_CLEAR,   // text
        METHOD_BASE64,  // base64
        METHOD_URI,     // uri format
        METHOD_MAX
    };

private:
    static const IMS_CHAR* METHOD[METHOD_MAX];

    IMS_SINT32 m_nMethod;
    AString m_strKey;
};

#endif
