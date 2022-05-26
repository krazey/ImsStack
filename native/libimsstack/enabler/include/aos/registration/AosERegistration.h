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
#ifndef AOS_E_REGISTRATION_H_
#define AOS_E_REGISTRATION_H_

#include "registration/AosRegistration.h"

class AosERegistration : public AosRegistration
{
public:
    AosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId);
    virtual ~AosERegistration();

    void Start() final;
    void Update(IN IMS_BOOL bIgnoreRetryTimer = IMS_FALSE,
            IN IMS_BOOL bExplicitUpdate = IMS_TRUE) final;
    void RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) final;

private:
    IMS_BOOL OnMessage(IN IMSMSG& objMsg) final;

    void Init() final;

    void ProcessAuthenticationFailed() final;

    void ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode = 0) final;
    void ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode = 0) final;

    void ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode) final;
    void ProcessStartFailed_TxnTimeout() final;
    void ProcessStartFailed_Others(IN IMS_SINT32 nReason) final;

    void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode) final;
    void ProcessUpdateFailed_TxnTimeout() final;
    void ProcessUpdateFailed_Others(IN IMS_SINT32 nReason) final;

    void ProcessTransactionTimerExpired() final;

    void SetRefreshPolicy() final;

    void Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) final;
    void Registration_Started() final;
    void Registration_StartFailed(IN IMS_SINT32 nReason) final;
    void Registration_Terminated(IN IMS_SINT32 nReason) final;

    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) final;

    IMS_BOOL IsFakeModeCondition();
    IMS_BOOL IsReinitiationRequested() const;

    void ProcessECallStarted();
    void ProcessFakeMode();
    void ProcessFakeModeWithRegState(IN IMS_BOOL bIsRegistered);
    void ProcessReinitiateWithRegState(IN IMS_BOOL bIsRegistered);

    void SetReinitiationRequested(IN IMS_BOOL bRequest);

private:
    IMS_BOOL m_bReinitiationRequested;
};
#endif  // AOS_E_REGISTRATION_H_
