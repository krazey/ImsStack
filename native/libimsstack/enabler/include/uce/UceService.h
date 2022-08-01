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

#include "ICoreService.h"
#include "ICoreServiceListener.h"
#include "ImsService.h"
#include "IUce.h"

class UceSubscribeManager;
class UcePublishManager;
class UceOptionsManager;
class IImsActivityController;

class UceService : public ImsService, public ICoreServiceListener
{
public:
    UceService(IN const AString& strAppName, IN const IMS_SINT32 nSlotId);
    virtual ~UceService();
    /* ------------------------------------------------------------------------------------------
        Method
    ---------------------------------------------------------------------------------------------
  */
public:
    void AoSConnected(IMS_UINT32 conectedService);
    void AoSDisconnected();
    void AosDisConnecting();

protected:
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    virtual void CoreService_PageMessageReceived(
            IN ICoreService* piService, IN IPageMessage* piMessage);
    virtual void CoreService_ReferenceReceived(
            IN ICoreService* piService, IN IReference* piReference);
    virtual void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo);
    virtual void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession);
    virtual void CoreService_UnsolicitedNotifyReceived(
            IN ICoreService* piService, IN IMessage* piNotify);
    virtual void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities);
    void EnableManager();
    void DisableManager();

private:
    void EnableCoreService();
    void DisableCoreService();
    // related to options
    IMS_BOOL SendOptionsRequest(IN IUceOptionsCmdPrm* pParam);
    IMS_BOOL SendOptionsResponse(IN IUceOptionsRespCmdPrm* pParam);
    IMS_BOOL OptionsReceived(IN ICoreService* piCoreService, IN ICapabilities* piCapabilities);
    // related to publish
    IMS_BOOL SendPublishRequest(IN IUcePubCmdPrm* pParam);
    // related to subscribe
    IMS_BOOL QuerySingleCapability(IN IUceSingleSubCmdPrm* pParam);
    IMS_BOOL QueryMultiCapability(IN IUceListSubCmdPrm* pParam);
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
