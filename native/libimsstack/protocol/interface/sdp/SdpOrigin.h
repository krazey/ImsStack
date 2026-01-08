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
#ifndef SDP_ORIGIN_H_
#define SDP_ORIGIN_H_

#include "SdpLine.h"

class SdpOrigin : public SdpLine
{
public:
    SdpOrigin();
    SdpOrigin(IN const SdpOrigin& other);
    ~SdpOrigin() override;

public:
    SdpOrigin& operator=(IN const SdpOrigin& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the origin line ("o=") in the session description.
     *        The strValue contains a full origin line without "o=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the origin line ("o=") in the session description.
     *        The returned value contains a full origin line with "o=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full origin line without "o=".
     */
    AString GetValue() const override;

    /**
     * @brief Returns the user name from the origin field.
     */
    inline const AString& GetUsername() const { return m_strUsername; }

    /**
     * @brief Returns the session id from the origin field.
     */
    inline const AString& GetSessionId() const { return m_strSessionId; }

    /**
     * @brief Returns the session version from the origin field.
     */
    inline const AString& GetSessionVersion() const { return m_strSessionVersion; }

    /**
     * @brief Returns the address type from the origin field.
     *
     * @return The address type as predefined enum type.\n
     *         #Sdp#ADDR_TYPE_IP4\n
     *         #Sdp#ADDR_TYPE_IP6
     */
    inline IMS_SINT32 GetAddressType() const { return m_nAddrType; }

    /**
     * @brief Returns the address type as a string value from the origin field.
     *
     * @return The address type as string.\n
     *         "IP4"\n
     *         "IP6"
     */
    inline const AString& GetAddressTypeToString() const { return m_strAddrType; }

    /**
     * @brief Returns the address as a string value from the origin field.
     */
    inline const AString& GetAddress() const { return m_strUnicastAddress; }

    /**
     * @brief Increases the session version.
     */
    void IncreaseSessionVersion();

    /**
     * @brief Sets the unicast address.
     */
    IMS_BOOL SetAddress(IN const AString& strAddress);

    /**
     * @brief Sets the user name & unicast address.
     */
    IMS_BOOL SetValue(IN const AString& strUsername, IN const AString& strAddress);

    /**
     * @brief Sets all the origin parameters to the current origin field.
     */
    IMS_BOOL SetValue(IN const AString& strUsername, IN const AString& strSessionId,
            IN const AString& strSessionVersion, IN IMS_SINT32 nAddrType,
            IN const AString& strAddress,
            IN const AString& strOtherAddrType = AString::ConstNull());

private:
    IMS_BOOL IsValid() const;

    /**
     * @brief Returns a NTP time value.
     */
    static IMS_UINT32 GetNtpTime();

public:
    static const IMS_CHAR DEFAULT_USERNAME[];

private:
    // 1900/01/01 ~ before 1970/01/01, 2208988800
    enum
    {
        NTP_OFFSET = 2208988800UL
    };

    static IMS_UINT32 s_nLastTime;

    // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    AString m_strUsername;
    AString m_strSessionId;
    AString m_strSessionVersion;
    IMS_SINT32 m_nNetType;
    AString m_strNetType;  // For IMS service, "IN" only
    IMS_SINT32 m_nAddrType;
    AString m_strAddrType;
    AString m_strUnicastAddress;
};

#endif
