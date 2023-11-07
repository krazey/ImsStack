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

#include "ImsTypeDef.h"
#include "ITimer.h"

#include "IAosService.h"
#include "interface/IAosPcscf.h"
#include "interface/IAosConnectionListener.h"
#include "interface/IAosServicePhoneListener.h"

class IAosAppContext;
class IAosConnection;
class IAosConnectorListener;
class AosUtil;

class AosConnector :
        public IAosConnectionListener,
        public IAosPcscfListener,
        public ITimerListener,
        public AosServicePhoneListener
{
public:
    explicit AosConnector(IN IAosAppContext* piAppContext);
    inline explicit AosConnector(){};
    virtual ~AosConnector();

    virtual IMS_BOOL Start();
    virtual void Stop();
    virtual void Stop(IN IMS_SINT32 nDelayTimeSec);

    virtual void SetListener(IN IAosConnectorListener* piListener);
    virtual IMS_BOOL IsReady() const;
    virtual void SetPdnDeactivationRequired(IN IMS_BOOL bIsRequired);
    virtual IMS_BOOL IsPdnDeactivationRequired();

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
        REASON_LIMITED_SERVICE_PCO,

        // Connection_Updated
        REASON_IP_CHANGED,
        REASON_PCSCF_CHANGED,
        REASON_IPCAN_CAT_CHANGED,

        REASON_OTHERS
    };

    enum
    {
        TIMER_IPV6 = 0,
        TIMER_STOP_DELAY,
        TIMER_READY_RECOVERY,
        TIMER_PCO_WAITING
    };

    enum
    {
        PENDING_NONE = 0x0,
        PENDING_IPV6_DELAY = 0x1,
        PENDING_PCSCF_CONFIG_READY = 0x2,
        PENDING_PCO_WAITING = 0x4
    };

    enum
    {
        LISTENER_TYPE_ACTIVATED = 0,
        LISTENER_TYPE_DEACTIVATED,
        LISTENER_TYPE_UPDATED
    };

protected:
    void ClearPending();

    void SetState(IN IMS_UINT32 nState);
    void SetDataConnected(IN IMS_BOOL bConnected);
    void SetEmergencyType(IN IMS_BOOL bEmergency);
    void SetPcscfConfigured(IN IMS_BOOL bConfigured);

    IMS_BOOL IsDataConnected() const;
    IMS_BOOL IsEmergencyType() const;
    IMS_BOOL IsPcscfChangeAvailable() const;
    IMS_BOOL IsPcscfConfigured() const;
    IMS_BOOL IsPcoWaitingRequired() const;
    IMS_BOOL IsCarrierSignalPcoEnabled() const;
    IMS_BOOL IsPending() const;
    IMS_BOOL IsTerminating() const;
    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) const;
    IMS_BOOL IsDataConnectedWithoutPending() const;

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
    virtual void ProcessPcoWaitingTimerExpired();
    virtual void ProcessCheckingPcscfAndIpa();

    // Timer
    virtual void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    virtual void StopTimer(IN IMS_UINT32 nType);
    virtual void ClearTimers();

    // IAosConnectionListener
    void AosConnection_StateChanged(IN IMS_UINT32 nDataState) override;
    void AosConnection_IpChanged() override;
    void AosConnection_IpcanCatChanged() override;
    void AosConnection_PcscfChanged() override;
    void AosConnection_ConnectionFailed() override;

    // IAosPcscfListener
    void Pcscf_NotifyResult(IN IMS_BOOL bResult) override;

    // AosServicePhoneListener
    void ServicePhone_PcoValueChanged(IN IMS_SINT32 nValue) override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual void CleanUp();

    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

protected:
    IAosAppContext* m_piAppContext;
    IAosConnection* m_piConnection;
    IAosService* m_piService;
    IAosPcscf* m_piPcscf;
    ITimer* m_piIpv6Timer;
    ITimer* m_piStopDelayTimer;
    ITimer* m_piReadyRecoveryTimer;
    ITimer* m_piPcoWaitingTimer;
    IAosConnectorListener* m_piListener;
    AosUtil* m_pUtil;

    IMS_UINT32 m_nState;
    IMS_UINT32 m_nPendingFeature;
    IMS_UINT32 m_nReadyRecoveryCount;

    IMS_BOOL m_bPcscfConfigured;
    IMS_BOOL m_bDataConnected;
    IMS_BOOL m_bEmergencyType;
    IMS_BOOL m_bIsTerminating;
    IMS_BOOL m_bIsPcscfChangeIgnored;
    IMS_BOOL m_bIsPdnDeactivationRequired;

    AString strTag;

    static const IMS_SINT32 PCO_INVALID_VALUE = -1;
    static const IMS_UINT32 READY_RECOVERY_DEFAULT_COUNT = 3;
    static const IMS_UINT32 READY_RECOVERY_DEFAULT_TIME = 20;         // 20 Sec.
    static const IMS_UINT32 READY_RECOVERY_BASE_TIME = 20;            // 20 Sec.
    static const IMS_UINT32 READY_RECOVERY_MAX_TIME = 1800;           // 1800 Sec.
    static const IMS_UINT32 IPV6_ADDRESS_WAIT_TIME_SEC = 4;           // 4 Sec.
    static const IMS_UINT32 WAITING_PCO_VALUE_TIMEOUT_MILLIS = 2000;  // 2 Sec.

protected:
    friend class AosApplication;
};
#endif  // AOS_CONNECTOR_H_
