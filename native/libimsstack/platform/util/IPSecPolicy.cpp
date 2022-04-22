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
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ImsIpSecType.h"
#include "IPSecPolicy.h"
#include "IPSecSA.h"

__IMS_TRACE_TAG_ADAPT__;

class PolicyInfoP
{
public:
    inline PolicyInfoP()
        : nIPvType(IPSecPolicy::IP_TYPE_V6)
        , objUeIPA(IPAddress::IPv6NONE)
        , objUeIPv6(ByteArray::ConstNull())
        , objServerIPA(IPAddress::IPv6NONE)
        , objServerIPv6(ByteArray::ConstNull())
        , nSecuProto(IPSecType::SECURITY_PROTOCOL_ESP)
        , nMode(IPSecType::MODE_TRANSPORT)
        , nAuthAlgo(IPSecType::INTEGRITY_ALGORITHM_HMAC_SHA_1_96)
        , nEncrAlgo(IPSecType::ENCRYPTION_ALGORITHM_NO)
        , objAuthKey(ByteArray::ConstNull())
        , objEncrKey(ByteArray::ConstNull())
        , strAuthHexKey(AString::ConstNull())
        , strEncrHexKey(AString::ConstNull())
    {}
    inline ~PolicyInfoP()
    {}

public:
    IMS_UINT32 nIPvType;
    IPAddress objUeIPA;
    ByteArray objUeIPv6;
    IPAddress objServerIPA;
    ByteArray objServerIPv6;
    IMS_UINT32 nSecuProto;
    IMS_UINT32 nMode;
    IMS_UINT32 nAuthAlgo;
    IMS_UINT32 nEncrAlgo;
    ByteArray objAuthKey;
    ByteArray objEncrKey;
    AString strAuthHexKey;
    AString strEncrHexKey;
};

PUBLIC
IPSecPolicy::IPSecPolicy()
    : pPolicyInfoP(new PolicyInfoP())
    , objIPSecSA(IMSList<IPSecSA*>())
{
}

PUBLIC
IPSecPolicy::~IPSecPolicy()
{
    DestroyAllSAs();

    if (pPolicyInfoP != IMS_NULL)
    {
        delete pPolicyInfoP;
        pPolicyInfoP = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC
IPSecSA* IPSecPolicy::CreateSA()
{
    IPSecSA* pSA = new IPSecSA();

    IMS_TRACE_I("CreateSA :: SA(%p)", pSA, 0, 0);

    objIPSecSA.Append(pSA);

    return pSA;
}

/*

Remarks

*/
PUBLIC
void IPSecPolicy::DestroySA(IN IPSecSA* pSA)
{
    IMS_TRACE_I("DestroySA :: SA(%p)", pSA, 0, 0);

    for (IMS_UINT32 i = 0; i < objIPSecSA.GetSize(); i++)
    {
        IPSecSA* pTmpSA = objIPSecSA.GetAt(i);

        if (pTmpSA == pSA)
        {
            delete pTmpSA;
            objIPSecSA.RemoveAt(i);
            break;
        }
    }
}

/*

Remarks

*/
PUBLIC
void IPSecPolicy::DestroyAllSAs()
{
    IMS_TRACE_I("DestroyAllSAs :: SA-size(%d)", objIPSecSA.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < objIPSecSA.GetSize(); i++)
    {
        IPSecSA* pSA = objIPSecSA.GetAt(i);
        delete pSA;
    }

    objIPSecSA.Clear();
}

/*

Remarks

*/
PUBLIC
void IPSecPolicy::SetTransportAddress(IN CONST IPAddress &objUeIPA,
        IN CONST IPAddress &objServerIPA)
{
    pPolicyInfoP->objUeIPA = objUeIPA;
    pPolicyInfoP->objServerIPA = objServerIPA;

    if (objUeIPA.IsIPv4Address())
    {
        pPolicyInfoP->nIPvType = IP_TYPE_V4;
    }
    else
    {
        pPolicyInfoP->nIPvType = IP_TYPE_V6;
        // Network Byte Order - VZW Mocana Library
        pPolicyInfoP->objUeIPv6.Append(pPolicyInfoP->objUeIPA.ToIPv6Address().GetAddress(),
                IPv6Address::MAX_SIZE);
        pPolicyInfoP->objServerIPv6.Append(pPolicyInfoP->objServerIPA.ToIPv6Address().GetAddress(),
                IPv6Address::MAX_SIZE);
    }
}

/*

Remarks

*/
PUBLIC
void IPSecPolicy::SetSAParameter
(
IN IMS_UINT32 nSecuProto,
IN IMS_UINT32 nMode,
IN IMS_UINT32 nAuthAlgo,
IN IMS_UINT32 nEncrAlgo,
IN CONST ByteArray &objAuthKey,
IN CONST ByteArray &objEncrKey
)
{
    pPolicyInfoP->nSecuProto = nSecuProto;
    pPolicyInfoP->nMode = nMode;
    pPolicyInfoP->nAuthAlgo = nAuthAlgo;
    pPolicyInfoP->nEncrAlgo = nEncrAlgo;
    pPolicyInfoP->objAuthKey = objAuthKey;
    pPolicyInfoP->objEncrKey = objEncrKey;

    SetAuthenticationKey();
    SetEncryptionKey();
}

/*

Remarks

*/
PUBLIC
void IPSecPolicy::GetSAParameter
(
OUT IMS_UINT32 &nIPvType,
OUT IMS_UINT32 &nSecuProto,
OUT IMS_UINT32 &nMode,
OUT IMS_UINT32 &nAuthAlgo,
OUT IMS_UINT32 &nEncrAlgo
)
{
    nIPvType = pPolicyInfoP->nIPvType;
    nSecuProto = pPolicyInfoP->nSecuProto;
    nMode = pPolicyInfoP->nMode;
    nAuthAlgo = pPolicyInfoP->nAuthAlgo;
    nEncrAlgo = pPolicyInfoP->nEncrAlgo;
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecPolicy::GetUeIPv4Address() const
{
    return pPolicyInfoP->objUeIPA.ToIPv4Address();
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecPolicy::GetServerIPv4Addresss() const
{
    return pPolicyInfoP->objServerIPA.ToIPv4Address();
}

/*

Remarks

*/
PUBLIC
const ByteArray& IPSecPolicy::GetUeIPv6Address() const
{
    return pPolicyInfoP->objUeIPv6;
}

/*

Remarks

*/
PUBLIC
const ByteArray& IPSecPolicy::GetServerIPv6Address() const
{
    return pPolicyInfoP->objServerIPv6;
}

/*

Remarks

*/
PUBLIC
const ByteArray& IPSecPolicy::GetAuthKey() const
{
    return pPolicyInfoP->objAuthKey;
}

/*

Remarks

*/
PUBLIC
const ByteArray& IPSecPolicy::GetEncrKey() const
{
    return pPolicyInfoP->objEncrKey;
}

/*

Remarks

*/
PUBLIC
const AString& IPSecPolicy::GetAuthHexKey() const
{
    return pPolicyInfoP->strAuthHexKey;
}

/*

Remarks

*/
PUBLIC
const AString& IPSecPolicy::GetEncrHexKey() const
{
    return pPolicyInfoP->strEncrHexKey;
}

/*

Remarks

*/
PUBLIC
const IMSList<IPSecSA*>& IPSecPolicy::GetSAs() const
{
    return objIPSecSA;
}

/*

Remarks

*/
PUBLIC
IPSecSA* IPSecPolicy::FindSA(IN IMS_UINT32 nSPI)
{
    for (IMS_UINT32 i = 0; i < objIPSecSA.GetSize(); i++)
    {
        IPSecSA* pSA = objIPSecSA.GetAt(i);

        if (pSA->GetSPI() == nSPI )
        {
            return pSA;
        }
    }

    IMS_TRACE_D("FindSA :: not found(spi=%d)", nSPI, 0, 0);
    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE
void IPSecPolicy::SetAuthenticationKey()
{
    // Set Authentication Algorithm & Key (default SHA1)
    AString strHEX;

    for (IMS_SINT32 i = 0; i < pPolicyInfoP->objAuthKey.GetLength(); ++i)
    {
        IMS_BYTE pbyte = pPolicyInfoP->objAuthKey[i];
        strHEX.Sprintf("%02x", pbyte );
        pPolicyInfoP->strAuthHexKey.Append(strHEX);
    }
}

/*

Remarks

*/
PRIVATE
void IPSecPolicy::SetEncryptionKey()
{
    if (pPolicyInfoP->nEncrAlgo != IPSecType::ENCRYPTION_ALGORITHM_NO)
    {
        AString strHEX;
        for (IMS_SINT32 i = 0; i < pPolicyInfoP->objEncrKey.GetLength(); ++i)
        {
            strHEX.Sprintf("%02x", (IMS_BYTE)pPolicyInfoP->objEncrKey[i]);
            pPolicyInfoP->strEncrHexKey.Append(strHEX);
        }
    }
}
