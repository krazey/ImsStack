/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef TEST_NETWORK_SERVICE_POLICY_H_
#define TEST_NETWORK_SERVICE_POLICY_H_

#include "ServiceNetworkPolicy.h"

class TestNetworkServicePolicy : public NetworkServicePolicy
{
public:
    inline TestNetworkServicePolicy() :
            NetworkServicePolicy(),
            m_pNetworkPolicy(&m_objNetworkPolicy)
    {
    }

    inline IMS_BOOL AddPolicy(IN const AString& /*strName*/, IN const NetworkPolicy& objPolicy,
            IN IMS_SINT32 /*nSlotId*/) override
    {
        m_objNetworkPolicy = objPolicy;
        m_pNetworkPolicy = &m_objNetworkPolicy;
        return IMS_TRUE;
    }
    inline const NetworkPolicy* GetPolicy(
            IN const AString& /*strName*/, IN IMS_SINT32 /*nSlotId*/) const override
    {
        return m_pNetworkPolicy;
    }
    inline const NetworkPolicy* GetPolicy(
            IN IMS_SINT32 /*nApnType*/, IN IMS_SINT32 /*nSlotId*/) const override
    {
        return m_pNetworkPolicy;
    }
    inline void RemovePolicy(IN const AString& /*strName*/, IN IMS_SINT32 /*nSlotId*/) override {}
    inline void RemoveAllPolicies(IN IMS_SINT32 /*nSlotId*/) override {}

private:
    NetworkPolicy m_objNetworkPolicy;
    NetworkPolicy* m_pNetworkPolicy;
};

#endif
