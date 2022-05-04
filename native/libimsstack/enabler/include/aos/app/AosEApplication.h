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

#include "app/AosApplication.h"

/**
 * @brief This class provides the interface for the emergency AoS.
 *
 * It controls the specific operation for emergency AoS
 *
 */

class AosEApplication : public AosApplication
{
public:
    AosEApplication(IN IAosAppContext* piAppContext, IN AString& strAppId);
    virtual ~AosEApplication();

    // IAosApplication
    virtual IMS_BOOL RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0);
    virtual void GetProperty(IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue);

protected:
    void SetTrm(IN IMS_BOOL bStart);

    // Clean
    virtual void ClearConnection();
    virtual void ProcessCleanAll(IN IMS_UINT32 nReason = 0);

    // Message
    virtual IMS_BOOL ProcessMessage(IN IMSMSG& objMsg);
    virtual void ProcessRegStart(IN IMSMSG& objMsg);
    virtual void ProcessRegStop(IN IMSMSG& objMsg);

    // StateMachine
    virtual IMS_BOOL StateNotReady_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateReady_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateReady_Condition(IN IMSMSG& objMsg);

    virtual void ProcessRegFailed_StateConnecting(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_StateConnected(IN IMS_UINT32 nReason);
    virtual void ProcessConnectionUpdated_StateDisconnecting(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_StateUpdating(IN IMS_UINT32 nReason);

    virtual void ProcessConnectionDeactivated(IN IMS_UINT32 nReason);
    virtual void ProcessConnectionUpdated(IN IMS_UINT32 nReason);

    virtual void ProcessRegSucceeded(IN IMS_UINT32 nReason);

    virtual void ProcessRegFailed_Start(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_Update(IN IMS_UINT32 nReason);

    virtual void ProcessAppActivatedTimerExpired();
    virtual void ProcessAppConnectedTimerExpired();
    virtual void ProcessAppTerminatedTimerExpired();
    virtual void ProcessReconfigTimerExpired();

    virtual IMS_BOOL IsEmergencyBlocked();
    virtual IMS_BOOL IsWIFIConnected();
    virtual IMS_BOOL IsWlanEmergencyBlocked();
    virtual IMS_BOOL CheckAppTerminatedTimerAndProcessCleanAll();
    virtual void ProcessECallStarted();
    virtual void ProcessECallTerminated();
    virtual void ProcessECallTerminating();

    // To External Interface
    virtual void UpdateRegState();
    virtual IMS_UINT32 UpdateConnectedServices(IN IMS_BOOL bEnforceUpdateRegService);

    // IAoSConditionListener
    virtual void Condition_RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason = 0);

    // IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    static const IMS_UINT32 EPDN_RELEASE_DELAY_TIME_MILLIS = 2000;

    IMS_BOOL m_bIsCallTerminating;
};
#endif  // AOS_E_APPLICATION_H_
