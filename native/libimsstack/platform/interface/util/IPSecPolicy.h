
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
#ifndef IPSEC_POLICY_H_
#define IPSEC_POLICY_H_

#include "IMSList.h"
#include "IPAddress.h"

class IPSecSA;
class PolicyInfoP;

class IPSecPolicy
{
public:
    IPSecPolicy();
    ~IPSecPolicy();

private:
    IPSecPolicy(IN const IPSecPolicy& objRHS);
    IPSecPolicy& operator=(IN const IPSecPolicy& objRHS);

public:
    enum
    {
        IP_TYPE_V4 = 0,
        IP_TYPE_V6
    };

    IPSecSA* CreateSA();
    void DestroySA(IN IPSecSA* pSA);
    void DestroyAllSAs();

    void SetTransportAddress(IN CONST IPAddress& objUeIPA, IN CONST IPAddress& objServerIPA);
    void SetSAParameter(IN IMS_UINT32 nSecuProto, IN IMS_UINT32 nMode, IN IMS_UINT32 nAuthAlgo,
            IN IMS_UINT32 nEncrAlgo, IN CONST ByteArray& objAuthKey,
            IN CONST ByteArray& objEncrKey);

    void GetSAParameter(OUT IMS_UINT32& nIPvType, OUT IMS_UINT32& nSecuProto, OUT IMS_UINT32& nMode,
            OUT IMS_UINT32& nAuthAlgo, OUT IMS_UINT32& nEncrAlgo);

    // Get IP Address
    IMS_UINT32 GetUeIPv4Address() const;
    IMS_UINT32 GetServerIPv4Addresss() const;
    const ByteArray& GetUeIPv6Address() const;
    const ByteArray& GetServerIPv6Address() const;

    // Get Auth & Encr Key
    const ByteArray& GetAuthKey() const;
    const ByteArray& GetEncrKey() const;
    const AString& GetAuthHexKey() const;
    const AString& GetEncrHexKey() const;

    IPSecSA* FindSA(IN IMS_UINT32 nSPI);
    const IMSList<IPSecSA*>& GetSAs() const;

private:
    void SetAuthenticationKey();
    void SetEncryptionKey();

private:
    PolicyInfoP* pPolicyInfoP;
    IMSList<IPSecSA*> objIPSecSA;
};

#endif  // IPSEC_POLICY_H_
