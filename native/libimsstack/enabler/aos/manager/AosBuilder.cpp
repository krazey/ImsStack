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
#include "ServiceTrace.h"

#include "ImsServiceConfig.h"
#include "ImsAosParameter.h"
#include "app/AosAppContext.h"
#include "app/AosApplication.h"
#include "app/AosEApplication.h"
#include "handle/AosHandle.h"
#include "handle/AosHandleMtc.h"
#include "handle/AosHandleMts.h"
#include "handle/AosHandleEmergencyMtc.h"
#include "handle/AosHandleEmergencyMts.h"
#include "handle/AosHandleUce.h"
#include "handle/AosHandleSipController.h"
#include "registration/AosRegistration.h"
#include "registration/AosERegistration.h"
#include "condition/AosSubscriber.h"
#include "connection/AosPcscf.h"
#include "condition/AosBlock.h"
#include "connection/AosConnection.h"
#include "network/AosNetTracker.h"
#include "provider/AosCallTracker.h"
#include "provider/AosRegStateManager.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosNConfiguration.h"
#include "provider/AosSubscriberManager.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosTransaction.h"
#include "external/AosService.h"

#include "manager/AosBuilder.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosBuilder::AosBuilder()
{
    IMS_TRACE_MEM(
            "AOS_MEM", "AOS_M : AosBuilder = %" PFLS_u "/%" PFLS_x, sizeof(AosBuilder), this, 0);
}

PUBLIC VIRTUAL AosBuilder::~AosBuilder()
{
    IMS_TRACE_MEM(
            "AOS_MEM", "AOS_F : AosBuilder = %" PFLS_u "/%" PFLS_x, sizeof(AosBuilder), this, 0);
}

PUBLIC VIRTUAL IAosAppContext* AosBuilder::BuildAppContext(IN AosStaticProfile* pProfile)
{
    IMS_TRACE_D("BuildAppContext :: profile id (%s)", pProfile->GetId().GetStr(), 0, 0);

    return new AosAppContext(pProfile);
}

PUBLIC VIRTUAL IAosApplication* AosBuilder::BuildApp(IN IAosAppContext* piAppContext)
{
    if (piAppContext->GetStaticProfile()->GetRegistrationType() == AosRegistrationType::EMERGENCY)
    {
        return new AosEApplication(piAppContext, piAppContext->GetStaticProfile()->GetId());
    }

    return new AosApplication(piAppContext, piAppContext->GetStaticProfile()->GetId());
}

PUBLIC VIRTUAL IAosHandle* AosBuilder::BuildHandle(
        IN IAosAppContext* piAppContext, IN const AString& strAppId, IN const AString& strSrvId)
{
    IMS_UINT32 nType = ImsAosService::NONE;

    if (strSrvId.EqualsIgnoreCase(ImsServiceConfig::GetServiceName(ImsServiceId::MTC)))
    {
        nType = ImsAosService::MTC;
        IMS_TRACE_D("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandleMtc)",
                strAppId.GetStr(), strSrvId.GetStr(), 0);

        return new AosHandleMtc(piAppContext, strAppId, strSrvId, nType);
    }
    else if (strSrvId.EqualsIgnoreCase(ImsServiceConfig::GetServiceName(ImsServiceId::MTS)))
    {
        nType = ImsAosService::MTS;
        IMS_TRACE_D("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandleMts)",
                strAppId.GetStr(), strSrvId.GetStr(), 0);

        return new AosHandleMts(piAppContext, strAppId, strSrvId, nType);
    }
    else if (strSrvId.EqualsIgnoreCase(
                     ImsServiceConfig::GetServiceName(ImsServiceId::MTC_EMERGENCY)))
    {
        nType = ImsAosService::EMERGENCY_MTC;
        IMS_TRACE_D("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandleEmergencyMtc)",
                strAppId.GetStr(), strSrvId.GetStr(), 0);

        return new AosHandleEmergencyMtc(piAppContext, strAppId, strSrvId, nType);
    }
    else if (strSrvId.EqualsIgnoreCase(
                     ImsServiceConfig::GetServiceName(ImsServiceId::MTS_EMERGENCY)))
    {
        nType = ImsAosService::EMERGENCY_MTS;
        IMS_TRACE_D("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandleEmergencyMts)",
                strAppId.GetStr(), strSrvId.GetStr(), 0);

        return new AosHandleEmergencyMts(piAppContext, strAppId, strSrvId, nType);
    }
    else if (strSrvId.EqualsIgnoreCase(ImsServiceConfig::GetServiceName(ImsServiceId::UCE)))
    {
        nType = ImsAosService::UCE;
        IMS_TRACE_D("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandleUce)",
                strAppId.GetStr(), strSrvId.GetStr(), 0);

        return new AosHandleUce(piAppContext, strAppId, strSrvId, nType);
    }
    else if (strSrvId.EqualsIgnoreCase(
            ImsServiceConfig::GetServiceName(ImsServiceId::SIP_DELEGATE)))
    {
        nType = ImsAosService::SIP_CONTROLLER;
        IMS_TRACE_D("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandleSipController)",
                strAppId.GetStr(), strSrvId.GetStr(), 0);

        return new AosHandleSipController(piAppContext, strAppId, strSrvId, nType);
    }

    IMS_TRACE_I("BuildHandle :: App ID(%s) , Service ID(%s), Type (AosHandle)", strAppId.GetStr(),
            strSrvId.GetStr(), 0);

    return new AosHandle(piAppContext, strAppId, strSrvId, nType);
}

PUBLIC VIRTUAL IAosRegistration* AosBuilder::BuildRegistration(IN IAosAppContext* piAppContext)
{
    if (piAppContext->GetStaticProfile()->GetRegistrationType() == AosRegistrationType::EMERGENCY)
    {
        return new AosERegistration(
                piAppContext, piAppContext->GetStaticProfile()->GetRegistrationId());
    }

    return new AosRegistration(piAppContext, piAppContext->GetStaticProfile()->GetRegistrationId());
}

PUBLIC VIRTUAL IAosSubscriber* AosBuilder::BuildSubscriber(IN IAosAppContext* piAppContext)
{
    return new AosSubscriber(piAppContext);
}

PUBLIC VIRTUAL IAosPcscf* AosBuilder::BuildPcscf(IN IAosAppContext* piAppContext)
{
    return new AosPcscf(piAppContext);
}

PUBLIC VIRTUAL IAosBlock* AosBuilder::BuildBlock(IN IAosAppContext* piAppContext)
{
    return new AosBlock(piAppContext);
}

PUBLIC VIRTUAL IAosConnection* AosBuilder::BuildConnection(IN IAosAppContext* piAppContext)
{
    return new AosConnection(piAppContext);
}

PUBLIC VIRTUAL IAosNetTracker* AosBuilder::BuildNetTracker(IN IAosAppContext* piAppContext)
{
    return new AosNetTracker(piAppContext);
}

PUBLIC VIRTUAL IAosCallTracker* AosBuilder::BuildCallTracker(IN IMS_SINT32 nSlotId)
{
    return new AosCallTracker(nSlotId);
}

PUBLIC VIRTUAL IAosRegStateManager* AosBuilder::BuildRegStateManager()
{
    return new AosRegStateManager();
}

PUBLIC VIRTUAL IAosService* AosBuilder::BuildService(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("BuildService", 0, 0, 0);

    return new AosService(nSlotId);
}

PUBLIC VIRTUAL IAosSubscriberManager* AosBuilder::BuildSubscriberManager(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("BuildSubscriberManager", 0, 0, 0);

    return new AosSubscriberManager(nSlotId);
}

PUBLIC VIRTUAL IAosRetryRepository* AosBuilder::BuildRetryRepository(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("BuildRetryRepository", 0, 0, 0);

    return new AosRetryRepository(nSlotId);
}

PUBLIC VIRTUAL IAosNConfiguration* AosBuilder::BuildNConfiguration()
{
    IMS_TRACE_D("BuildNConfiguration", 0, 0, 0);

    return new AosNConfiguration();
}

PUBLIC VIRTUAL IAosTransaction* AosBuilder::BuildTransaction(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("BuildTransaction", 0, 0, 0);

    return new AosTransaction(nSlotId);
}
