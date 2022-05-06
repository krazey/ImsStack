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
#ifndef SERVICE_NETWORK_POLICY_H_
#define SERVICE_NETWORK_POLICY_H_

#include "AString.h"

class NetworkServicePolicyPrivate;

class NetworkPolicy
{
public:
    explicit NetworkPolicy(IN IMS_BOOL bPrimary = IMS_FALSE);
    NetworkPolicy(IN IMS_BOOL bPrimary, IN const AString& strName, IN IMS_SINT32 nApnType);
    NetworkPolicy(IN const NetworkPolicy& other);
    ~NetworkPolicy();

public:
    NetworkPolicy& operator=(IN const NetworkPolicy& other);

public:
    inline IMS_SINT32 GetApnType() const { return m_nApnType; }
    inline const AString& GetName() const { return m_strName; }
    inline IMS_BOOL IsMobilePolicy() const { return IsMobilePolicy(m_nApnType); }
    inline IMS_BOOL IsPrimary() const { return m_bPrimary; }

    static IMS_BOOL IsMobilePolicy(IN const AString& strName);
    static IMS_BOOL IsMobilePolicy(IN IMS_SINT32 nApnType);
    static IMS_BOOL IsWiFiPolicy(IN const AString& strName);
    static IMS_BOOL IsWiFiPolicy(IN IMS_SINT32 nApnType);

public:
    /// APN types
    enum
    {
        APN_NONE = (-1),

        /// Mobile
        APN_IMS = 1,
        APN_INTERNET = 2,
        APN_EMERGENCY = 9,
        APN_MOBILE_MAX,

        /// WiFi
        APN_WIFI = 21,
        APN_WIFI_MAX
    };

private:
    // Flag to inidicate that the policy is a primary or not
    IMS_BOOL m_bPrimary;
    // Unique name to identify a network
    AString m_strName;
    // APN type to identify the network which the application want to use
    IMS_SINT32 m_nApnType;
};

class NetworkServicePolicy
{
private:
    NetworkServicePolicy();
    ~NetworkServicePolicy();

public:
    NetworkServicePolicy(IN const NetworkServicePolicy&) = delete;
    NetworkServicePolicy& operator=(IN const NetworkServicePolicy&) = delete;

public:
    IMS_BOOL AddPolicy(
            IN const AString& strName, IN const NetworkPolicy& objPolicy, IN IMS_SINT32 nSlotId);
    const NetworkPolicy* GetPolicy(IN const AString& strName, IN IMS_SINT32 nSlotId) const;
    inline const NetworkPolicy* GetPolicy(IN IMS_SINT32 nApnType)
            __IMS_DEPRECATED__("Use GetPolicy(IMS_SINT32,IMS_SINT32) instead")
    {
        return GetPolicy(nApnType, IMS_SLOT_0);
    }
    const NetworkPolicy* GetPolicy(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) const;
    void RemovePolicy(IN const AString& strName, IN IMS_SINT32 nSlotId);
    void RemoveAllPolicies(IN IMS_SINT32 nSlotId);

    static NetworkServicePolicy* GetInstance();

private:
    NetworkServicePolicyPrivate* m_pPrivate;
};

#endif
