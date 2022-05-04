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

#include "IMSMap.h"

class IMutex;
class IAosCallTracker;
class IAosLocationStarter;
class IAosMsgHandler;
class IAosNConfiguration;
class IAosRegStateManager;
class IAosService;
class IAosSubscriberManager;
class IAosTrm;
class IAosVonr;
class IAosRetryRepository;

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

    AosDnsQuery* CreateDnsQuery();
    AosKeepAlive* CreateKeepAlive(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    IAosCallTracker* GetCallTracker(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosLocationStarter* GetLocationStarter(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosMsgHandler* GetMsgHandler(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosNConfiguration* GetNConfiguration(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosRegStateManager* GetRegStateManager(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosService* GetService(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosSubscriberManager* GetSubscriberManager(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosTrm* GetTrm(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosVonr* GetVonr(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IAosRetryRepository* GetRetryRepository(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    void SetCallTracker(IN IAosCallTracker* piCt, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetLocationStarter(IN IAosLocationStarter* piLs, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetMsgHandler(IN IAosMsgHandler* piMh, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetNConfiguration(IN IAosNConfiguration* piNc, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetRegStateManager(IN IAosRegStateManager* piRsm, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetService(IN IAosService* piService, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetSubscriberManager(
            IN IAosSubscriberManager* piSubscriberManager, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetTrm(IN IAosTrm* piTrm, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetVonr(IN IAosVonr* piVonr, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetRetryRepository(
            IN IAosRetryRepository* piRetryRepository, IN IMS_SINT32 nSlotId = IMS_SLOT_0);

private:
    class ProviderParam
    {
    public:
        inline ProviderParam() :
                m_piCallTracker(IMS_NULL),
                m_piLocationStarter(IMS_NULL),
                m_piMsgHandler(IMS_NULL),
                m_piNConfiguration(IMS_NULL),
                m_piRegStateManager(IMS_NULL),
                m_piService(IMS_NULL),
                m_piSubscriberManager(IMS_NULL),
                m_piTrm(IMS_NULL),
                m_piVonr(IMS_NULL),
                m_piRetryRepository(IMS_NULL)
        {
        }
        inline ~ProviderParam() {}

    public:
        IAosCallTracker* m_piCallTracker;
        IAosLocationStarter* m_piLocationStarter;
        IAosMsgHandler* m_piMsgHandler;
        IAosNConfiguration* m_piNConfiguration;
        IAosRegStateManager* m_piRegStateManager;
        IAosService* m_piService;
        IAosSubscriberManager* m_piSubscriberManager;
        IAosTrm* m_piTrm;
        IAosVonr* m_piVonr;
        IAosRetryRepository* m_piRetryRepository;
    };

    IMutex* m_piLock;
    // <slot-id, ProviderParam>
    IMSMap<IMS_SINT32, ProviderParam*> m_objParam;
};
#endif  // AOS_PROVIDER_H_