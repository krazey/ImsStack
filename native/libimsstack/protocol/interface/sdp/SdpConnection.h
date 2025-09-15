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
#ifndef SDP_CONNECTION_H_
#define SDP_CONNECTION_H_

#include "SdpLine.h"

class SdpConnection : public SdpLine
{
public:
    SdpConnection();
    SdpConnection(IN const SdpConnection& other);
    ~SdpConnection() override;

public:
    SdpConnection& operator=(IN const SdpConnection& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the connection line ("c=") in the session description.
     *        The strValue contains a full connection line without "c=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the connection line ("c=") in the session description.
     *        The returned value contains a full connection line with "c=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full connection line without "c=".
     */
    AString GetValue() const override;

    /**
     * @brief Returns the address type in the connection line.
     *
     * @return The address type. "IP4" or "IP6".
     */
    inline const AString& GetAddressType() const { return m_strAddrType; }

    /**
     * @brief Returns the address in the connection line.
     */
    inline const AString& GetAddress() const { return m_strAddress; }

    // In case of multicase address, only IPv4
    /**
     * @brief Returns the address in the connection line.
     */
    ImsList<AString> GetAddresses() const;

    /**
     * @brief Sets the connection parameters.
     */
    IMS_BOOL SetValue(IN IMS_SINT32 nAddrType, IN const AString& strAddress,
            IN const AString& strOtherAddrType = AString::ConstNull(), IN IMS_SINT32 nTtl = 0,
            IN IMS_SINT32 nNumOfAddress = 0);

private:
    IMS_BOOL IsValid() const;

private:
    // c=<nettype> <addrtype> <connection-address>
    IMS_SINT32 m_nNetType;
    AString m_strNetType;  // For IMS service, "IN" only
    IMS_SINT32 m_nAddrType;
    AString m_strAddrType;
    AString m_strAddress;

    // In case of multicast address
    IMS_SINT32 m_nTtl;
    IMS_SINT32 m_nNumOfAddress;
};

#endif
