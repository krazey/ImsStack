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
    virtual ~AosBuilder();

    /// AosAppContext
    virtual IAosAppContext* BuildAppContext(IN AosStaticProfile* pProflie);
    virtual IAosApplication* BuildApp(IN IAosAppContext* piAppContext);
    virtual IAosHandle* BuildHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strSrvId);
    virtual IAosRegistration* BuildRegistration(IN IAosAppContext* piAppContext);
    virtual IAosSubscriber* BuildSubscriber(IN IAosAppContext* piAppContext);
    virtual IAosPcscf* BuildPcscf(IN IAosAppContext* piAppContext);
    virtual IAosBlock* BuildBlock(IN IAosAppContext* piAppContext);
    virtual IAosConnection* BuildConnection(IN IAosAppContext* piAppContext);
    virtual IAosNetTracker* BuildNetTracker(IN IAosAppContext* piAppContext);

    /// AoSProvider
    virtual IAosCallTracker* BuildCallTracker(IN IMS_SINT32 nSlotId);
    virtual IAosRegStateManager* BuildRegStateManager();
    virtual IAosService* BuildService(IN IMS_SINT32 nSlotId);
    virtual IAosSubscriberManager* BuildSubscriberManager(IN IMS_SINT32 nSlotId);
    virtual IAosRetryRepository* BuildRetryRepository(IN IMS_SINT32 nSlotId);
    virtual IAosNConfiguration* BuildNConfiguration();
};
#endif  // AOS_BUILDER_H_
