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

#include "IMSList.h"
#include "IMSMap.h"

#include "IEventListener.h"
// #include "IUCCallListener.h" _UC_TO_MTC_
#include "interface/IAosCallTracker.h"
#include "interface/IAosServicePhoneListener.h"

class AosServicePhoneListener;

class AosCallTracker : public IAosCallTracker, public IEventListener, public AosServicePhoneListener
//, public IUCCallListener _UC_TO_MTC_
{
public:
    AosCallTracker(IN IMS_SINT32 nSlotId_);
    virtual ~AosCallTracker();

    virtual IMS_BOOL IsCsCallActive() const;
    virtual IMS_BOOL IsNormalCallActive() const;
    virtual IMS_BOOL IsEmergencyCallActive() const;
    virtual IMS_BOOL IsVideoCallingActive() const;

    virtual IMS_SINT32 GetSlotId() const;
    virtual IMS_UINT32 GetCallState(IN IMS_UINT32 nType) const;
    virtual IMS_UINT32 GetSessionType(IN IMS_UINT32 nType) const;

    virtual void SetCsCallStateWatchMode();
    virtual void SetActiveCsCallState(IN IMS_UINT32 nActiveCsState);

    virtual void SetListener(IN IAosCallTrackerListener* piListener);
    virtual void RemoveListener(IN IAosCallTrackerListener* piListener);

protected:
    void AddOrUpdateCall(
            IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey, IN IMS_UINT32 nState);
    void RemoveCall(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey);

    IMS_UINT32 GetConvertedState(IN IMS_UINT32 nState);
    IMS_UINT32 GetTotalState(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls);
    IMS_UINT32 GetTotalSessionType(IN IMSMap<IMS_SINTP, IMS_UINT32>& objSessionTypes);

    IMS_UINT32 GetState(IN IMS_UINT32 nType) const;
    void SetState(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    void Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    virtual void ProcessCsChanged(IN IMS_UINT32 nState);
    virtual void ProcessEmergencyChanged(IN IMS_SINTP nKey, IN IMS_UINT32 nState);
    virtual void ProcessNormalChanged(
            IN IMS_SINTP nKey, IN IMS_UINT32 nState, IN IMS_SINT32 nSessionType);

    // IEventListener
    virtual void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam);

    // AosServicePhoneListener
    virtual void ServicePhone_PreciseCallStateChanged(IN PreciseCallState eState);

    // IUCCallListener _UC_TO_MTC_
    virtual void ChangedCallState(IN IMS_UINTP nParam);
    virtual void ChangedCallTotalState(IN IMS_UINTP nParam);

    // Log
    static const IMS_CHAR* TypeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);
    void PrintSessionType(IN IMS_UINT32 nSessType);

protected:
    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nCsState;
    IMS_UINT32 m_nNormalState;
    IMS_UINT32 m_nEmergencyState;
    IMS_UINT32 m_nNormalSessionType;
    IMS_UINT32 m_nActiveCsState;

    IMSMap<IMS_SINTP, IMS_UINT32> m_objNormalCalls;
    IMSMap<IMS_SINTP, IMS_UINT32> m_objEmergencyCalls;
    IMSMap<IMS_SINTP, IMS_UINT32> m_objNormalSessionTypes;

    IMSList<IAosCallTrackerListener*> m_objListeners;

private:
    AString m_strTag;
};

#endif  // AOS_CALL_TRACKER_H_
