/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _IP_ADDRESS_H_
#define _IP_ADDRESS_H_

#include "ByteArray.h"

class IPAddressPrivate;

class IPv6Address
{
public:
    IMS_BYTE& operator[](IN IMS_SINT32 i);
    IMS_BYTE operator[](IN IMS_SINT32 i) const;
    const IMS_BYTE* GetAddress() const;
    ByteArray ToNetworkByteOrder() const;

public:
    enum
    {
        MAX_SIZE = 16
    };

private:
    IMS_BYTE aIP6[MAX_SIZE];
};

class IPAddress
{
public:
    IPAddress();
    explicit IPAddress(IN IMS_UINT32 nIP_);
    explicit IPAddress(IN CONST IMS_BYTE* pIP_);
    explicit IPAddress(IN CONST IPv6Address& objIP_);
    explicit IPAddress(IN CONST AString& strIP_);
    IPAddress(IN CONST IPAddress& objRHS);
    ~IPAddress();

public:
    IPAddress& operator=(IN CONST IPAddress& objRHS);

public:
    IMS_BOOL Equals(IN CONST IPAddress& objIPA) const;
    IMS_SINT32 GetVersion() const;
    IMS_BOOL IsIPv4Address() const;
    IMS_BOOL IsIPv6Address() const;
    IMS_BOOL IsUnknownAddress() const;
    IMS_BOOL IsAnyAddress() const;
    IMS_BOOL IsLoopbackAddress() const;
    IMS_BOOL IsMulticastAddress() const;
    IMS_BOOL IsNoneAddress() const;
    IMS_BOOL Parse(IN CONST AString& strIP_);

    IMS_UINT32 ToIPv4Address() const;
    IPv6Address ToIPv6Address() const;
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

    static const IPAddress ANY;
    static const IPAddress BROADCAST;
    static const IPAddress LOOPBACK;
    static const IPAddress NONE;

    static const IPAddress IPv6ANY;
    static const IPAddress IPv6LOOPBACK;
    static const IPAddress IPv6NONE;

private:
    IPAddressPrivate* pIPA;
};

inline IMS_BOOL operator==(IN CONST IPAddress& objA1, IN CONST IPAddress& objA2)
{
    return objA1.Equals(objA2);
}

inline IMS_BOOL operator!=(IN CONST IPAddress& objA1, IN CONST IPAddress& objA2)
{
    return !objA1.Equals(objA2);
}

#endif  // _IP_ADDRESS_H_
