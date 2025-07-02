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

#ifndef _UCE_SERVICE_H_
#define _UCE_SERVICE_H_

#include "ICoreServiceListener.h"
#include "ImsService.h"
#include "IUce.h"

class ICoreService;
class UceSubscribeManager;
class UcePublishManager;
class UceOptionsManager;
class ICapabilities;
class IPageMessage;
class IReference;
class ISession;
class IFeatureCaps;

class UceService : public ImsService, public ICoreServiceListener
{
public:
    explicit UceService(IN const AString& strAppName, IN const IMS_SINT32 nSlotId);
    explicit UceService(IN ICoreService* piCoreService);
    virtual ~UceService() override;
    /* ------------------------------------------------------------------------------------------
        Method
    ---------------------------------------------------------------------------------------------
  */
public:
    void AoSConnected(IMS_UINT32 conectedService);
    void AoSDisconnected();
    void AosDisConnecting();

    // related to publish
    IMS_BOOL SendPublishCmd(IMS_UINT32 key, IMS_UINT32 extended, IMS_UINT32 capability,
            const AString& pidfXml, const AString& eTag);

    // related to subscribe
    IMS_BOOL SendSingleSubscribeCmd(IMS_UINT32 key, const AString& user);
    IMS_BOOL SendListSubscribeCmd(IMS_UINT32 key, const ImsList<AString>& userList);

    // related to options
    IMS_BOOL SendOptionsCmd(IMS_UINT32 key, IMS_UINT32 myCaps, const AString& remoteUri);
    IMS_BOOL SendOptionsRespCmd(
            IMS_UINT32 key, IMS_SINT32 responseCode, const AString& reason, IMS_UINT32 myCaps);

protected:
    virtual void CoreService_PageMessageReceived(
            IN ICoreService* piService, IN IPageMessage* piMessage) override;
    virtual void CoreService_ReferenceReceived(
            IN ICoreService* piService, IN IReference* piReference) override;
    virtual void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo) override;
    virtual void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession) override;
    virtual void CoreService_UnsolicitedNotifyReceived(
            IN ICoreService* piService, IN IMessage* piNotify) override;
    virtual void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities) override;
    void EnableManager();
    void DisableManager();
    void SetFeatures(
            IFeatureCaps* piFCaps, IMS_BOOL bAddVideoTagInPublish, IMS_UINT32 conectedService);

private:
    void EnableCoreService();
    void DisableCoreService();
    // received options request
    IMS_BOOL OptionsReceived(IN ICoreService* piCoreService, IN ICapabilities* piCapabilities);

    /* ------------------------------------------------------------------------------------------
        VARIABLE
    ---------------------------------------------------------------------------------------------
  */
protected:
    UceSubscribeManager* m_pUceSubscribeManager;
    UcePublishManager* m_pUcePublishManager;
    UceOptionsManager* m_pUceOptionsManager;

private:
    IMS_SINT32 m_nSlotId;
    ICoreService* m_piCoreService;
    AString m_strAppName;
};
#endif /* _UCE_SERVICE_H_ */
