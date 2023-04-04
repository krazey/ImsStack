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

#ifndef MTC_RADIO_CHECKER_H_
#define MTC_RADIO_CHECKER_H_

#include "IImsRadio.h"
#include "IMtcCallStateListener.h"
#include "INetworkWatcher.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/radio/IMtcRadioChecker.h"
#include "helper/IMtcAosStateListener.h"

using TrafficType = IMS_UINT32;
using CallDirection = IMS_UINT32;

class IMtcRadioConnectionListener
{
public:
    virtual ~IMtcRadioConnectionListener() = default;

    /**
     * @brief Notifies
     *
     * @param eTrafficType
     * @param eCallDirection
     * @param nFailureReason
     * @param nCauseCode
     * @param nWaitTimeMillis
     */
    virtual void OnConnectionFailed(IN TrafficType eTrafficType, IN CallDirection eCallDirection,
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) = 0;

    /**
     * @brief Notifies
     *
     * @param eTrafficType
     * @param eCallDirection
     */
    virtual void OnConnectionSetupPrepared(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection) = 0;
};

class IMtcContext;
class IMtcRadioConnectionFailureListener;
class IMtcService;
class MtcTrafficInfo;

class MtcRadioChecker final :
        public IMtcRadioChecker,
        public IMtcCallStateListener,
        public IMtcRadioConnectionListener,
        public IMtcAosStateListener
{
public:
    explicit MtcRadioChecker(IN IMtcContext& objContext,
            IN IMtcRadioConnectionFailureListener& objMtcRadioConnectionFailureListener);
    ~MtcRadioChecker();
    MtcRadioChecker(IN const MtcRadioChecker&) = delete;
    MtcRadioChecker& operator=(IN const MtcRadioChecker&) = delete;

    void Init();

    void SetTrafficCheckerListener(IN IMtcRadioCheckerListener* pListener) override;
    CheckResult Check(IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
            IN IMS_BOOL bWifi, IN CallKey nCallKey) override;

    // IMtcAosStateListener
    inline void OnAosStateChanged(IN IMtcService&, IN MtcAosState, IN IMS_UINT32) override {}
    void OnIpcanChanged(IN IMtcService& objMtcService, IN IMS_UINT32 eIpcan) override;

    // IMtcCallStateListener
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State eState) override;

    // IMtcRadioConnectionListener
    void OnConnectionFailed(IN TrafficType eTrafficType, IN CallDirection eCallDirection,
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void OnConnectionSetupPrepared(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection) override;

    // for test
    void CreateCallTrafficInfoWithGivenValue(IN TrafficType eTrafficType,
            IN CallDirection eCallDirection, IN IMS_BOOL bActive, IN CallKey nCallKeyIn);

private:
    void DeInit();
    static TrafficType ConvertCallTypeToTrafficType(IN CallType eCallType, IN IMS_BOOL bEmergency);
    IMS_UINT32 ConvertNetworkType(IN IMS_BOOL bWifi) const;
    void AddCallKey(IN MtcTrafficInfo& pMtcTrafficInfo, IN CallKey nCallKey);
    void RemoveCallKeyAndStopTrafficCheckingIfNeeded(IN CallKey nCallKeyIn);
    void NotifyRadioConnectionFailedListener(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection);
    MtcTrafficInfo* GetCallTrafficInfo(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection) const;
    MtcTrafficInfo* CreateCallTrafficInfo(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection);
    IMS_BOOL IsTrafficPrepared(
            IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType) const;
    IMS_BOOL IsTrafficAllowed(IN CallType eCallType, IN IMS_BOOL bEmergency) const;
    void StartTrafficChecking(IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
            IN IMS_BOOL bWifi, IN CallKey nCallKey);
    void StopTrafficChecking(IN MtcTrafficInfo& objTrafficInfo);

private:
    IMtcContext& m_objContext;
    IMtcRadioConnectionFailureListener& m_objMtcRadioConnectionFailureListener;
    INetworkWatcher* m_piNetworkWatcher;
    IImsRadio* m_piImsRadio;
    IMtcRadioCheckerListener* m_piMtcRadioCheckerListener;
    ImsList<MtcTrafficInfo*> m_objMtcTrafficInfos;
};

class MtcTrafficInfo final : public IImsRadioConnectionListener
{
public:
    explicit MtcTrafficInfo(IN TrafficType eTrafficType, IN CallDirection eCallDirection,
            IN IMtcRadioConnectionListener& objMtcRadioConnectionListener) :
            m_eTrafficType(eTrafficType),
            m_eCallDirection(eCallDirection),
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
    friend class MtcRadioChecker;

    TrafficType m_eTrafficType;
    CallDirection m_eCallDirection;
    IMtcRadioConnectionListener& m_objMtcRadioConnectionListener;
    ImsList<CallKey> m_objCallKeys;
    IMS_BOOL m_bTrafficActive;
};

#endif
