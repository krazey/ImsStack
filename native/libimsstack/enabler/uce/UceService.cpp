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

#include "UceService.h"

#include "Connector.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IPageMessage.h"
#include "IReasonInfo.h"
#include "IReference.h"
#include "IServiceFilterCriteria.h"
#include "ISession.h"
#include "ImsMessage.h"
#include "ImsServiceConfig.h"
#include "ServiceTrace.h"
#include "SipMethod.h"
#include "TriggerPoint.h"
#include "config/UceConfig.h"
#include "def/UceDef.h"
#include "options/UceOptionsManager.h"
#include "publish/UcePublishManager.h"
#include "subscribe/UceSubscribeManager.h"

__IMS_TRACE_TAG_UCE__;
/* -------------------------------------------------------------------------------------------------
    Constructor, Destructor
-------------------------------------------------------------------------------------------------
*/
PUBLIC
UceService::UceService(IN const AString& strAppName, IN const IMS_SINT32 nSlotId) :
        ImsService(AString::ConstNull()),
        m_pUceSubscribeManager(IMS_NULL),
        m_pUcePublishManager(IMS_NULL),
        m_pUceOptionsManager(IMS_NULL),
        m_nSlotId(nSlotId),
        m_piCoreService(IMS_NULL),
        m_strAppName(strAppName)
{
    IMS_TRACE_I("UCE_M : UceService = %" PFLS_u, sizeof(UceService), 0, 0);

    EnableCoreService();
    EnableManager();
}

PUBLIC
UceService::UceService(IN ICoreService* piCoreService) :
        ImsService(AString::ConstNull()),
        m_pUceSubscribeManager(IMS_NULL),
        m_pUcePublishManager(IMS_NULL),
        m_pUceOptionsManager(IMS_NULL),
        m_nSlotId(0),
        m_piCoreService(piCoreService),
        m_strAppName(AString("UceServiceTest"))
{
    IMS_TRACE_I("UCE_M : UceService = %" PFLS_u, sizeof(UceService), 0, 0);
    EnableManager();
}

PUBLIC VIRTUAL UceService::~UceService()
{
    IMS_TRACE_I("UCE_F : UceService = %" PFLS_u, sizeof(UceService), 0, 0);
    DisableManager();
    DisableCoreService();
}

/* -------------------------------------------------------------------------------------------------
    Method
-------------------------------------------------------------------------------------------------
*/
PUBLIC
void UceService::AoSConnected(IMS_UINT32 conectedService)
{
    if (m_pUceSubscribeManager != IMS_NULL)
    {
        m_pUceSubscribeManager->AosConnected(conectedService);
    }

    if (m_pUcePublishManager != IMS_NULL)
    {
        m_pUcePublishManager->AosConnected(conectedService);
    }

    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->AoSConnected();
    }

    if (m_piCoreService != IMS_NULL)
    {
        IFeatureCaps* piFCaps = m_piCoreService->GetFeatureCaps();
        if (piFCaps == IMS_NULL)
        {
            return;
        }
        IMS_BOOL bAddVideoTagInPublish = UceConfig::GetInstance()->GetBoolValue(
                UceConfig::KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH, m_nSlotId);
        SetFeatures(piFCaps, bAddVideoTagInPublish, conectedService);
    }
}

void UceService::AoSDisconnected()
{
    if (m_pUceSubscribeManager != IMS_NULL)
    {
        m_pUceSubscribeManager->AosDisConnected();
    }

    if (m_pUcePublishManager != IMS_NULL)
    {
        m_pUcePublishManager->AosDisConnected();
    }

    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->AoSDisconnected();
    }
}

void UceService::AosDisConnecting()
{
    if (m_pUcePublishManager != IMS_NULL)
    {
        m_pUcePublishManager->AosDisConnecting();
    }
}

IMS_BOOL UceService::SendPublishCmd(IMS_UINT32 key, IMS_UINT32 extended, IMS_UINT32 capability,
        const AString& pidfXml, const AString& eTag)
{
    if (m_pUcePublishManager == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendPublishCmd:m_pUcePublishManager is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pUcePublishManager->SendPublishRequest(key, pidfXml, eTag, capability, extended);
    return IMS_TRUE;
}

IMS_BOOL UceService::SendOptionsCmd(IMS_UINT32 key, IMS_UINT32 myCaps, const AString& remoteUri)
{
    if (m_pUceOptionsManager == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendOptionsCmd:m_pUceOptionsManager is null", 0, 0, 0);
        return IMS_FALSE;
    }
    m_pUceOptionsManager->SendOptionsRequest(key, remoteUri, myCaps);
    return IMS_TRUE;
}

IMS_BOOL UceService::SendOptionsRespCmd(
        IMS_UINT32 key, IMS_SINT32 responseCode, const AString& reason, IMS_UINT32 myCaps)
{
    if (m_pUceOptionsManager == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendOptionsRespCmd:m_pUceOptionsManager is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pUceOptionsManager->SendOptionsResponse(key, responseCode, reason, myCaps);
    return IMS_TRUE;
}

IMS_BOOL UceService::SendSingleSubscribeCmd(IMS_UINT32 key, const AString& user)
{
    if (m_pUceSubscribeManager == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendSingleSubscribeCmd:m_pUceSubscribeManager is null", 0, 0, 0);
        return IMS_FALSE;
    }
    m_pUceSubscribeManager->QuerySingleCapability(user, key);
    return IMS_TRUE;
}

IMS_BOOL UceService::SendListSubscribeCmd(IMS_UINT32 key, const ImsList<AString>& userList)
{
    if (m_pUceSubscribeManager == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendListSubscribeCmd:m_pUceSubscribeManager is null", 0, 0, 0);
        return IMS_FALSE;
    }
    m_pUceSubscribeManager->QueryMultiCapability(userList, key);
    return IMS_TRUE;
}

PROTECTED VIRTUAL void UceService::CoreService_PageMessageReceived(
        IN ICoreService* piService, IN IPageMessage* piMessage)
{
    IMS_TRACE_I("CoreService_PageMessageReceived:invalid case- send 480 response", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    (void)piService;
    piMessage->Reject(480);
    piMessage->Destroy();
}

void UceService::CoreService_ReferenceReceived(
        IN ICoreService* piService, IN IReference* piReference)
{
    IMS_TRACE_I("CoreService_ReferenceReceived:invalid case : send 480 response", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    (void)piService;
    piReference->RejectEx(480);
    piReference->Destroy();
}

void UceService::CoreService_ServiceClosed(IN ICoreService* piService, IN IReasonInfo* piReasonInfo)
{
    IMS_TRACE_I("CoreService_ServiceClosed", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    (void)piService;
    (void)piReasonInfo;

    if (m_piCoreService != piService)
    {
        IMS_TRACE_D("CoreService_ServiceClosed:ICoreService is not matched", 0, 0, 0);
        return;
    }

    if (m_pUceSubscribeManager != IMS_NULL)
    {
        m_pUceSubscribeManager->ClosedService();
    }

    if (m_pUcePublishManager != IMS_NULL)
    {
        m_pUcePublishManager->ClosedService();
    }

    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->ClosedService();
    }
}

void UceService::CoreService_SessionInvitationReceived(
        IN ICoreService* piService, IN ISession* piSession)
{
    IMS_TRACE_I("CoreService_SessionInvitationReceived:invalid case : send 480 response", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    (void)piService;
    piSession->RejectEx(480);
    piSession->Destroy();
}

void UceService::CoreService_UnsolicitedNotifyReceived(
        IN ICoreService* piService, IN IMessage* piNotify)
{
    IMS_TRACE_I("CoreService_UnsolicitedNotifyReceived:invalid case", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    (void)piService;
    (void)piNotify;
}

void UceService::CoreService_CapabilityQueryReceived(
        IN ICoreService* piService, IN ICapabilities* piCapabilities)
{
    if (m_piCoreService != piService)
    {
        IMS_TRACE_D("CoreService_CapabilityQueryReceived:ICoreService is not matched", 0, 0, 0);
        return;
    }
    IMS_TRACE_I("CoreService_CapabilityQueryReceived ", 0, 0, 0);
    OptionsReceived(piService, piCapabilities);
}

void UceService::EnableManager()
{
    if (m_pUceSubscribeManager == IMS_NULL)
    {
        m_pUceSubscribeManager =
                new UceSubscribeManager(AString().Sprintf("UceSubscribeManager%d", m_nSlotId),
                        m_piCoreService, m_strAppName, m_nSlotId);
    }

    if (m_pUcePublishManager == IMS_NULL)
    {
        m_pUcePublishManager = new UcePublishManager(m_piCoreService, m_strAppName, m_nSlotId);
    }

    if (m_pUceOptionsManager == IMS_NULL)
    {
        m_pUceOptionsManager = new UceOptionsManager(m_strAppName, m_piCoreService, m_nSlotId);
    }
}

void UceService::DisableManager()
{
    if (m_pUceSubscribeManager != IMS_NULL)
    {
        delete m_pUceSubscribeManager;
        m_pUceSubscribeManager = IMS_NULL;
    }

    if (m_pUcePublishManager != IMS_NULL)
    {
        delete m_pUcePublishManager;
        m_pUcePublishManager = IMS_NULL;
    }

    if (m_pUceOptionsManager != IMS_NULL)
    {
        delete m_pUceOptionsManager;
        m_pUceOptionsManager = IMS_NULL;
    }
}

void UceService::SetFeatures(
        IFeatureCaps* piFCaps, IMS_BOOL bAddVideoTagInPublish, IMS_UINT32 conectedService)
{
    IMS_TRACE_D("SetFeatures:bAddVideoTagInPublish[%d], conectedService[%d]", bAddVideoTagInPublish,
            conectedService, 0);
    if (bAddVideoTagInPublish)
    {
        piFCaps->RemoveFeature("video", AString::ConstEmpty(), SipMethod::PUBLISH);
        if (conectedService & CONNECTED_SERVICE_VIDEO)
        {
            piFCaps->AddFeature("video", AString::ConstEmpty(), SipMethod::PUBLISH);
        }
    }
    else
    {
        piFCaps->RemoveFeature("video", AString::ConstEmpty());
        if (conectedService & CONNECTED_SERVICE_VIDEO)
        {
            piFCaps->AddFeature("video", AString::ConstEmpty());
        }
    }
}

PRIVATE
void UceService::EnableCoreService()
{
    IMS_TRACE_I("EnableCoreService", 0, 0, 0);

    AString strServiceID("serviceId=");
    strServiceID += ImsServiceConfig::GetServiceName(ImsServiceId::UCE);
    m_piCoreService = DYNAMIC_CAST(ICoreService*,
            (Connector::Open(
                    "imscore", ImsServiceConfig::GetAppName(ImsAppId::UCE), strServiceID)));

    if (m_piCoreService != IMS_NULL)
    {
        if (UceConfig::GetInstance()->GetBoolValue(UceConfig::KEY_SUPPORT_OPTIONS, m_nSlotId) ==
                IMS_TRUE)
        {
            IServiceFilterCriteria* piSFC = m_piCoreService->GetFilterCriteria();
            if (piSFC != IMS_NULL)
            {
                SipMethod objMethod(SipMethod::OPTIONS);
                TriggerPoint objTP(objMethod);

                // Sets the trigger point
                piSFC->AddTriggerPoint(objTP);

                // Sets the callee preference
                piSFC->SetCalleePreference(objMethod);
            }
        }
        m_piCoreService->SetListener(this);
    }
}

void UceService::DisableCoreService()
{
    IMS_TRACE_I("DisableCoreService", 0, 0, 0);
    if (m_piCoreService != IMS_NULL)
    {
        m_piCoreService->SetListener(IMS_NULL);
        m_piCoreService->Close();
        m_piCoreService = IMS_NULL;
    }
}

IMS_BOOL UceService::OptionsReceived(
        IN const ICoreService* piCoreService, IN ICapabilities* piCapabilities)
{
    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->ReceivedOptions(piCoreService, piCapabilities);
    }
    return IMS_TRUE;
}
