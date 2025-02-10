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

#ifndef SILENT_REDIAL_HELPER_H_
#define SILENT_REDIAL_HELPER_H_

#include "AString.h"
#include "CallReasonInfo.h"
#include "IMtcCallStateListener.h"
#include "ITimer.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/ISilentRedialHelper.h"

class IMtcCallContext;

class SilentRedialHelper :
        public ISilentRedialHelper,
        public IMtcCallStateListener,
        public ITimerListener
{
public:
    SilentRedialHelper(IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason);
    virtual ~SilentRedialHelper();
    SilentRedialHelper(IN const SilentRedialHelper&) = delete;
    SilentRedialHelper& operator=(IN const SilentRedialHelper&) = delete;

    inline IMS_BOOL IsSameRedialType(IN const CallReasonInfo& objReason) const
    {
        return objReason.nExtraCode == m_nType;
    }

    // ISilentRedialHelper implementation
    IMS_RESULT Redial(IN IMS_SINT32 nIntervalInMillis = INTERVAL_BY_TYPE) override;
    inline IMS_UINT32 GetType() override { return m_nType; }

    // IMtcCallStateListener implementation
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    inline void OnTotalCallStateChanged(IN State) override {};
    // must be true. Otherwise, Context will be null when State is changed to Terminating
    inline IMS_BOOL IsSynchronousCallRequired() override { return IMS_TRUE; }

    void Timer_TimerExpired(IN ITimer* piTimer) override;

private:
    void ReStart();
    void SetRedialDetail();
    void LoadRetryLimitsFromConfiguration();
    void ReleaseCallResources();
    void StopCallTimers();
    IMS_BOOL IsRedialAvailable() const;
    CallType GetCallType() const;
    const AString GetRemoteTarget() const;

    IMtcCallContext& m_objContext;
    CallKey m_nCallKey;
    IMS_SINT32 m_nType;
    IMS_SINT32 m_nMaxDuration;
    IMS_SINT32 m_nInterval;
    IMS_SINT32 m_nMaxCount;
    IMS_SINT32 m_nCount;
    const AString m_strExtra;
    ITimer* m_piTimer;

    LOCAL const IMS_UINT32 NO_LIMIT = 999;
};

#endif
