/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    20110618  hwangoo.park@             Changed the display format of the group of zeros for IPv6
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"
#include "IMSStrLib.h"
#include "AStringArray.h"
#include "AStringBuffer.h"
#include "IPAddress.h"

LOCAL
IMS_BOOL ipAddress_ParseIP4(IN CONST AString& strIP4, OUT IMS_UINT32* pnIP4)
{
    IMSList<AString> objIPv4 = strIP4.Split('.');

    if (objIPv4.GetSize() != 4)
    {
        (*pnIP4) = 0;
        return IMS_FALSE;
    }

    IMS_UINT32 nIP4 = 0;
    IMS_BOOL bOK;
    IMS_UINT32 nByteValue;

    for (IMS_SINT32 i = 0; i < 4; ++i)
    {
        bOK = IMS_FALSE;

        const AString& strValue = objIPv4.GetAt(i);

        nByteValue = strValue.ToUInt32(&bOK);

        if (!bOK || (nByteValue > 0xff))
        {
            (*pnIP4) = 0;
            return IMS_FALSE;
        }

        nIP4 <<= 8;
        nIP4 += nByteValue;
    }

    (*pnIP4) = nIP4;

    return IMS_TRUE;
}

LOCAL
IMS_BOOL ipAddress_ParseIP6(IN CONST AString& strIP6, OUT IMS_BYTE* pIP6)
{
    AStringArray objIPv6 = strIP6.Split(':');
    IMS_SINT32 nCount = objIPv6.GetCount();

    if ((nCount < 3) || (nCount > 8))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nMC = IPv6Address::MAX_SIZE;
    IMS_SINT32 nReducedCount = 9 - nCount;
    AString strValue;

    for (IMS_SINT32 i = nCount - 1; i >= 0; --i)
    {
        if (nMC <= 0)
        {
            return IMS_FALSE;
        }

        const AString& strValue = objIPv6.GetElementAt(i);

        if (strValue.IsEmpty())
        {
            if (i == nCount - 1)
            {
                // Special case: ":" is last character
                const AString& strTmp = objIPv6.GetElementAt(i - 1);

                if (!strTmp.IsEmpty())
                {
                    return IMS_FALSE;
                }

                pIP6[--nMC] = 0;
                pIP6[--nMC] = 0;
            }
            else if (i == 0)
            {
                // Special case: ":" is first character
                const AString& strTmp = objIPv6.GetElementAt(i + 1);

                if (!strTmp.IsEmpty())
                {
                    return IMS_FALSE;
                }

                pIP6[--nMC] = 0;
                pIP6[--nMC] = 0;
            }
            else
            {
                for (IMS_SINT32 j = 0; j < nReducedCount; ++j)
                {
                    if (nMC <= 0)
                    {
                        return IMS_FALSE;
                    }

                    pIP6[--nMC] = 0;
                    pIP6[--nMC] = 0;
                }
            }
        }
        else
        {
            IMS_BOOL bOK = IMS_FALSE;
            IMS_UINT32 nByteValue = strValue.ToUInt32(&bOK, 16);

            if (bOK && (nByteValue <= 0xffff))
            {
                pIP6[--nMC] = nByteValue & 0xff;
                pIP6[--nMC] = (nByteValue >> 8) & 0xff;
            }
            else
            {
                if (i != nCount - 1)
                {
                    return IMS_FALSE;
                }

                // Parse the IPv4 part of a mixed type
                IMS_UINT32 nIP4 = 0;

                if (!ipAddress_ParseIP4(strValue, &nIP4))
                {
                    return IMS_FALSE;
                }

                pIP6[--nMC] = nIP4 & 0xff;
                pIP6[--nMC] = (nIP4 >> 8) & 0xff;
                pIP6[--nMC] = (nIP4 >> 16) & 0xff;
                pIP6[--nMC] = (nIP4 >> 24) & 0xff;
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
            nStart(-1),
            nEnd(-1)
    {
    }

    inline GroupOfZeroes(IN CONST GroupOfZeroes& objRHS) :
            nStart(objRHS.nStart),
            nEnd(objRHS.nEnd)
    {
    }

    inline ~GroupOfZeroes() {}

public:
    inline GroupOfZeroes& operator=(IN CONST GroupOfZeroes& objRHS)
    {
        if (this != &objRHS)
        {
            nStart = objRHS.nStart;
            nEnd = objRHS.nEnd;
        }

        return (*this);
    }

    // Gets the start / end index of group of zeroes
    inline IMS_SINT32 GetIndexS() const { return nStart; }
    inline IMS_SINT32 GetIndexE() const { return nEnd; }

    // Sets the start / end index of group of zeroes
    inline void SetIndexS(IN IMS_SINT32 nIndex) { nStart = nIndex; }
    inline void SetIndexE(IN IMS_SINT32 nIndex) { nEnd = nIndex; }

private:
    IMS_SINT32 nStart;
    IMS_SINT32 nEnd;
};

#endif  // __IMS_IPV6_SHORT_FORM__

class IPAddressPrivate
{
public:
    IPAddressPrivate();
    IPAddressPrivate(IN CONST IPAddressPrivate& objRHS);

public:
    void SetAddress(IN IMS_UINT32 nA_ = 0);
    void SetAddress(IN CONST IMS_BYTE* pA_);
    void SetAddress(IN CONST IPv6Address& objA_);

private:
    friend class IPAddress;

    enum
    {
        TYPE_UNKNOWN = 0,
        TYPE_IPv4,
        TYPE_IPv6
    };

    IMS_SINT32 nType;

    // IPv4 address
    IMS_UINT32 nA;
    // IPv6 address
    IPv6Address objA6;
};

PUBLIC
IPAddressPrivate::IPAddressPrivate() :
        nType(TYPE_UNKNOWN),
        nA(0)
{
    IMS_MEM_Memset(&objA6, 0x00, sizeof(IPv6Address));
}

PUBLIC
IPAddressPrivate::IPAddressPrivate(IN CONST IPAddressPrivate& objRHS) :
        nType(objRHS.nType),
        nA(objRHS.nA),
        objA6(objRHS.objA6)
{
}

PUBLIC
void IPAddressPrivate::SetAddress(IN IMS_UINT32 nA_ /* = 0 */)
{
    nType = TYPE_IPv4;
    nA = nA_;
}

PUBLIC
void IPAddressPrivate::SetAddress(IN CONST IMS_BYTE* pA_)
{
    nType = TYPE_IPv6;

    for (IMS_SINT32 i = 0; i < IPv6Address::MAX_SIZE; ++i)
    {
        objA6[i] = pA_[i];
    }
}

PUBLIC
void IPAddressPrivate::SetAddress(IN CONST IPv6Address& objA_)
{
    nType = TYPE_IPv6;
    objA6 = objA_;
}

PUBLIC
IMS_BYTE& IPv6Address::operator[](IN IMS_SINT32 i)
{
    IMS_ASSERT((i >= 0) && (i < MAX_SIZE));
    return aIP6[i];
}

PUBLIC
IMS_BYTE IPv6Address::operator[](IN IMS_SINT32 i) const
{
    IMS_ASSERT((i >= 0) && (i < MAX_SIZE));
    return aIP6[i];
}

PUBLIC
const IMS_BYTE* IPv6Address::GetAddress() const
{
    return &(aIP6[0]);
}

PUBLIC
ByteArray IPv6Address::ToNetworkByteOrder() const
{
    ByteArray objIP6;

    for (IMS_SINT32 i = 0; i < MAX_SIZE; i += 2)
    {
        objIP6.Append(aIP6[i + 1]);
        objIP6.Append(aIP6[i]);
    }

    return objIP6;
}

const IPAddress IPAddress::ANY = IPAddress(AString("0.0.0.0"));
const IPAddress IPAddress::BROADCAST = IPAddress(AString("255.255.255.255"));
const IPAddress IPAddress::LOOPBACK = IPAddress(AString("127.0.0.1"));
const IPAddress IPAddress::NONE = IPAddress(AString("0.0.0.0"));
const IPAddress IPAddress::IPv6ANY = IPAddress(AString("::"));
const IPAddress IPAddress::IPv6LOOPBACK = IPAddress(AString("::1"));
const IPAddress IPAddress::IPv6NONE = IPAddress(AString("::"));

PUBLIC
IPAddress::IPAddress() :
        pIPA(new IPAddressPrivate())
{
}

PUBLIC
IPAddress::IPAddress(IN IMS_UINT32 nIP_) :
        pIPA(new IPAddressPrivate())
{
    pIPA->SetAddress(nIP_);
}

PUBLIC
IPAddress::IPAddress(IN CONST IMS_BYTE* pIP_) :
        pIPA(new IPAddressPrivate())
{
    pIPA->SetAddress(pIP_);
}

PUBLIC
IPAddress::IPAddress(IN CONST IPv6Address& objIP_) :
        pIPA(new IPAddressPrivate())
{
    pIPA->SetAddress(objIP_);
}

PUBLIC
IPAddress::IPAddress(IN CONST AString& strIP_) :
        pIPA(new IPAddressPrivate())
{
    Parse(strIP_);
}

PUBLIC
IPAddress::IPAddress(IN CONST IPAddress& objRHS) :
        pIPA(new IPAddressPrivate(*(objRHS.pIPA)))
{
}

PUBLIC
IPAddress::~IPAddress()
{
    delete pIPA;
}

PUBLIC
IPAddress& IPAddress::operator=(IN CONST IPAddress& objRHS)
{
    if (this != &objRHS)
    {
        *pIPA = *(objRHS.pIPA);
    }

    return (*this);
}

PUBLIC
IMS_BOOL IPAddress::Equals(IN CONST IPAddress& objIPA) const
{
    if (pIPA->nType != objIPA.pIPA->nType)
    {
        return IMS_FALSE;
    }

    if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        return (pIPA->nA == objIPA.pIPA->nA);
    }
    else if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        return (IMS_MEM_Memcmp(&pIPA->objA6, &(objIPA.pIPA->objA6), sizeof(IPv6Address)) == 0);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 IPAddress::GetVersion() const
{
    if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        return IPV6;
    }
    else if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        return IPV4;
    }

    return UNKNOWN;
}

PUBLIC
IMS_BOOL IPAddress::IsIPv4Address() const
{
    return (pIPA->nType == IPAddressPrivate::TYPE_IPv4);
}

PUBLIC
IMS_BOOL IPAddress::IsIPv6Address() const
{
    return (pIPA->nType == IPAddressPrivate::TYPE_IPv6);
}

PUBLIC
IMS_BOOL IPAddress::IsUnknownAddress() const
{
    return (pIPA->nType == IPAddressPrivate::TYPE_UNKNOWN);
}

PUBLIC
IMS_BOOL IPAddress::IsAnyAddress() const
{
    if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        return Equals(ANY);
    }
    else if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        return Equals(IPv6ANY);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IPAddress::IsLoopbackAddress() const
{
    if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        return Equals(LOOPBACK);
    }
    else if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        return Equals(IPv6LOOPBACK);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IPAddress::IsMulticastAddress() const
{
    // IPv4 multicast : 224.0.0.0 ~ 239.255.255.255
    // IPv6 ???

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL IPAddress::IsNoneAddress() const
{
    if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        return Equals(NONE);
    }
    else if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        return Equals(IPv6NONE);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IPAddress::Parse(IN CONST AString& strIP_)
{
    pIPA->nType = IPAddressPrivate::TYPE_UNKNOWN;

    AString strIP = strIP_.Trim();

    // All IPv6 addresses contain a ':', and may contain a '.'
    if (strIP.Contains(':'))
    {
        IMS_BYTE aIP6[IPv6Address::MAX_SIZE];

        // If IPv6 address includes '[' & ']', then remove this character.
        if (strIP.StartsWith('[') && strIP.EndsWith(']'))
        {
            strIP = strIP.GetSubStr(1, strIP.GetLength() - 2);
        }

        if (ipAddress_ParseIP6(strIP, aIP6))
        {
            pIPA->nType = IPAddressPrivate::TYPE_IPv6;
            pIPA->SetAddress(aIP6);

            return IMS_TRUE;
        }
    }

    // All IPv6 addresses contain a '.'
    if (strIP.Contains('.'))
    {
        IMS_UINT32 nIP4 = 0;

        if (ipAddress_ParseIP4(strIP, &nIP4))
        {
            pIPA->nType = IPAddressPrivate::TYPE_IPv4;
            pIPA->SetAddress(nIP4);

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_UINT32 IPAddress::ToIPv4Address() const
{
    return pIPA->nA;
}

PUBLIC
IPv6Address IPAddress::ToIPv6Address() const
{
    return pIPA->objA6;
}

PUBLIC
AString IPAddress::ToString() const
{
    if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        IMS_UINT32 nA = ToIPv4Address();
        AString strIP;

        strIP.Sprintf(
                "%d.%d.%d.%d", (nA >> 24) & 0xff, (nA >> 16) & 0xff, (nA >> 8) & 0xff, nA & 0xff);

        return strIP;
    }

    if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        IMS_UINT16 aTmp[8];

        for (IMS_SINT32 i = 0; i < 8; ++i)
        {
            aTmp[i] = (IMS_UINT16(pIPA->objA6[2 * i] << 8)) | (IMS_UINT16(pIPA->objA6[2 * i + 1]));
        }

#ifndef __IMS_IPV6_SHORT_FORM__
        IMS_BOOL bGoZStarted = IMS_FALSE;
        GroupOfZeroes objGoZ;
        IMSList<GroupOfZeroes> objGoZs;

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
            AString strIP;
            strIP.Sprintf("%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2], aTmp[3], aTmp[4],
                    aTmp[5], aTmp[6], aTmp[7]);

            return strIP;
        }
        else
        {
            AStringBuffer objIP(50);

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
                objIP.Append((objGoZ.GetIndexE() == 0) ? '0' : ':');
            }
            else
            {
                for (IMS_SINT32 i = 0; i < objGoZ.GetIndexS(); ++i)
                {
                    strTmp.Sprintf("%x:", aTmp[i]);
                    objIP.Append(strTmp);
                }
            }

            // RFC 5952, 4.2.2.  Handling One 16-Bit 0 Field
            // The symbol "::" MUST NOT be used to shorten just one 16-bit 0 field.
            // For example, the representation 2001:db8:0:1:1:1:1:1 is correct, but
            // 2001:db8::1:1:1:1:1 is not correct.
            if ((objGoZ.GetIndexS() == objGoZ.GetIndexE()) && (objGoZ.GetIndexS() != 0) &&
                    (objGoZ.GetIndexS() != 7))
            {
                objIP.Append('0');
            }

            if (objGoZ.GetIndexE() == 7)
            {
                // Ends with the group of zeroes
                objIP.Append((objGoZ.GetIndexS() == 7) ? '0' : ':');
            }
            else
            {
                for (IMS_SINT32 i = (objGoZ.GetIndexE() + 1); i < 8; ++i)
                {
                    strTmp.Sprintf(":%x", aTmp[i]);
                    objIP.Append(strTmp);
                }
            }

            return static_cast<const AStringBuffer&>(objIP).GetString();
        }
#else
        AString strIP;
        strIP.Sprintf("%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2], aTmp[3], aTmp[4],
                aTmp[5], aTmp[6], aTmp[7]);

        return strIP;
#endif  // __IMS_IPV6_SHORT_FORM__
    }

    return AString::ConstNull();
}

PUBLIC
const IMS_CHAR* IPAddress::ToCharString() const
{
    const IMS_UINT32 MAX_SIZE = 50;
    static IMS_CHAR acIP_0[MAX_SIZE + 1] = {
            0,
    };
    static IMS_CHAR acIP_1[MAX_SIZE + 1] = {
            0,
    };

    IMS_SINT32 nSlotId = SystemConfig::IsMultiSimEnabled()
            ? ThreadService::GetCurrentSlotId(IMS_SLOT_0)
            : IMS_SLOT_0;
    IMS_CHAR* acIP = (nSlotId == IMS_SLOT_0) ? acIP_0 : acIP_1;

    acIP[0] = '\0';

    if (pIPA->nType == IPAddressPrivate::TYPE_IPv4)
    {
        IMS_UINT32 nA = ToIPv4Address();

        IMS_Sprintf(acIP, MAX_SIZE, "%d.%d.%d.%d", (nA >> 24) & 0xff, (nA >> 16) & 0xff,
                (nA >> 8) & 0xff, nA & 0xff);

        return acIP;
    }

    if (pIPA->nType == IPAddressPrivate::TYPE_IPv6)
    {
        IMS_UINT16 aTmp[8];

        for (IMS_SINT32 i = 0; i < 8; ++i)
        {
            aTmp[i] = (IMS_UINT16(pIPA->objA6[2 * i] << 8)) | (IMS_UINT16(pIPA->objA6[2 * i + 1]));
        }

#ifndef __IMS_IPV6_SHORT_FORM__
        IMS_BOOL bGoZStarted = IMS_FALSE;
        GroupOfZeroes objGoZ;
        IMSList<GroupOfZeroes> objGoZs;

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
            IMS_Sprintf(acIP, MAX_SIZE, "%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2],
                    aTmp[3], aTmp[4], aTmp[5], aTmp[6], aTmp[7]);

            return acIP;
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

            IMS_CHAR* pIP = &acIP[0];
            IMS_UINT32 nMaxSize = MAX_SIZE;

            if (objGoZ.GetIndexS() == 0)
            {
                // Starts with the group of zeroes
                *pIP = (objGoZ.GetIndexE() == 0) ? '0' : ':';
                pIP++;
                nMaxSize--;
            }
            else
            {
                for (IMS_SINT32 i = 0; i < objGoZ.GetIndexS(); ++i)
                {
                    IMS_SINT32 nCount = IMS_Sprintf(pIP, nMaxSize, "%x:", aTmp[i]);
                    pIP = (pIP + nCount);
                    nMaxSize -= nCount;
                }
            }

            if (objGoZ.GetIndexE() == 7)
            {
                // Ends with the group of zeroes
                *pIP = (objGoZ.GetIndexS() == 7) ? '0' : ':';
                pIP++;
            }
            else
            {
                for (IMS_SINT32 i = (objGoZ.GetIndexE() + 1); i < 8; ++i)
                {
                    IMS_SINT32 nCount = IMS_Sprintf(pIP, nMaxSize, ":%x", aTmp[i]);
                    pIP = (pIP + nCount);
                    nMaxSize -= nCount;
                }
            }

            *pIP = '\0';

            return acIP;
        }
#else
        IMS_Sprintf(acIP, MAX_SIZE, "%x:%x:%x:%x:%x:%x:%x:%x", aTmp[0], aTmp[1], aTmp[2], aTmp[3],
                aTmp[4], aTmp[5], aTmp[6], aTmp[7]);

        return acIP;
#endif  // __IMS_IPV6_SHORT_FORM__
    }

    return acIP;
}

PUBLIC GLOBAL IMS_UINT16 IPAddress::HToNS(IN IMS_UINT16 nHost)
{
    return (((nHost & 0x00FF) << 8) | ((nHost & 0xFF00) >> 8));
}

PUBLIC GLOBAL IMS_UINT32 IPAddress::HToNL(IN IMS_UINT32 nHost)
{
    return (((nHost & 0x000000FFU) << 24) | ((nHost & 0x0000FF00U) << 8) |
            ((nHost & 0x00FF0000U) >> 8) | ((nHost & 0xFF000000U) >> 24));
}

PUBLIC GLOBAL IMS_UINT16 IPAddress::NToHS(IN IMS_UINT16 nNetwork)
{
    return (((nNetwork & 0x00FF) << 8) | ((nNetwork & 0xFF00) >> 8));
}

PUBLIC GLOBAL IMS_UINT32 IPAddress::NToHL(IN IMS_UINT32 nNetwork)
{
    return (((nNetwork & 0x000000FFU) << 24) | ((nNetwork & 0x0000FF00U) << 8) |
            ((nNetwork & 0x00FF0000U) >> 8) | ((nNetwork & 0xFF000000U) >> 24));
}
