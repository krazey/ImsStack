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
#ifndef AOS_BUILDER_H_
#define AOS_BUILDER_H_

#include "interface/IAosBuilder.h"

class AosBuilder : public IAosBuilder
{
    // Operation
public:
    AosBuilder();
    ~AosBuilder() override;

    /// AosAppContext
    IAosAppContext* BuildAppContext(IN AosStaticProfile* pProfile) override;
    IAosApplication* BuildApp(IN IAosAppContext* piAppContext) override;
    IAosHandle* BuildHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strSrvId) override;
    IAosRegistration* BuildRegistration(IN IAosAppContext* piAppContext) override;
    IAosSubscriber* BuildSubscriber(IN IAosAppContext* piAppContext) override;
    IAosPcscf* BuildPcscf(IN IAosAppContext* piAppContext) override;
    IAosBlock* BuildBlock(IN IAosAppContext* piAppContext) override;
    IAosConnection* BuildConnection(IN IAosAppContext* piAppContext) override;
    IAosNetTracker* BuildNetTracker(IN IAosAppContext* piAppContext) override;

    /// AoSProvider
    IAosCallTracker* BuildCallTracker(IN IMS_SINT32 nSlotId) override;
    IAosRegStateManager* BuildRegStateManager() override;
    IAosService* BuildService(IN IMS_SINT32 nSlotId) override;
    IAosSubscriberManager* BuildSubscriberManager(IN IMS_SINT32 nSlotId) override;
    IAosRetryRepository* BuildRetryRepository(IN IMS_SINT32 nSlotId) override;
    IAosNConfiguration* BuildNConfiguration() override;
    IAosTracer* BuildTracer(IN IMS_SINT32 nSlotId) override;
    IAosTransaction* BuildTransaction(IN IMS_SINT32 nSlotId) override;
};
#endif  // AOS_BUILDER_H_
