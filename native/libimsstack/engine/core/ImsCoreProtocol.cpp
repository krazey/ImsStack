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
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"

#include "private/ConfigurationManager.h"

#include "CoreServiceImpl.h"
#include "IServiceManager.h"
#include "ImsCoreProtocol.h"
#include "ServiceContext.h"
#include "SipDebug.h"
#include "SipError.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE
ImsCoreProtocol::ImsCoreProtocol() :
        ServiceProtocol()
{
}

/**
 * @brief Returns a singleton object of IMS Core Protocol.
 *
 * It throws the error as follows:
 *   ILLEGAL_ARGUMENT,
 *   CONNECTION_NOT_FOUND
 */
PUBLIC GLOBAL ImsCoreProtocol* ImsCoreProtocol::GetInstance()
{
    static ImsCoreProtocol* s_pImsCoreProtocol = IMS_NULL;

    if (s_pImsCoreProtocol == IMS_NULL)
    {
        s_pImsCoreProtocol = new ImsCoreProtocol();
    }

    return s_pImsCoreProtocol;
}

/**
 * @brief Creates a IMS Service.
 *
 * It throws the error as follows:
 *   ILLEGAL_ARGUMENT,
 *   CONNECTION_NOT_FOUND
 */
PRIVATE VIRTUAL IService* ImsCoreProtocol::CreateService(
        IN const AString& strAppId, IN const AString& strServiceId, IN const AString& strUserId)
{
    IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();
    IMS_SINT32 nSlotId = (piThread == IMS_NULL) ? IMS_SLOT_0 : piThread->GetSlotId();

    IMS_TRACE_D("CoreService - appId=%s, serviceId=%s, slotId=%d", strAppId.GetStr(),
            strServiceId.GetStr(), nSlotId);

    if (strUserId.GetLength() > 0)
    {
        IMS_TRACE_D("CoreService - userId=%s", SipDebug::GetUri1(strUserId).GetStr(), 0, 0);
    }

    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    if (!pConfigMngr->IsAppConfigured(strAppId, nSlotId))
    {
        // Throw exception: Connection Not Found
        IMS_TRACE_E(0, "Application (%s) configuration is not registered", strAppId.GetStr(), 0, 0);
        return IMS_NULL;
    }

    const AppConfig* pAppConfig = pConfigMngr->GetAppConfig(strAppId, nSlotId);

    if (pAppConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Getting App.(%s) configuration failed", strAppId.GetStr(), 0, 0);
        return IMS_NULL;
    }

    const CoreServiceConfig* pServiceConfig = pAppConfig->GetCoreServiceConfigEx(strServiceId);

    if (pServiceConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid service Id: %s", strServiceId.GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!IsRegistryConsistent(pAppConfig, pServiceConfig))
    {
        IMS_TRACE_E(0, "The IMS registry is not consistent : App (%s), Service (%s)",
                strAppId.GetStr(), strServiceId.GetStr(), 0);
        return IMS_NULL;
    }

    // Check if the service is already created
    IServiceManager* piServiceMngr = ServiceContext::GetInstance()->GetServiceManager();
    Service* pService = piServiceMngr->GetService(nSlotId, strAppId, strServiceId);

    if (pService != IMS_NULL)
    {
        IMS_TRACE_E(0, "A service with appId (%s) and serviceId (%s) is already created",
                strAppId.GetStr(), strServiceId.GetStr(), 0);
        return IMS_NULL;
    }

    // Check the validity of user identity
    CoreService* pCoreService = IMS_NULL;

    if (!strUserId.IsNULL() && !strUserId.IsEmpty())
    {
        SipAddress objUserId;

        if (!objUserId.Create(strUserId))
        {
            IMS_TRACE_E(0, "The userId (%s) parameter is invalid :: SipError (%d)",
                    SipDebug::GetUri1(strUserId).GetStr(), SipError::GetLastError(), 0);
            return IMS_NULL;
        }

        // Create a Core service
        pCoreService = new CoreService(strAppId, strServiceId, &objUserId);
    }
    else
    {
        // Create a Core service
        pCoreService = new CoreService(strAppId, strServiceId);
    }

    if (pCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating CoreService failed", 0, 0, 0);
        return IMS_NULL;
    }

    ICoreService* piCoreService = new CoreServiceImpl(pCoreService);

    if (piCoreService == IMS_NULL)
    {
        delete pCoreService;

        IMS_TRACE_E(0, "Allocating CoreServiceImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pCoreService->CreateConfig(*pAppConfig))
    {
        piCoreService->Close();
        return IMS_NULL;
    }

    if (!piServiceMngr->AttachService(pCoreService))
    {
        piCoreService->Close();

        IMS_TRACE_E(0, "Appending a CoreService failed", 0, 0, 0);
        return IMS_NULL;
    }

    pCoreService->SetServiceCloseListener(piServiceMngr);

    return piCoreService;
}

/**
 * @brief Checks if the IMS registry is consistent.
 */
PRIVATE GLOBAL IMS_BOOL ImsCoreProtocol::IsRegistryConsistent(
        IN const AppConfig* pAppConfig, IN const CoreServiceConfig* pServiceConfig)
{
    if (pAppConfig->IsStreamMediaAudioSupported() || pAppConfig->IsStreamMediaVideoSupported())
    {
        return IMS_TRUE;
    }

    if (pAppConfig->IsFramedMediaSupported())
    {
        return IMS_TRUE;
    }

    if (!pAppConfig->GetBasicMediaMimeTypes().IsEmpty())
    {
        return IMS_TRUE;
    }

    if (!pAppConfig->GetSupportedEventPackages().IsEmpty())
    {
        return IMS_TRUE;
    }

    if (pServiceConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
