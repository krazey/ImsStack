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
#ifndef IP_ADDRESS_H_
#define IP_ADDRESS_H_

#include "ByteArray.h"

#define IPAddress IpAddress

class IpAddressPrivate;

class Ipv6Address
{
public:
    Ipv6Address();
    Ipv6Address(IN const Ipv6Address& other);

    Ipv6Address& operator=(IN const Ipv6Address& other);

public:
    IMS_BYTE& operator[](IN IMS_SINT32 i);
    IMS_BYTE operator[](IN IMS_SINT32 i) const;
    const IMS_BYTE* GetAddress() const;
    ByteArray ToNetworkByteOrder() const;

private:
    static IMS_BOOL IsValidIndex(IN IMS_SINT32 nIndex);

public:
    enum
    {
        MAX_SIZE = 16
    };

private:
    IMS_BYTE aIp6[MAX_SIZE];
};

class IpAddress
{
public:
    IpAddress();
    explicit IpAddress(IN IMS_UINT32 nAddress);
    explicit IpAddress(IN const IMS_BYTE* pAddress);
    explicit IpAddress(IN const Ipv6Address& objAddress);
    explicit IpAddress(IN const AString& strAddress);
    IpAddress(IN const IpAddress& other);
    ~IpAddress();

public:
    IpAddress& operator=(IN const IpAddress& other);

public:
    IMS_BOOL Equals(IN const IpAddress& other) const;
    IMS_SINT32 GetVersion() const;
    inline IMS_BOOL IsIPv4Address() const { return IsIpv4Address(); }
    inline IMS_BOOL IsIPv6Address() const { return IsIpv6Address(); }
    IMS_BOOL IsIpv4Address() const;
    IMS_BOOL IsIpv6Address() const;
    IMS_BOOL IsUnknownAddress() const;
    IMS_BOOL IsAnyAddress() const;
    IMS_BOOL IsLoopbackAddress() const;
    IMS_BOOL IsMulticastAddress() const;
    IMS_BOOL IsNoneAddress() const;
    IMS_BOOL Parse(IN const AString& strAddress);

    inline IMS_UINT32 ToIPv4Address() const { return ToIpv4Address(); }
    inline Ipv6Address ToIPv6Address() const { return ToIpv6Address(); }
    IMS_UINT32 ToIpv4Address() const;
    Ipv6Address ToIpv6Address() const;
    AString ToString() const;
    const IMS_CHAR* ToCharString() const;

    static IMS_UINT16 HToNS(IN IMS_UINT16 nHost);
    static IMS_UINT32 HToNL(IN IMS_UINT32 nHost);
    static IMS_UINT16 NToHS(IN IMS_UINT16 nNetwork);
    static IMS_UINT32 NToHL(IN IMS_UINT32 nNetwork);

public:
    enum
    {
        UNKNOWN = 0,
        IPV4 = 4,
        IPV6 = 6
    };

    static const IpAddress ANY;
    static const IpAddress BROADCAST;
    static const IpAddress LOOPBACK;
    static const IpAddress NONE;

    static const IpAddress IPv6ANY;
    static const IpAddress IPv6LOOPBACK;
    static const IpAddress IPv6NONE;

private:
    IpAddressPrivate* m_pIpaPrivate;
};

inline IMS_BOOL operator==(IN const IpAddress& objA1, IN const IpAddress& objA2)
{
    return objA1.Equals(objA2);
}

inline IMS_BOOL operator!=(IN const IpAddress& objA1, IN const IpAddress& objA2)
{
    return !objA1.Equals(objA2);
}

#endif
