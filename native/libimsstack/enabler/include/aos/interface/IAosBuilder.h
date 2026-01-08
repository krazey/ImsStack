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
#ifndef INTERFACE_AOS_BUILDER_H_
#define INTERFACE_AOS_BUILDER_H_

#include "ImsTypeDef.h"

#include "provider/AosStaticProfile.h"

class AString;

class IAosAppContext;
class IAosHandle;
class IAosApplication;
class IAosConnection;
class IAosRegistration;
class IAosNetTracker;
class IAosBlock;
class IAosSubscriber;
class IAosPcscf;
class IAosCallTracker;
class IAosRegStateManager;
class IAosService;
class IAosSubscriberManager;
class IAosNConfiguration;
class IAosRetryRepository;
class IAosTracer;
class IAosTransaction;

class IAosBuilder
{
public:
    virtual ~IAosBuilder(){};

    /// AosAppContext
    virtual IAosAppContext* BuildAppContext(IN AosStaticProfile* pProfile) = 0;
    virtual IAosApplication* BuildApp(IN IAosAppContext* piAppContext) = 0;
    virtual IAosHandle* BuildHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strSrvId) = 0;
    virtual IAosRegistration* BuildRegistration(IN IAosAppContext* piAppContext) = 0;
    virtual IAosSubscriber* BuildSubscriber(IN IAosAppContext* piAppContext) = 0;
    virtual IAosPcscf* BuildPcscf(IN IAosAppContext* piAppContext) = 0;
    virtual IAosBlock* BuildBlock(IN IAosAppContext* piAppContext) = 0;
    virtual IAosConnection* BuildConnection(IN IAosAppContext* piAppContext) = 0;
    virtual IAosNetTracker* BuildNetTracker(IN IAosAppContext* piAppContext) = 0;

    /// AoSProvider
    virtual IAosCallTracker* BuildCallTracker(IN IMS_SINT32 nSlotId) = 0;
    virtual IAosRegStateManager* BuildRegStateManager() = 0;
    virtual IAosService* BuildService(IN IMS_SINT32 nSlotId) = 0;
    virtual IAosSubscriberManager* BuildSubscriberManager(IN IMS_SINT32 nSlotId) = 0;
    virtual IAosRetryRepository* BuildRetryRepository(IN IMS_SINT32 nSlotId) = 0;
    virtual IAosNConfiguration* BuildNConfiguration() = 0;
    virtual IAosTracer* BuildTracer(IN IMS_SINT32 nSlotId) = 0;
    virtual IAosTransaction* BuildTransaction(IN IMS_SINT32 nSlotId) = 0;
};
#endif  // INTERFACE_AOS_BUILDER_H_
