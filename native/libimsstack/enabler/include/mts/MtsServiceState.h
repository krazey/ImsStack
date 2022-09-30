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

#ifndef MTS_SERVICESTATE_H_
#define MTS_SERVICESTATE_H_

#include "IMtsServiceState.h"
#include "ITimer.h"

class IImsRadio;

class MtsServiceState final : public IMtsServiceState, public ITimerListener
{
public:
    MtsServiceState(IN IMS_SINT32 nSlotId);
    ~MtsServiceState();

    inline IMS_UINT32 GetConnectedServices() const { return m_nConnectedServices; }
    inline IMS_BOOL GetImsRegState() const { return m_bImsConnected; }
    inline IMS_BOOL GetImsRegMod() const { return m_bAosRegModAdmin; }
    inline IMS_BOOL GetImsSuspendState() const { return m_bImsSuspend; }
    inline IMS_SINT32 GetMtsServiceState() const { return m_nMtsServiceState; }
    inline IMS_SINT32 GetSlotId() const { return m_nSlotId; }
    inline IMS_BOOL GetSmsOverIpState() const { return m_bSmsOverIpConf; }

    void UpdateServiceState();

    // IMtsServiceState
    IMS_BOOL IsMoServiceBlocked() const override;
    IMS_BOOL IsMtServiceBlocked() const override;
    IMS_BOOL IsTemporaryServiceBlocked() const override;
    IMS_BOOL IsImsTrafficAllowed(IN IMS_UINT32 nTrafficType) override;
    void StartImsTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nAccessNetworkType,
            IN IImsRadioConnectionListener* piListener) override;
    void TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason) override;
    void AddListenerForTrafficPriority(IN IImsRadioTrafficPriorityListener* piListener) override;
    void RemoveListenerForTrafficPriority(
            IN IImsRadioTrafficPriorityListener* piListener) override;
    void StartRadioGuardTimer(IN IMS_UINT32 nTrafficType);
    IMS_BOOL IsRadioGuardTimerActive(IN IMS_UINT32 nTrafficType);

    IMS_SINT32 GetServiceState() override;
    IMS_BOOL IsServiceConnected(IN IMS_UINT32 nService) override;
    void NotifySpecificMessage(
            IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam) override;
    void OnImsConnected() override;
    void OnImsDisconnected(IN IMS_UINT32 nReason) override;
    void OnImsDisconnecting(IN IMS_UINT32 nReason) override;
    void OnImsSuspended(IN IMS_UINT32 nReason) override;
    void OnImsResumed() override;
    void SetConnectedServices(IN IMS_UINT32 nServices) override;
    void SetImsRegConnected(IN IMS_BOOL bConnected) override;
    void SetSmsOverIpState(IN IMS_BOOL bState) override;

    // ITmerListener
    void Timer_TimerExpired(IN ITimer* piTimer);

private:
    void Init();
    void DeInit();
    void SetImsSuspendState(IN IMS_BOOL bState);
    void SetMtsServiceState(IN IMS_SINT32 nServiceState);
    void SetTemporaryServiceBlocked(IN IMS_BOOL bBlocked);
    void StopImsTraffic(IN IMS_UINT32 nTrafficType);
    void StopRadioGuardTimer(IN ITimer* piTimer);

    IMS_SINT32 m_nMtsServiceState;
    // Check Condition for SMS SERVICE MODE
    IMS_BOOL m_bImsConnected;    // if Connected true enable sms mo/mt service.
    IMS_BOOL m_bAosRegModAdmin;  // if Mod Admin true. block mo service.
    IMS_BOOL m_bImsSuspend;      // if IMSAoSApp_IMSSuspended true. block mo service
    // if sms_over_ip_network Ind is false. block mo service
    IMS_BOOL m_bSmsOverIpConf;
    IMS_BOOL m_bTemporaryBlocked;
    IMS_UINT32 m_nConnectedServices;
    IMS_SINT32 m_nSlotId;
    IImsRadio* m_piImsRadio;
    ITimer* m_piEmergencyRadioGuardTimer;
    ITimer* m_piRadioGuardTimer;
};

#endif
