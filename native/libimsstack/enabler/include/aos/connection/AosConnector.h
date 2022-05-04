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
#ifndef AOS_CONNECTOR_H_
#define AOS_CONNECTOR_H_

#include "IMSTypeDef.h"
#include "ITimer.h"

#include "interface/IAosPcscf.h"
#include "interface/IAosConnectionListener.h"

class IAosAppContext;
class IAosConnection;
class IAosConnectorListener;
class AosUtil;

class AosConnector : public IAosConnectionListener, public IAosPcscfListener, public ITimerListener
{
public:
    AosConnector(IN IAosAppContext* piAppContext);
    virtual ~AosConnector();

    virtual void Start();
    virtual void Stop();
    virtual void Stop(IN IMS_SINT32 nDelayTimeSec);

    virtual void SetListener(IN IAosConnectorListener* piListener);
    virtual IMS_BOOL IsReady() const;

    // GetState
    enum
    {
        STATE_IDLE = 0,
        STATE_READY
    };

    enum
    {
        REASON_NONE = 0,

        // Connection_Deactivated
        REASON_DISCONNECTED,
        REASON_FAILED,
        REASON_PCSCF_DISCOVERY_FAILED,
        REASON_PERMANENTLY_FAILED,

        // Connection_Updated
        REASON_IP_CHANGED,
        REASON_PCSCF_CHANGED,
        REASON_IPCAN_CAT_CHANGED,

        REASON_OTHERS
    };

    // RequestCmd
    enum
    {
        CMD_BLOCK = 0,
        CMD_UNBLOCK
    };

protected:
    void ClearPending();

    void SetState(IN IMS_UINT32 nState);
    void SetDataConnected(IN IMS_BOOL bConnected);
    void SetEmergencyType(IN IMS_BOOL bEmergency);
    void SetPcscfConfigured(IN IMS_BOOL bConfigured);

    IMS_BOOL IsDataConnected() const;
    IMS_BOOL IsEmergencyType() const;
    IMS_BOOL IsIpv6DelayRequired();
    IMS_BOOL IsPcscfChangeAvailable();
    IMS_BOOL IsPcscfConfigured() const;
    IMS_BOOL IsPending() const;
    IMS_BOOL IsTerminating() const;
    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) const;

    void CheckReadyRecoveryAndSetTimer();
    IMS_BOOL CheckIpChangedForEmergency();
    IMS_BOOL CheckIpaAndProcessReadyRecovery();
    IMS_UINT32 GetActualRecoveryWaitingTime();
    IMS_BOOL SelectIpVersion();

    void Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nReason = REASON_NONE);

    virtual void CleanAll();
    virtual IMS_BOOL ConfigurePcscf();

    virtual void ProcessIpv6TimerExpired();
    virtual void ProcessStopDelayTimerExpired();
    virtual void ProcessReadyRecoveryTimerExpired();

    // Timer
    virtual void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    virtual void StopTimer(IN IMS_UINT32 nType);
    virtual void ClearTimers();

    // IAosConnectionListener
    virtual void AosConnection_StateChanged(IN IMS_UINT32 nDataState);
    virtual void AosConnection_IpChanged();
    virtual void AosConnection_IpcanCatChanged();
    virtual void AosConnection_PcscfChanged();
    virtual void AosConnection_ConnectionFailed();

    // IAosPcscfListener
    virtual void Pcscf_NotifyResult(IN IMS_BOOL bResult);

    // ITimerListener
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    friend class AosApplication;
    virtual void CleanUp();

    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

    enum
    {
        TIMER_IPV6 = 0,
        TIMER_STOP_DELAY,
        TIMER_READY_RECOVERY
    };

    enum
    {
        PENDING_NONE = 0x0,
        PENDING_IPV6_DELAY = 0x1,
        PENDING_PCSCF_CONFIG_READY = 0x2
    };

    enum
    {
        LISTENER_TYPE_ACTIVATED = 0,
        LISTENER_TYPE_DEACTIVATED,
        LISTENER_TYPE_UPDATED
    };

protected:
    IAosAppContext* m_piAppContext;
    IAosConnection* m_piConnection;
    IAosPcscf* m_piPcscf;
    ITimer* m_piIpv6Timer;
    ITimer* m_piStopDelayTimer;
    ITimer* m_piReadyRecoveryTimer;
    IAosConnectorListener* m_piListener;
    AosUtil* m_pUtil;

    IMS_UINT32 m_nState;
    IMS_UINT32 m_nPendingFeature;
    IMS_UINT32 m_nReadyRecoveryCount;

    IMS_BOOL m_bPcscfConfigured;
    IMS_BOOL m_bDataConnected;
    IMS_BOOL m_bEmergencyType;
    IMS_BOOL m_bIsTerminating;

    AString strTag;

    static const IMS_UINT32 READY_RECOVERY_DEFAULT_COUNT = 3;
    static const IMS_UINT32 READY_RECOVERY_DEFAULT_TIME = 20;  // 20 Sec.
    static const IMS_UINT32 READY_RECOVERY_BASE_TIME = 20;     // 20 Sec.
    static const IMS_UINT32 READY_RECOVERY_MAX_TIME = 1800;    // 1800 Sec.
    static const IMS_UINT32 IPV6_ADDRESS_WAIT_TIME_SEC = 4;    // 4 Sec.
};
#endif  // AOS_CONNECTOR_H_
