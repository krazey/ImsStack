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
#include "AStringArray.h"
#include "AStringBuffer.h"
#include "ImsStrLib.h"
#include "IpAddress.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"

static IMS_BOOL ipAddress_ParseIpv4(IN const AString& strIp4, OUT IMS_UINT32* pnIp4)
{
    ImsList<AString> objIpv4 = strIp4.Split('.');

    if (objIpv4.GetSize() != 4)
    {
        (*pnIp4) = 0;
        return IMS_FALSE;
    }

    IMS_UINT32 nIp4 = 0;
    IMS_BOOL bOk;

    for (IMS_SINT32 i = 0; i < 4; ++i)
    {
        bOk = IMS_FALSE;

        const AString& strValue = objIpv4.GetAt(i);
        IMS_UINT32 nByteValue = strValue.ToUInt32(&bOk);

        if (!bOk || (nByteValue > 0xff))
        {
            (*pnIp4) = 0;
            return IMS_FALSE;
        }

        nIp4 <<= 8;
        nIp4 += nByteValue;
    }

    (*pnIp4) = nIp4;

    return IMS_TRUE;
}

static IMS_BOOL ipAddress_ParseIpv6(IN const AString& strIp6, OUT IMS_BYTE* pIp6)
{
    AStringArray objIpv6 = strIp6.Split(':');
    IMS_SINT32 nCount = objIpv6.GetCount();

    if ((nCount < 3) || (nCount > 8))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nMaxCount = Ipv6Address::MAX_SIZE;
    IMS_SINT32 nReducedCount = 9 - nCount;

    for (IMS_SINT32 i = nCount - 1; i >= 0; --i)
    {
        if (nMaxCount <= 0)
        {
            return IMS_FALSE;
        }

        const AString& strValue = objIpv6.GetElementAt(i);

        if (strValue.GetLength() == 0)
        {
            if (i == nCount - 1)
            {
                // Special case: ":" is last character
                const AString& strTmp = objIpv6.GetElementAt(i - 1);

                if (strTmp.GetLength() != 0)
                {
                    return IMS_FALSE;
                }

                pIp6[--nMaxCount] = 0;
                pIp6[--nMaxCount] = 0;
            }
            else if (i == 0)
            {
                // Special case: ":" is first character
                const AString& strTmp = objIpv6.GetElementAt(i + 1);

                if (strTmp.GetLength() != 0)
                {
                    return IMS_FALSE;
                }

                pIp6[--nMaxCount] = 0;
                pIp6[--nMaxCount] = 0;
            }
            else
            {
                for (IMS_SINT32 j = 0; j < nReducedCount; ++j)
                {
                    if (nMaxCount <= 0)
                    {
                        return IMS_FALSE;
                    }

                    pIp6[--nMaxCount] = 0;
                    pIp6[--nMaxCount] = 0;
                }
            }
        }
        else
        {
            IMS_BOOL bOk = IMS_FALSE;
            IMS_UINT32 nByteValue = strValue.ToUInt32(&bOk, 16);

            if (bOk && (nByteValue <= 0xffff))
            {
                pIp6[--nMaxCount] = nByteValue & 0xff;
                pIp6[--nMaxCount] = (nByteValue >> 8) & 0xff;
            }
            else
            {
                if (i != nCount - 1)
                {
                    return IMS_FALSE;
                }

                // Parse the IPv4 part of a mixed type
                IMS_UINT32 nIp4 = 0;

                if (!ipAddress_ParseIpv4(strValue, &nIp4))
                {
                    return IMS_FALSE;
                }

                pIp6[--nMaxCount] = nIp4 & 0xff;
                pIp6[--nMaxCount] = (nIp4 >> 8) & 0xff;
                pIp6[--nMaxCount] = (nIp4 >> 16) & 0xff;
                pIp6[--nMaxCount] = (nIp4 >> 24) & 0xff;
                --nReducedCount;
            }
        }
    }

    return IMS_TRUE;
}

#ifndef __IMS_IPV6_SHORT_FORM__

class GroupOfZeroes
{
public:
    inline GroupOfZeroes() :
            m_nStart(-1),
            m_nEnd(-1)
    {
    }

    inline GroupOfZeroes(IN const GroupOfZeroes& other) :
            m_nStart(other.m_nStart),
            m_nEnd(other.m_nEnd)
    {
    }

    inline ~GroupOfZeroes() {}

public:
    inline GroupOfZeroes& operator=(IN const GroupOfZeroes& other)
    {
        if (this != &other)
        {
            m_nStart = other.m_nStart;
            m_nEnd = other.m_nEnd;
        }

        return (*this);
    }

    // Gets the start / end index of group of zeroes
    inline IMS_SINT32 GetIndexS() const { return m_nStart; }
    inline IMS_SINT32 GetIndexE() const { return m_nEnd; }

    // Sets the start / end index of group of zeroes
    inline void SetIndexS(IN IMS_SINT32 nIndex) { m_nStart = nIndex; }
    inline void SetIndexE(IN IMS_SINT32 nIndex) { m_nEnd = nIndex; }

private:
    IMS_SINT32 m_nStart;
    IMS_SINT32 m_nEnd;
};

#endif  // __IMS_IPV6_SHORT_FORM__

class IpAddressPrivate
{
public:
    IpAddressPrivate();
    IpAddressPrivate(IN const IpAddressPrivate& other);

public:
    void SetAddress(IN IMS_UINT32 nAddress = 0);
    void SetAddress(IN const IMS_BYTE* pAddress);
    void SetAddress(IN const Ipv6Address& objAddress);

private:
    friend class IpAddress;

    enum
    {
        TYPE_UNKNOWN = 0,
        TYPE_IPv4,
        TYPE_IPv6
    };

    IMS_SINT32 m_nType;

    // IPv4 address
    IMS_UINT32 m_nA;
    // IPv6 address
    Ipv6Address m_objA6;
};

PUBLIC
IpAddressPrivate::IpAddressPrivate() :
        m_nType(TYPE_UNKNOWN),
        m_nA(0)
{
}

PUBLIC
IpAddressPrivate::IpAddressPrivate(IN const IpAddressPrivate& other) :
        m_nType(other.m_nType),
        m_nA(other.m_nA),
        m_objA6(other.m_objA6)
{
}

PUBLIC
void IpAddressPrivate::SetAddress(IN IMS_UINT32 nAddress /*= 0*/)
{
    m_nType = TYPE_IPv4;
    m_nA = nAddress;
}

PUBLIC
void IpAddressPrivate::SetAddress(IN const IMS_BYTE* pAddress)
{
    m_nType = TYPE_IPv6;

    for (IMS_SINT32 i = 0; i < Ipv6Address::MAX_SIZE; ++i)
    {
        m_objA6[i] = pAddress[i];
    }
}

PUBLIC
void IpAddressPrivate::SetAddress(IN const Ipv6Address& objAddress)
{
    m_nType = TYPE_IPv6;
    m_objA6 = objAddress;
}

PUBLIC
Ipv6Address::Ipv6Address()
{
    IMS_MEM_Memset(aIp6, 0x00, MAX_SIZE);
}

PUBLIC
Ipv6Address::Ipv6Address(IN const Ipv6Address& other)
{
    IMS_MEM_Memcpy(aIp6, other.aIp6, MAX_SIZE);
}

PUBLIC
Ipv6Address& Ipv6Address::operator=(IN const Ipv6Address& other)
{
    if (this != &other)
    {
        IMS_MEM_Memcpy(aIp6, other.aIp6, MAX_SIZE);
    }
    return (*this);
}

PUBLIC
IMS_BYTE& Ipv6Address::operator[](IN IMS_SINT32 i)
{
    IMS_ASSERT(IsValidIndex(i));
    return aIp6[i];
}

PUBLIC
IMS_BYTE Ipv6Address::operator[](IN IMS_SINT32 i) const
{
    IMS_ASSERT(IsValidIndex(i));
    return aIp6[i];
}

PUBLIC
const IMS_BYTE* Ipv6Address::GetAddress() const
{
    return &(aIp6[0]);
}

PUBLIC
ByteArray Ipv6Address::ToNetworkByteOrder() const
{
    ByteArray objIp6;

    for (IMS_SINT32 i = 0; i < MAX_SIZE; i += 2)
    {
        objIp6.Append(aIp6[i + 1]);
        objIp6.Append(aIp6[i]);
    }

    return objIp6;
}

PRIVATE GLOBAL IMS_BOOL Ipv6Address::IsValidIndex(IN IMS_SINT32 nIndex)
{
    return (nIndex >= 0) && (nIndex < MAX_SIZE);
}

const IpAddress IpAddress::ANY = IpAddress(AString("0.0.0.0"));
const IpAddress IpAddress::BROADCAST = IpAddress(AString("255.255.255.255"));
const IpAddress IpAddress::LOOPBACK = IpAddress(AString("127.0.0.1"));
const IpAddress IpAddress::NONE = IpAddress(AString("0.0.0.0"));
const IpAddress IpAddress::IPv6ANY = IpAddress(AString("::"));
const IpAddress IpAddress::IPv6LOOPBACK = IpAddress(AString("::1"));
const IpAddress IpAddress::IPv6NONE = IpAddress(AString("::"));

PUBLIC
IpAddress::IpAddress() :
        m_pIpaPrivate(new IpAddressPrivate())
{
}

PUBLIC
IpAddress::IpAddress(IN IMS_UINT32 nAddress) :
        m_pIpaPrivate(new IpAddressPrivate())
{
    m_pIpaPrivate->SetAddress(nAddress);
}

PUBLIC
IpAddress::IpAddress(IN const IMS_BYTE* pAddress) :
        m_pIpaPrivate(new IpAddressPrivate())
{
    m_pIpaPrivate->SetAddress(pAddress);
}

PUBLIC
IpAddress::IpAddress(IN const Ipv6Address& objAddress) :
        m_pIpaPrivate(new IpAddressPrivate())
{
    m_pIpaPrivate->SetAddress(objAddress);
}

PUBLIC
IpAddress::IpAddress(IN const AString& strAddress) :
        m_pIpaPrivate(new IpAddressPrivate())
{
    Parse(strAddress);
}

PUBLIC
IpAddress::IpAddress(IN const IpAddress& other) :
        m_pIpaPrivate(new IpAddressPrivate(*(other.m_pIpaPrivate)))
{
}

PUBLIC
IpAddress::~IpAddress()
{
    delete m_pIpaPrivate;
}

PUBLIC
IpAddress& IpAddress::operator=(IN const IpAddress& other)
{
    if (this != &other)
    {
        *m_pIpaPrivate = *(other.m_pIpaPrivate);
    }

    return (*this);
}

PUBLIC
IMS_BOOL IpAddress::Equals(IN const IpAddress& other) const
{
    if (m_pIpaPrivate->m_nType != other.m_pIpaPrivate->m_nType)
    {
        return IMS_FALSE;
    }

    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        return (m_pIpaPrivate->m_nA == other.m_pIpaPrivate->m_nA);
    }
    else if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        return IMS_MEM_Memcmp(&m_pIpaPrivate->m_objA6, &(other.m_pIpaPrivate->m_objA6),
                       sizeof(Ipv6Address)) == 0;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 IpAddress::GetVersion() const
{
    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        return IPV6;
    }
    else if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        return IPV4;
    }

    return UNKNOWN;
}

PUBLIC
IMS_BOOL IpAddress::IsIpv4Address() const
{
    return (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4);
}

PUBLIC
IMS_BOOL IpAddress::IsIpv6Address() const
{
    return (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6);
}

PUBLIC
IMS_BOOL IpAddress::IsUnknownAddress() const
{
    return (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_UNKNOWN);
}

PUBLIC
IMS_BOOL IpAddress::IsAnyAddress() const
{
    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        return Equals(ANY);
    }
    else if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        return Equals(IPv6ANY);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IpAddress::IsLoopbackAddress() const
{
    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        return Equals(LOOPBACK);
    }
    else if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        return Equals(IPv6LOOPBACK);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IpAddress::IsMulticastAddress() const
{
    // IPv4 multicast : 224.0.0.0 ~ 239.255.255.255
    // IPv6 ???

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL IpAddress::IsNoneAddress() const
{
    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        return Equals(NONE);
    }
    else if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        return Equals(IPv6NONE);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IpAddress::Parse(IN const AString& strAddress)
{
    m_pIpaPrivate->m_nType = IpAddressPrivate::TYPE_UNKNOWN;

    AString strIp = strAddress.Trim();

    // All IPv6 addresses contain a ':', and may contain a '.'
    if (strIp.Contains(':'))
    {
        IMS_BYTE aIp6[Ipv6Address::MAX_SIZE];

        // If IPv6 address includes '[' & ']', then remove this character.
        if (strIp.StartsWith('[') && strIp.EndsWith(']'))
        {
            strIp = strIp.GetSubStr(1, strIp.GetLength() - 2);
        }

        if (ipAddress_ParseIpv6(strIp, aIp6))
        {
            m_pIpaPrivate->m_nType = IpAddressPrivate::TYPE_IPv6;
            m_pIpaPrivate->SetAddress(aIp6);

            return IMS_TRUE;
        }
    }

    // All IPv4 addresses contain a '.'
    if (strIp.Contains('.'))
    {
        IMS_UINT32 nIp4 = 0;

        if (ipAddress_ParseIpv4(strIp, &nIp4))
        {
            m_pIpaPrivate->m_nType = IpAddressPrivate::TYPE_IPv4;
            m_pIpaPrivate->SetAddress(nIp4);

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_UINT32 IpAddress::ToIpv4Address() const
{
    return m_pIpaPrivate->m_nA;
}

PUBLIC
Ipv6Address IpAddress::ToIpv6Address() const
{
    return m_pIpaPrivate->m_objA6;
}

PUBLIC
AString IpAddress::ToString() const
{
    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        IMS_UINT32 nA = ToIpv4Address();
        AString strIp;

        strIp.Sprintf(
                "%d.%d.%d.%d", (nA >> 24) & 0xff, (nA >> 16) & 0xff, (nA >> 8) & 0xff, nA & 0xff);

        return strIp;
    }

    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        IMS_UINT16 aTmp[8];

        for (IMS_SINT32 i = 0; i < 8; ++i)
        {
            aTmp[i] = (IMS_UINT16(m_pIpaPrivate->m_objA6[2 * i] << 8)) |
                    (IMS_UINT16(m_pIpaPrivate->m_objA6[2 * i + 1]));
        }

#ifndef __IMS_IPV6_SHORT_FORM__
        IMS_BOOL bGoZStarted = IMS_FALSE;
        GroupOfZeroes objGoZ;
        ImsList<GroupOfZeroes> objGoZs;

        for (IMS_SINT32 j = 0; j < 8; ++j)
        {
            if ((aTmp[j] == 0) && (!bGoZStarted))
            {
                objGoZ.SetIndexS(j);
                objGoZ.SetIndexE(j);
                bGoZStarted = IMS_TRUE;
            }

            if (bGoZStarted && aTmp[j] != 0)
            {
                objGoZ.SetIndexE(j - 1);
                objGoZs.Append(objGoZ);
                bGoZStarted = IMS_FALSE;
            }
        }

        // If the address ends with the group of zeroes, then it needs to be added to the list.
        if (bGoZStarted)
        {
            objGoZ.SetIndexE(7);
            objGoZs.Append(objGoZ);
        }

        if (objGoZs.IsEmpty())
        {
            AString strIp;
            strIp.Sprintf("%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2], aTmp[3], aTmp[4],
                    aTmp[5], aTmp[6], aTmp[7]);

            return strIp;
        }
        else
        {
            AStringBuffer objIp(50);

            // RFC 5952, 4.2.3.  Choice in Placement of "::"
            // When the length of the consecutive 16-bit 0 fields are equal
            // (i.e., 2001:db8:0:0:1:0:0:1), the first sequence of zero bits MUST be shortened.
            // For example, 2001:db8::1:0:0:1 is correct representation.
            objGoZ = objGoZs.GetAt(0);

            // Find the largest group of zeroes
            for (IMS_UINT32 i = 0; i < objGoZs.GetSize(); ++i)
            {
                const GroupOfZeroes& objTmpGoZ = objGoZs.GetAt(i);

                if ((objGoZ.GetIndexE() - objGoZ.GetIndexS()) <
                        (objTmpGoZ.GetIndexE() - objTmpGoZ.GetIndexS()))
                {
                    objGoZ = objTmpGoZ;
                }
            }

            AString strTmp;

            if (objGoZ.GetIndexS() == 0)
            {
                // Starts with the group of zeroes
                objIp.Append((objGoZ.GetIndexE() == 0) ? '0' : ':');
            }
            else
            {
                for (IMS_SINT32 i = 0; i < objGoZ.GetIndexS(); ++i)
                {
                    strTmp.Sprintf("%x:", aTmp[i]);
                    objIp.Append(strTmp);
                }
            }

            // RFC 5952, 4.2.2.  Handling One 16-Bit 0 Field
            // The symbol "::" MUST NOT be used to shorten just one 16-bit 0 field.
            // For example, the representation 2001:db8:0:1:1:1:1:1 is correct, but
            // 2001:db8::1:1:1:1:1 is not correct.
            if ((objGoZ.GetIndexS() == objGoZ.GetIndexE()) && (objGoZ.GetIndexS() != 0) &&
                    (objGoZ.GetIndexS() != 7))
            {
                objIp.Append('0');
            }

            if (objGoZ.GetIndexE() == 7)
            {
                // Ends with the group of zeroes
                objIp.Append((objGoZ.GetIndexS() == 7) ? '0' : ':');
            }
            else
            {
                for (IMS_SINT32 i = (objGoZ.GetIndexE() + 1); i < 8; ++i)
                {
                    strTmp.Sprintf(":%x", aTmp[i]);
                    objIp.Append(strTmp);
                }
            }

            return static_cast<const AStringBuffer&>(objIp).GetString();
        }
#else
        AString strIp;
        strIp.Sprintf("%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2], aTmp[3], aTmp[4],
                aTmp[5], aTmp[6], aTmp[7]);

        return strIp;
#endif  // __IMS_IPV6_SHORT_FORM__
    }

    return AString::ConstNull();
}

PUBLIC
const IMS_CHAR* IpAddress::ToCharString() const
{
    const IMS_UINT32 MAX_SIZE = 50;
    static IMS_CHAR acIp_0[MAX_SIZE + 1] = {
            0,
    };
    static IMS_CHAR acIp_1[MAX_SIZE + 1] = {
            0,
    };

    IMS_SINT32 nSlotId = SystemConfig::IsMultiSimEnabled()
            ? ThreadService::GetCurrentSlotId(IMS_SLOT_0)
            : IMS_SLOT_0;
    IMS_CHAR* acIp = (nSlotId == IMS_SLOT_0) ? acIp_0 : acIp_1;

    acIp[0] = '\0';

    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv4)
    {
        IMS_UINT32 nA = ToIpv4Address();

        IMS_Sprintf(acIp, MAX_SIZE, "%d.%d.%d.%d", (nA >> 24) & 0xff, (nA >> 16) & 0xff,
                (nA >> 8) & 0xff, nA & 0xff);

        return acIp;
    }

    if (m_pIpaPrivate->m_nType == IpAddressPrivate::TYPE_IPv6)
    {
        IMS_UINT16 aTmp[8];

        for (IMS_SINT32 i = 0; i < 8; ++i)
        {
            aTmp[i] = (IMS_UINT16(m_pIpaPrivate->m_objA6[2 * i] << 8)) |
                    (IMS_UINT16(m_pIpaPrivate->m_objA6[2 * i + 1]));
        }

#ifndef __IMS_IPV6_SHORT_FORM__
        IMS_BOOL bGoZStarted = IMS_FALSE;
        GroupOfZeroes objGoZ;
        ImsList<GroupOfZeroes> objGoZs;

        for (IMS_SINT32 j = 0; j < 8; ++j)
        {
            if ((aTmp[j] == 0) && (!bGoZStarted))
            {
                objGoZ.SetIndexS(j);
                objGoZ.SetIndexE(j);
                bGoZStarted = IMS_TRUE;
            }

            if (bGoZStarted && aTmp[j] != 0)
            {
                objGoZ.SetIndexE(j - 1);
                objGoZs.Append(objGoZ);
                bGoZStarted = IMS_FALSE;
            }
        }

        // If the address ends with the group of zeroes, then it needs to be added to the list.
        if (bGoZStarted)
        {
            objGoZ.SetIndexE(7);
            objGoZs.Append(objGoZ);
        }

        if (objGoZs.IsEmpty())
        {
            IMS_Sprintf(acIp, MAX_SIZE, "%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2],
                    aTmp[3], aTmp[4], aTmp[5], aTmp[6], aTmp[7]);

            return acIp;
        }
        else
        {
            objGoZ = objGoZs.GetAt(objGoZs.GetSize() - 1);

            // Find the largest group of zeroes
            for (IMS_UINT32 i = 0; i < objGoZs.GetSize(); ++i)
            {
                const GroupOfZeroes& objTmpGoZ = objGoZs.GetAt(i);

                if ((objGoZ.GetIndexE() - objGoZ.GetIndexS()) <
                        (objTmpGoZ.GetIndexE() - objTmpGoZ.GetIndexS()))
                {
                    objGoZ = objTmpGoZ;
                }
            }

            IMS_CHAR* pIp = &acIp[0];
            IMS_UINT32 nMaxSize = MAX_SIZE;

            if (objGoZ.GetIndexS() == 0)
            {
                // Starts with the group of zeroes
                *pIp = (objGoZ.GetIndexE() == 0) ? '0' : ':';
                pIp++;
                nMaxSize--;
            }
            else
            {
                for (IMS_SINT32 i = 0; i < objGoZ.GetIndexS(); ++i)
                {
                    IMS_SINT32 nCount = IMS_Sprintf(pIp, nMaxSize, "%x:", aTmp[i]);
                    pIp = (pIp + nCount);
                    nMaxSize -= nCount;
                }
            }

            if (objGoZ.GetIndexE() == 7)
            {
                // Ends with the group of zeroes
                *pIp = (objGoZ.GetIndexS() == 7) ? '0' : ':';
                pIp++;
            }
            else
            {
                for (IMS_SINT32 i = (objGoZ.GetIndexE() + 1); i < 8; ++i)
                {
                    IMS_SINT32 nCount = IMS_Sprintf(pIp, nMaxSize, ":%x", aTmp[i]);
                    pIp = (pIp + nCount);
                    nMaxSize -= nCount;
                }
            }

            *pIp = '\0';

            return acIp;
        }
#else
        IMS_Sprintf(acIp, MAX_SIZE, "%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2], aTmp[3],
                aTmp[4], aTmp[5], aTmp[6], aTmp[7]);

        return acIp;
#endif  // __IMS_IPV6_SHORT_FORM__
    }

    return acIp;
}

PUBLIC GLOBAL IMS_UINT16 IpAddress::HToNS(IN IMS_UINT16 nHost)
{
    return (((nHost & 0x00FF) << 8) | ((nHost & 0xFF00) >> 8));
}

PUBLIC GLOBAL IMS_UINT32 IpAddress::HToNL(IN IMS_UINT32 nHost)
{
    return (((nHost & 0x000000FFU) << 24) | ((nHost & 0x0000FF00U) << 8) |
            ((nHost & 0x00FF0000U) >> 8) | ((nHost & 0xFF000000U) >> 24));
}

PUBLIC GLOBAL IMS_UINT16 IpAddress::NToHS(IN IMS_UINT16 nNetwork)
{
    return (((nNetwork & 0x00FF) << 8) | ((nNetwork & 0xFF00) >> 8));
}

PUBLIC GLOBAL IMS_UINT32 IpAddress::NToHL(IN IMS_UINT32 nNetwork)
{
    return (((nNetwork & 0x000000FFU) << 24) | ((nNetwork & 0x0000FF00U) << 8) |
            ((nNetwork & 0x00FF0000U) >> 8) | ((nNetwork & 0xFF000000U) >> 24));
}
