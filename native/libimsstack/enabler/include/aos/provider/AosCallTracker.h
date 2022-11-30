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
#ifndef AOS_CALL_TRACKER_H_
#define AOS_CALL_TRACKER_H_

#include "ImsList.h"
#include "ImsMap.h"

#include "IEventListener.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosServicePhoneListener.h"
#include "../../interface/mtc/IMtcCallStateListener.h"

class AosCallTracker :
        public IAosCallTracker,
        public IEventListener,
        public AosServicePhoneListener,
        public IMtcCallStateListener
{
public:
    AosCallTracker(IN IMS_SINT32 nSlotId);
    virtual ~AosCallTracker();

    IMS_BOOL SetMtcReady() const override;

    IMS_BOOL IsCsCallActive() const override;
    IMS_BOOL IsNormalCallActive() const override;
    IMS_BOOL IsEmergencyCallActive() const override;
    IMS_BOOL IsVideoCallingActive() const override;

    IMS_SINT32 GetSlotId() const override;
    CallState GetCallState(IN IMS_UINT32 nType) const override;

    void SetCsCallStateWatchMode() override;
    void SetActiveCsCallState(IN CallState eActiveCsState) override;

    void SetListener(IN IAosCallTrackerListener* piListener) override;
    void RemoveListener(IN IAosCallTrackerListener* piListener) override;

private:
    template <typename T>
    void AddOrUpdateCall(OUT IMSMap<CallKey, T>& objCalls, IN CallKey eKey, IN T eValue);
    template <typename T>
    void RemoveCall(OUT IMSMap<CallKey, T>& objCalls, IN CallKey eKey);

    CallState GetConvertedState(IN IMtcCall::State eState);
    CallState GetTotalState(IN IMSMap<CallKey, CallState>& objCalls);
    IMS_UINT32 GetTotalCallType(IN IMSMap<CallKey, CallType>& objCallTypes);
    IMS_BOOL IsExistCallType(IN CallType eCallType) const;

    CallState GetState(IN IMS_UINT32 nType) const;
    void SetState(IN IMS_UINT32 nType, IN CallState eState);

    void Notify(IN IMS_UINT32 nType, IN CallState eState);

    void ProcessCsChanged(IN CallState eState);
    void ProcessEmergencyChanged(IN CallKey eKey, IN CallState eState);
    void ProcessNormalChanged(IN CallKey nCallKey, IN CallState eCallState, IN CallType eCallType);

    // IEventListener
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    // AosServicePhoneListener
    void ServicePhone_PreciseCallStateChanged(IN PreciseCallState eState) override;

    // IMtcCallStateListener
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State eState) override;

    // Log
    static const IMS_CHAR* TypeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* StateToString(IN CallState eState);
    static const IMS_CHAR* CallTypeToString(IN CallType eType);

    AString PrintCallTypes(IN IMS_UINT32 nCallTypes);

private:
    IMS_SINT32 m_nSlotId;
    CallState m_eCsState;
    CallState m_eNormalState;
    CallState m_eEmergencyState;
    IMS_UINT32 m_nNormalCallType;
    CallState m_eActiveCsState;

    IMSMap<CallKey, CallState> m_objNormalCalls;
    IMSMap<CallKey, CallState> m_objEmergencyCalls;
    IMSMap<CallKey, CallType> m_objNormalCallTypes;

    IMSList<IAosCallTrackerListener*> m_objListeners;

    AString m_strTag;

private:
    friend class AosCallTrackerTest;
};

#endif  // AOS_CALL_TRACKER_H_
