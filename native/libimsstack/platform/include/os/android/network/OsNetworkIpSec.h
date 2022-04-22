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
#ifndef OS_NETWORK_IPSEC_H_
#define OS_NETWORK_IPSEC_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "INetworkIpSec.h"
#include "IpSecSaParameter.h"

class OsIpSecPolicy;

class OsNetworkIpSec
    : public INetIPSec
{
public:
    OsNetworkIpSec();
    virtual ~OsNetworkIpSec();

    OsNetworkIpSec(IN const OsNetworkIpSec&) = delete;
    OsNetworkIpSec& operator=(IN const OsNetworkIpSec&) = delete;

public:
    // INetIPSec class
    IIPSecPolicy* CreatePolicy() override;
    void DestroyPolicy(IN IIPSecPolicy* piPolicy) override;
    void DestroyAllPolicies() override;
    IMS_BOOL AddPolicy(IN IIPSecPolicy* piPolicy) override;
    void DeletePolicy(IN IIPSecPolicy* piPolicy) override;
    void FlushPolicies() override;
    void DumpSAs(IN IIPSecPolicy* piPolicy) override;
    IIPSecPolicy* GetPolicy(IN IMS_SINT32 nId) const override;
    IMS_BOOL ApplyIpSecTransform(IN INetSocket* piSocket,
            IN const SocketAddress& objLocal,
            IN const SocketAddress* pRemote = IMS_NULL) override;
    IMS_BOOL ApplyIpSecTransform(IN INetSocket* piSocket,
            IN INetSocket* piServerSocket) override;
    void RemoveIpSecTransforms(IN IMS_SINT32 nSocketId) override;
    void SetSDBFlushCapability(IN IMS_BOOL bCapability) override;

private:
    IMS_SINT32 GetAvailableId();
    static IMS_SINT32 GetSlotId();

private:
    IMS_SINT32 m_nNextAvailableId;
    IMS_BOOL m_bSdbFlushCapability;
    IMSList<OsIpSecPolicy*> m_objPolicy;

    IMSMap<IMS_UINTP, IpSecSaParameter> m_objSaParams;
};

#endif
