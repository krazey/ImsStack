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
#ifndef AOS_PROVIDER_H_
#define AOS_PROVIDER_H_

#include "ImsMap.h"

class IMutex;

class IAosCallTracker;
class IAosLocationStarter;
class IAosNConfiguration;
class IAosRegStateManager;
class IAosRetryRepository;
class IAosService;
class IAosSubscriberManager;
class IAosTracer;
class IAosTransaction;

class AosDnsQuery;
class AosKeepAlive;
class AosLog;

class AosProvider
{
public:
    AosProvider();
    virtual ~AosProvider();

private:
    AosProvider(IN const AosProvider& objRhs);
    AosProvider& operator=(IN const AosProvider& objRhs);

public:
    static AosProvider* GetInstance();
    static AosLog* GetLog();

    static AosDnsQuery* CreateDnsQuery();
    static AosKeepAlive* CreateKeepAlive(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    IAosCallTracker* GetCallTracker(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosLocationStarter* GetLocationStarter(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosNConfiguration* GetNConfiguration(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosRegStateManager* GetRegStateManager(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosRetryRepository* GetRetryRepository(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosService* GetService(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosSubscriberManager* GetSubscriberManager(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosTracer* GetTracer(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosTransaction* GetTransaction(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    void SetCallTracker(IN IAosCallTracker* piCt, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetLocationStarter(IN IAosLocationStarter* piLs, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetNConfiguration(IN IAosNConfiguration* piNc, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetRegStateManager(IN IAosRegStateManager* piRsm, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetRetryRepository(
            IN IAosRetryRepository* piRetryRepository, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetService(IN IAosService* piService, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetSubscriberManager(
            IN IAosSubscriberManager* piSubscriberManager, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetTracer(IN IAosTracer* piTracer, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetTransaction(IN IAosTransaction* piTransaction, IN IMS_SINT32 nSlotId = IMS_SLOT_0);

private:
    class ProviderParam
    {
    public:
        inline ProviderParam() :
                m_piCallTracker(IMS_NULL),
                m_piLocationStarter(IMS_NULL),
                m_piNConfiguration(IMS_NULL),
                m_piRegStateManager(IMS_NULL),
                m_piRetryRepository(IMS_NULL),
                m_piService(IMS_NULL),
                m_piSubscriberManager(IMS_NULL),
                m_piTracer(IMS_NULL),
                m_piTransaction(IMS_NULL)
        {
        }
        inline ~ProviderParam() {}

    public:
        IAosCallTracker* m_piCallTracker;
        IAosLocationStarter* m_piLocationStarter;
        IAosNConfiguration* m_piNConfiguration;
        IAosRegStateManager* m_piRegStateManager;
        IAosRetryRepository* m_piRetryRepository;
        IAosService* m_piService;
        IAosSubscriberManager* m_piSubscriberManager;
        IAosTracer* m_piTracer;
        IAosTransaction* m_piTransaction;
    };

    IMutex* m_piLock;
    // <slot-id, ProviderParam>
    ImsMap<IMS_SINT32, ProviderParam*> m_objParam;
};
#endif  // AOS_PROVIDER_H_