/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef SIPCONTROLLER_MANAGER_H_
#define SIPCONTROLLER_MANAGER_H_

#include "ImsApp.h"
#include "ISipControllerService.h"

class RcsMessageService;
class RcsRegistrationService;
class RcsConfigurationService;

class SipControllerManager : public ImsApp, public ISipControllerService
{
public:
    explicit SipControllerManager(IN const IMS_SINT32 nSlotId, IN const AString& strAppName);
    virtual ~SipControllerManager();

    inline RcsMessageService* GetMsgService() { return m_pRcsMsgService; }
    inline RcsRegistrationService* GetRegService() { return m_pRcsRegService; }
    inline RcsConfigurationService* GetConfService() { return m_pRcsConfService; }

protected:
    inline void NotifyJniEnablerSet() override {}
    virtual void UpdateDelegateRegistration(IN IMS_UINTP nParam) override;
    virtual void TriggerDelegateDeregistration(void) override;
    virtual void OpenMessageTracker(IN const AString& strThreadName) override;
    virtual void SendMessage(IN IMS_UINTP nParam) override;
    virtual void NotifyMessageReceiveError(IN IMS_UINTP nParam) override;
    virtual void CloseSession(IN const AString& strCallId) override;

private:
    void CreateRegService(void);
    void CreateConfService(void);
    void CreateMsgService(void);
    void DisableService(void);

protected:
    IMS_SINT32 m_nSlotId;
    AString m_strAppName;
    AString m_strAppID;
    AString m_strServiceID;

private:
    RcsMessageService* m_pRcsMsgService;
    RcsRegistrationService* m_pRcsRegService;
    RcsConfigurationService* m_pRcsConfService;
};
#endif  // SIPCONTROLLER_MANAGER_H_
