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
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/radio/IMtcRadioChecker.h"
#include "helper/IMtcNetworkWatcherListener.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"

using TrafficType = IMS_UINT32;
using CallDirection = IMS_UINT32;

class ISession;

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
class IMtcService;
class MtcTrafficInfo;

/**
 * Decides whether calling `StartImsTraffic` and `StopImsTraffic` APIs to the Radio and relays
 * response from the Radio.
 *
 * The decision is made as follows.
 * 1. Do `StartImsTraffic` and `StopImsTraffic`
 *    per same traffic type(EMERGENCY/VOICE/VIDEO) and call direction(MO/MT).
 * 2. Do not `StartImsTraffic` again if that traffic was already unblocked.
 * 3. The Calling `StartImsTraffic` should call the `StopImsTraffic` at the end regardless of
 *    the IImsRadioConnectionListener notification.
 * 4. The IImsRadioConnectionListener callback will be invoked after StartImsTraffic.
 * 5. Do `StartImsTraffic` again when the RAT is changed.(handover including NR<->LTE, EPSFB)
 *    and with this case, still one `StopImsTraffic` is required at the end.
 * 6. Even if the UE received `OnConnectionFailed`, if the INVITE is not blocked for some reasons,
 *    consider that traffic is unblocked.
 */
class MtcRadioChecker final :
        public IMtcRadioChecker,
        public IInterfaceHolderListener,
        public IMtcRadioConnectionListener,
        public IMtcNetworkWatcherListener
{
public:
    explicit MtcRadioChecker(IN IMtcContext& objContext);
    ~MtcRadioChecker() override;
    MtcRadioChecker(IN const MtcRadioChecker&) = delete;
    MtcRadioChecker& operator=(IN const MtcRadioChecker&) = delete;

    void Init();

    void AddTrafficCheckerListener(IN IMtcRadioCheckerListener& objListener) override;
    void RemoveTrafficCheckerListener(IN IMtcRadioCheckerListener& objListener) override;
    void OnTerminatedBeforeCreatingSession(IN CallKey nCallKey) override;
    CheckResult Check(IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
            IN IMS_SINT32 eRatType, IN IMS_BOOL bUssi, IN CallKey nCallKey) override;
    inline IMS_UINT32 GetRegistrationThrottlingTimeMillis() const override
    {
        return m_nRegistrationThrottlingTimeMillis;
    }

    // IInterfaceHolderListener
    void OnSessionInterfaceReleased(IN CallKey nKey, IN ISession& objSession) override;

    // IMtcRadioConnectionListener
    void OnConnectionFailed(IN TrafficType eTrafficType, IN CallDirection eCallDirection,
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void OnConnectionSetupPrepared(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection) override;

    // IMtcNetworkWatcherListener
    void OnRatChanged(IN ServiceType eServiceType, IN IMS_SINT32 eOldRatType,
            IN IMS_SINT32 eRatType) override;

    static IMS_BOOL IsReasonToIgnore(IN IMS_UINT32 nFailureReason);

private:
    void DeInit();
    static TrafficType ConvertCallTypeToTrafficType(IN CallType eCallType, IN IMS_BOOL bEmergency);
    IMS_UINT32 ConvertRatType(IN IMS_SINT32 eRatType) const;
    void AddCallKey(IN MtcTrafficInfo& objMtcTrafficInfo, IN CallKey nCallKey);
    void RemoveCallKeyAndStopTrafficCheckingIfNeeded(IN CallKey nCallKeyIn);
    void MaybeStoreRegistrationThrottlingTime(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis);
    MtcTrafficInfo* GetCallTrafficInfo(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection) const;
    MtcTrafficInfo* CreateCallTrafficInfo(
            IN TrafficType eTrafficType, IN CallDirection eCallDirection, IN IMS_SINT32 eRatType);
    void StartTrafficChecking(IN_OUT MtcTrafficInfo& objTrafficInfo, IN IMS_SINT32 eRat);
    void StopTrafficChecking(IN MtcTrafficInfo& objTrafficInfo);
    IMS_BOOL IsCallTerminated(IN CallKey nKey);
    void UpdateTrafficIfRatChanged(IN_OUT MtcTrafficInfo& objTrafficInfo, IN IMS_SINT32 m_eRat);
    IMS_BOOL IsInEpsfbSilentRedial(ImsList<CallKey>& objCallKeys) const;

private:
    IMtcContext& m_objContext;
    IImsRadio* m_piImsRadio;
    ImsList<IMtcRadioCheckerListener*> m_objMtcRadioCheckerListeners;
    ImsList<MtcTrafficInfo*> m_objMtcTrafficInfos;
    IMS_UINT32 m_nRegistrationThrottlingTimeMillis;
};

class MtcTrafficInfo final : public IImsRadioConnectionListener
{
public:
    explicit MtcTrafficInfo(IN TrafficType eTrafficType, IN CallDirection eCallDirection,
            IMS_SINT32 eRat, IN IMtcRadioConnectionListener& objMtcRadioConnectionListener) :
            m_eTrafficType(eTrafficType),
            m_eCallDirection(eCallDirection),
            m_eRat(eRat),
            m_objMtcRadioConnectionListener(objMtcRadioConnectionListener),
            m_objCallKeys(),
            m_objResult(IMtcRadioChecker::CheckResult::Pending())
    {
    }
    ~MtcTrafficInfo() override = default;
    MtcTrafficInfo(IN const MtcTrafficInfo&) = delete;
    MtcTrafficInfo& operator=(IN const MtcTrafficInfo&) = delete;

    // IImsRadioConnectionListener
    void ImsRadio_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void ImsRadio_OnConnectionSetupPrepared() override;

private:
    friend class MtcRadioChecker;

    const TrafficType m_eTrafficType;
    const CallDirection m_eCallDirection;
    IMS_SINT32 m_eRat;
    IMtcRadioConnectionListener& m_objMtcRadioConnectionListener;
    ImsList<CallKey> m_objCallKeys;
    IMtcRadioChecker::CheckResult m_objResult;
};

#endif
