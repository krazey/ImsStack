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
#include "config/UceConfig.h"
#include "def/UceDef.h"
#include "options/UceOptionsManager.h"
#include "publish/UcePublishManager.h"
#include "subscribe/UceSubscribeManager.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");
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

PROTECTED VIRTUAL IMS_BOOL UceService::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage:nMSG [%d]", objMSG.nMSG, 0, 0);

    switch (objMSG.nMSG)
    {
        case IUUceService::UCE_SEND_PUBLISH_CMD:
        {
            IUcePubCmdPrm* pParam = (IUcePubCmdPrm*)objMSG.nLparam;
            return SendPublishRequest(pParam);
        }
        break;
        case IUUceService::UCE_SEND_SINGLE_SUBSCRIBE_CMD:
        {
            IUceSingleSubCmdPrm* pParam = (IUceSingleSubCmdPrm*)objMSG.nLparam;
            return QuerySingleCapability(pParam);
        }
        case IUUceService::UCE_SEND_LIST_SUBSCRIBE_CMD:
        {
            IUceListSubCmdPrm* pParam = (IUceListSubCmdPrm*)objMSG.nLparam;
            return QueryMultiCapability(pParam);
        }
        break;
        case IUUceService::UCE_SEND_OPTIONS_CMD:
        {
            IUceOptionsCmdPrm* pParam = (IUceOptionsCmdPrm*)objMSG.nLparam;
            return SendOptionsRequest(pParam);
        }
        break;
        case IUUceService::UCE_SEND_OPTIONS_RESP_CMD:
        {
            IUceOptionsRespCmdPrm* pParam = (IUceOptionsRespCmdPrm*)objMSG.nLparam;
            return SendOptionsResponse(pParam);
        }
        break;
        default:
            break;
    }
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

IMS_BOOL UceService::SendOptionsRequest(IN IUceOptionsCmdPrm* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->SendOptionsRequest(
                pParam->m_nKey, pParam->m_strRemoteUri, pParam->m_nMyCaps);
    }
    delete pParam;
    return IMS_TRUE;
}

IMS_BOOL UceService::SendOptionsResponse(IUceOptionsRespCmdPrm* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->SendOptionsResponse(
                pParam->m_nKey, pParam->m_nResponseCode, pParam->m_strReason, pParam->m_nMyCaps);
    }
    delete pParam;
    return IMS_TRUE;
}

IMS_BOOL UceService::OptionsReceived(
        IN ICoreService* piCoreService, IN ICapabilities* piCapabilities)
{
    if (m_pUceOptionsManager != IMS_NULL)
    {
        m_pUceOptionsManager->ReceivedOptions(piCoreService, piCapabilities);
    }
    return IMS_TRUE;
}

IMS_BOOL UceService::SendPublishRequest(IN IUcePubCmdPrm* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_pUcePublishManager != IMS_NULL)
    {
        m_pUcePublishManager->SendPublishRequest(pParam->m_nKey, pParam->m_strPidfXml,
                pParam->m_strEtag, pParam->m_nCapability, pParam->m_nExtended);
    }
    delete pParam;
    return IMS_TRUE;
}

IMS_BOOL UceService::QuerySingleCapability(IN IUceSingleSubCmdPrm* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_pUceSubscribeManager != IMS_NULL)
    {
        m_pUceSubscribeManager->QuerySingleCapability(pParam->m_strUser, pParam->m_nKey);
    }
    delete pParam;
    return IMS_TRUE;
}

IMS_BOOL UceService::QueryMultiCapability(IN IUceListSubCmdPrm* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_pUceSubscribeManager != IMS_NULL)
    {
        m_pUceSubscribeManager->QueryMultiCapability(pParam->userList, pParam->m_nKey);
    }
    delete pParam;
    return IMS_TRUE;
}
