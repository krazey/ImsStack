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
#ifndef AOS_E_APPLICATION_H_
#define AOS_E_APPLICATION_H_

#include "interface/IAosRegStateManagerListener.h"
#include "app/AosApplication.h"

/**
 * @brief This class provides the interface for the emergency AoS.
 *
 * It controls the specific operation for emergency AoS
 *
 */

class AosEApplication : public AosApplication, public IAosRegStateManagerListener
{
public:
    AosEApplication(IN IAosAppContext* piAppContext, IN AString& strAppId);
    ~AosEApplication() override;

    // IAosApplication
    IMS_BOOL RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) override;
    void GetProperty(IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue) override;

protected:
    void InitEmergencyVariable();
    void SetKeepEPdnWhenNoPcscf(IN IMS_BOOL bEnable);
    void SetRegBlockInCbm(IN IMS_BOOL bBlock);
    IMS_BOOL IsKeepEPdnWhenNoPcscf() const;
    IMS_BOOL IsRegBlockInCbm() const;
    IMS_BOOL IsReleaseEmergencyPdnUponEmergencyCallEnd();
    IMS_BOOL MaybeRedialOverCrossStack();

    // Clean
    void ClearConnection() override;
    virtual void ProcessCleanAll(IN IMS_UINT32 nReason = 0);

    // Message
    IMS_BOOL ProcessMessage(IN IMSMSG& objMsg) override;
    void ProcessRegStart(IN IMSMSG& objMsg) override;
    void ProcessRegStop(IN IMSMSG& objMsg) override;

    // StateMachine
    IMS_BOOL StateNotReady_Condition(IN IMSMSG& objMsg) override;
    IMS_BOOL StateReady_Connection(IN IMSMSG& objMsg) override;
    IMS_BOOL StateReady_Condition(IN IMSMSG& objMsg) override;

    void ProcessRegFailed_StateConnecting(IN IMS_UINT32 nReason) override;
    void ProcessRegFailed_StateConnected(IN IMS_UINT32 nReason) override;
    void ProcessConnectionUpdated_StateDisconnecting(IN IMS_UINT32 nReason) override;
    void ProcessRegFailed_StateUpdating(IN IMS_UINT32 nReason) override;

    void ProcessConnectionDeactivated(IN IMS_UINT32 nReason) override;
    void ProcessConnectionUpdated(IN IMS_UINT32 nReason) override;

    void ProcessRegSucceeded(IN IMS_UINT32 nReason) override;

    void ProcessRegFailed_Start(IN IMS_UINT32 nReason) override;
    void ProcessRegFailed_Update(IN IMS_UINT32 nReason) override;

    void ProcessAppActivatedTimerExpired() override;
    void ProcessAppConnectedTimerExpired() override;
    void ProcessAppTerminatedTimerExpired() override;
    void ProcessReconfigTimerExpired() override;
    void ProcessRegBlockedTimerExpired() override;

    virtual IMS_BOOL IsEmergencyBlocked();
    virtual IMS_BOOL IsWifiConnected();
    virtual IMS_BOOL IsWlanEmergencyBlocked();
    virtual IMS_BOOL IsRegWaitingRequired();
    virtual IMS_BOOL IsECallConnectedNetworkUnavailable();
    virtual void ProcessRegStateCheck();
    virtual void ProcessECallStarted();
    virtual void ProcessECallTerminated();

    // To External Interface
    void UpdateRegState() override;
    IMS_UINT32 UpdateConnectedServices(IN IMS_BOOL bEnforceUpdateRegService) override;

    // IAoSConditionListener
    void Condition_RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason = 0) override;

    // IAosCallTrackerListener
    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) override;
    void CallTracker_ECallSessionReleased(IN IMS_BOOL bEstablished) override;

    // IAosNetTrackerListener
    void NetTracker_StatusChanged() override;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    // IAosRegStateManagerListener
    void RegStateManager_RegStateChanged(IN IMS_UINT32 nState) override;

    void Init() override;
    void CleanUp() override;

    static const IMS_UINT32 EPDN_RELEASE_DELAY_TIME_MILLIS = 2000;

    IMS_BOOL m_bKeepEPdnWhenNoPcscf;
    IMS_BOOL m_bRegBlockInCbm;
};
#endif  // AOS_E_APPLICATION_H_
