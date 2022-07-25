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

#ifndef MTS_CALL_TRACKER_H_
#define MTS_CALL_TRACKER_H_

#include "ImsList.h"
#include "ImsMap.h"
#include "IMtsCallTracker.h"

class MtsCallTracker final : public IMtsCallTracker
{
public:
    MtsCallTracker(IN IMS_SINT32 nSlotId_);
    ~MtsCallTracker();

    // IMtsCallTracker
    IMS_BOOL IsEmergencyCallActive() const;
    IMS_SINT32 GetSlotId() const;
    IMS_UINT32 GetCallState(IN IMS_UINT32 nType) const;
    IMS_UINT32 GetSessionType(IN IMS_UINT32 nType) const;
    void AddListener(IN IMtsCallTrackerListener* piListener);
    void RemoveListener(IN IMtsCallTrackerListener* piListener);

private:
    void AddOrUpdateCall(
            IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey, IN IMS_UINT32 nState);
    void RemoveCall(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey);
    IMS_UINT32 GetConvertedState(IN IMS_UINT32 nState);
    IMS_UINT32 GetTotalState(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls);
    IMS_UINT32 GetState(IN IMS_UINT32 nType) const;
    void SetState(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    void Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    void ProcessEmergencyChanged(IN IMS_SINTP nKey, IN IMS_UINT32 nState);

    // IUCCallListener _UC_TO_MTC_
    void ChangedCallState(IN IMS_UINTP nParam);
    void ChangedCallTotalState(IN IMS_UINTP nParam);

    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nEmergencyState;
    IMSMap<IMS_SINTP, IMS_UINT32> m_objEmergencyCalls;
    IMSList<IMtsCallTrackerListener*> m_objListeners;
};

#endif
