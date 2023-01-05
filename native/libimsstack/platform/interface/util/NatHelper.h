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
#ifndef NAT_HELPER_H_
#define NAT_HELPER_H_

#include "IpAddress.h"

class NatHelper
{
private:
    NatHelper();
    ~NatHelper();

public:
    NatHelper(IN const NatHelper&) = delete;
    NatHelper& operator=(IN const NatHelper&) = delete;

public:
    void Clear(IN IMS_SINT32 nSlotId);
    // Argument: device's public IP address
    IpAddress GetPrivateAddress(IN IMS_SINT32 nSlotId, IN const IpAddress& objPublicIp) const;
    // Argument: device's IP address
    IpAddress GetPublicAddress(IN IMS_SINT32 nSlotId, IN const IpAddress& objPrivateIp) const;
    IMS_BOOL IsBehindNat(
            IN IMS_SINT32 nSlotId, IN const IpAddress& objPrivateIp = IpAddress::NONE) const;
    void RemovePublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId);
    // Argument: device's IP address
    void RemovePublicAddress(IN IMS_SINT32 nSlotId, IN const IpAddress& objPrivateIp);
    void SetPublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId,
            IN const IpAddress& objPrivateIp, IN const IpAddress& objPublicIp);

    static NatHelper* GetInstance();
    static IMS_BOOL IsNatResolverRequired();

private:
    void RemoveIpBinding(
            IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId, IN const IpAddress& objPrivateIp);

private:
    class IpBinding
    {
    public:
        inline IpBinding() :
                m_nId(0),
                m_objIp(IpAddress::NONE),
                m_objPublicIp(IpAddress::NONE)
        {
        }

        inline IpBinding(
                IN IMS_SINT32 nId, IN const IpAddress& objIp, IN const IpAddress& objPublicIp) :
                m_nId(nId),
                m_objIp(objIp),
                m_objPublicIp(objPublicIp)
        {
        }

        inline IpBinding(IN const IpBinding& other) :
                m_nId(other.m_nId),
                m_objIp(other.m_objIp),
                m_objPublicIp(other.m_objPublicIp)
        {
        }

        inline ~IpBinding() {}

    public:
        inline IpBinding& operator=(IN const IpBinding& other)
        {
            if (this != &other)
            {
                m_nId = other.m_nId;
                m_objIp = other.m_objIp;
                m_objPublicIp = other.m_objPublicIp;
            }

            return (*this);
        }

    public:
        inline IMS_SINT32 GetId() const { return m_nId; }
        inline const IpAddress& GetPrivateIp() const { return m_objIp; }
        inline const IpAddress& GetPublicIp() const { return m_objPublicIp; }
        inline void SetPublicIp(IN const IpAddress& objIp) { m_objPublicIp = objIp; }

    private:
        IMS_SINT32 m_nId;
        IpAddress m_objIp;
        IpAddress m_objPublicIp;
    };

    ImsList<IpBinding>* GetIpBindings(IN IMS_SINT32 nSlotId) const;

private:
    ImsList<IpBinding>** m_ppBindings;
};

#endif
