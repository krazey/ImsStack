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

class AosERegistration
    : public AosRegistration
{
public:
    AosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId);
    virtual ~AosERegistration();

    /// IAosRegistration
    virtual void Start();
    virtual void Update(IN IMS_BOOL bIgnoreRetryTimer = IMS_FALSE,
            IN IMS_BOOL bExplicitUpdate = IMS_TRUE);

    virtual void RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0);

protected:
    /// IMSActivityEx
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);

    /// Initialize
    virtual void Init();

    virtual IMS_BOOL IsFakeModeCondition();
    virtual IMS_BOOL IsReinitiationRequested() const;

    virtual void ProcessAuthenticationFailed();

    virtual void ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode = 0);
    virtual void ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode = 0);

    virtual void ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessStartFailed_TxnTimeout();
    virtual void ProcessStartFailed_Others(IN IMS_SINT32 nReason);

    virtual void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessUpdateFailed_TxnTimeout();
    virtual void ProcessUpdateFailed_Others(IN IMS_SINT32 nReason);

    virtual void ProcessECallStarted();
    virtual void ProcessFakeMode();
    virtual void ProcessFakeModeWithRegState(IN IMS_BOOL bIsRegistered);
    virtual void ProcessReinitiateWithRegState(IN IMS_BOOL bIsRegistered);


    virtual void SetReinitiationRequested(IN IMS_BOOL bRequest);

    /// Timer
    virtual void ProcessTransactionTimerExpired();

    /// IRegistrationListener
    virtual void Registration_RefreshTimerExpired(OUT IMS_BOOL &bDoImplicitRefresh);
    virtual void Registration_Started();
    virtual void Registration_StartFailed(IN IMS_SINT32 nReason);
    virtual void Registration_Terminated(IN IMS_SINT32 nReason);

    /// IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

protected:
    IMS_BOOL m_bReinitiationRequested;
};
#endif // AOS_E_REGISTRATION_H_
