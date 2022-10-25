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

#ifndef MTC_CALL_TRAFFIC_CHECKER_H_
#define MTC_CALL_TRAFFIC_CHECKER_H_

#include "IImsRadio.h"
#include "IMtcCallStateListener.h"
#include "INetworkWatcher.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/traffic/IMtcCallTrafficChecker.h"
#include "helper/IMtcAosStateListener.h"

class IMtcContext;
class IMtcRadioConnectionFailureListener;
class IMtcService;
class MtcTrafficInfo;

class MtcCallTrafficChecker final :
        public IMtcCallTrafficChecker,
        public IMtcCallStateListener,
        public IMtcRadioConnectionListener,
        public IMtcAosStateListener
{
public:
    explicit MtcCallTrafficChecker(IN IMtcContext& objContext,
            IN IMtcRadioConnectionFailureListener& objMtcRadioConnectionFailureListener);
    ~MtcCallTrafficChecker();
    MtcCallTrafficChecker(IN const MtcCallTrafficChecker&) = delete;
    MtcCallTrafficChecker& operator=(IN const MtcCallTrafficChecker&) = delete;

    void Init();

    void SetTrafficCheckerListener(IN IMtcCallTrafficCheckerListener* pListener) override;
    IMS_BOOL IsTrafficPrepared(IN CallType eCallType, IN IMS_BOOL bEmergency) const override;
    IMS_BOOL IsTrafficAllowed(IN CallType eCallType, IN IMS_BOOL bEmergency) const override;
    void StartTrafficChecking(
            IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi) override;
    void StopTrafficChecking(IN TrafficType eTrafficType) override;

    // IMtcAosStateListener
    inline void OnAosStateChanged(IN IMtcService&, IN MtcAosState, IN IMS_UINT32) override {}
    void OnIpcanChanged(IN IMtcService& objMtcService, IN IMS_UINT32 eIpcan) override;

    // IMtcCallStateListener
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State eState) override;

    // IMtcRadioConnectionListener
    void OnConnectionFailed(IN TrafficType eTrafficType, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis) override;
    void OnConnectionSetupPrepared(IN TrafficType eTrafficType) override;

    // for test
    void SetTrafficStatus(IN TrafficType eTrafficType, IN IMS_BOOL bActive);
    ImsList<CallKey>& GetCallKeys(IN TrafficType eTrafficType) const;

private:
    void DeInit();
    TrafficType ConvertCallTypeToTrafficType(IN CallType eCallType, IN IMS_BOOL bEmergency) const;
    IMS_UINT32 ConvertNetworkType(IN IMS_BOOL bWifi);
    void AddCallKeyIfNeeded(IN TrafficType eTrafficType, IN CallKey nCallKey);
    void RemoveCallKeyAndStopTrafficCheckingIfNeeded(IN CallKey nCallKey);
    void NotifyRadioConnectionFailedListener(IN TrafficType eTrafficType);
    void NotifyTrafficCheckerListenerIfPossible(IN IMS_BOOL bReady);

private:
    IMtcContext& m_objContext;
    IMtcRadioConnectionFailureListener& m_objMtcRadioConnectionFailureListener;
    INetworkWatcher* m_piNetworkWatcher;
    IImsRadio* m_piImsRadio;
    IMtcCallTrafficCheckerListener* m_piMtcCallTrafficCheckerListener;
    ImsMap<TrafficType, MtcTrafficInfo*> m_objMtcTrafficInfos;
};

class MtcTrafficInfo final : public IImsRadioConnectionListener
{
public:
    explicit MtcTrafficInfo(IN TrafficType eTrafficType,
            IN IMtcRadioConnectionListener& objMtcRadioConnectionListener) :
            m_eTrafficType(eTrafficType),
            m_objMtcRadioConnectionListener(objMtcRadioConnectionListener),
            m_objCallKeys(),
            m_bTrafficActive(IMS_FALSE)
    {
    }
    ~MtcTrafficInfo() = default;
    MtcTrafficInfo(IN const MtcTrafficInfo&) = delete;
    MtcTrafficInfo& operator=(IN const MtcTrafficInfo&) = delete;

    // IImsRadioConnectionListener
    void ImsRadio_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void ImsRadio_OnConnectionSetupPrepared() override;

private:
    friend class MtcCallTrafficChecker;

    TrafficType m_eTrafficType;
    IMtcRadioConnectionListener& m_objMtcRadioConnectionListener;
    ImsList<CallKey> m_objCallKeys;
    IMS_BOOL m_bTrafficActive;
};

class IMtcRadioConnectionFailureListener
{
public:
    ~IMtcRadioConnectionFailureListener() = default;

    virtual void OnConnectionFailed(IN CallKey nCallKey) = 0;
};

#endif
