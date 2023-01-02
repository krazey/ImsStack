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
#ifndef RCS_MESSAGE_SERVICE_H_
#define RCS_MESSAGE_SERVICE_H_

#include "ICoreService.h"
#include "ICoreServiceListener.h"
#include "IDirectCoreServiceListener.h"
#include "ISipConnectionFactoryListener.h"
#include "ImsService.h"
#include "IMSTypeDef.h"
#include "ISipMessage.h"
#include "RcsMessageRepository.h"

class RcsMessageService :
        public ImsService,
        public IDirectCoreServiceListener,
        public ISipConnectionFactoryListener
{
public:
    RcsMessageService(IN const AString& strAppName, IN const IMS_SINT32 nSlotId);
    virtual ~RcsMessageService();

private:
    void EnableCoreService();
    void DisableCoreService();
    IMS_BOOL OnMessage(IN IMSMSG& objMSG) override;
    IMS_BOOL CloseSession(IN IMS_UINTP nSessionId);
    void HandleErrorCase(IN IMSMSG& objMSG);
    void PostNotification(IN IMS_SINT32 nMSG, IN IMS_UINTP npParam);
    IMS_BOOL RegisterIMServiceTag();
    void convertMessage(ISipMessage* message);

public:
    IMS_BOOL HandleOpenMSG(IN IMSMSG& objMSG);
    IMS_BOOL HandleSessionMSG(IN IMSMSG& objMSG);
    IMS_BOOL HandleCloseSessionMSG(IN IMSMSG& objMSG);
    IMS_BOOL HandleNotifyReceiveErrorMSG(IN IMSMSG& objMSG);

    IMS_SINT32 DirectCoreService_TransactionReceived(
            IN ICoreService* piService, IN ISipConnectionFactory* piScf) override;
    void ConnectionFactory_NotifyRequest(
            IN ISipConnectionFactory* piScFactory, IN ISipServerConnection* piSsc) override;

private:
    IMS_SINT32 m_nSlotId;
    ICoreService* m_piCoreService;
    ISipConnectionFactory* m_piscf;
    RcsMessageRepository objRcsMessages;
};
#endif /*RCS_MESSAGE_SERVICE_H_*/
