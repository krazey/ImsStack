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

#ifndef MTS_TRAFFIC_H_
#define MTS_TRAFFIC_H_

#include "IImsRadio.h"
#include "IMtsTraffic.h"
#include "ITimer.h"
#include "MtsDef.h"

class IMtsContext;

class MtsTraffic final : public IMtsTraffic, public ITimerListener
{
public:
    MtsTraffic(IN IMtsContext& objContext, IN IMS_UINT32 nDirection, IN IMS_UINT32 nTrafficType,
            IN IMtsTrafficListener& objListener);
    ~MtsTraffic() override;

    // IImsRadioConnectionListener
    void ImsRadio_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void ImsRadio_OnConnectionSetupPrepared() override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // IMtsTraffic
    inline IMS_UINT32 GetDirection() const override { return m_nDirection; }
    inline IMS_UINT32 GetTrafficType() const override { return m_nTrafficType; }
    IMS_BOOL IsRadioGuardTimerActive() override;
    void StartRadioGuardTimer(IN IMS_UINT32 nDuration = MTS_RADIO_GUARD_TIMER_MS) override;

private:
    static IMS_BOOL IsReasonToIgnore(IN IMS_UINT32 nFailureReason);

private:
    IMtsContext& m_objContext;
    IMS_BOOL m_bDefaultGuardTimerStarted;
    IMS_UINT32 m_nDirection;
    IMS_UINT32 m_nTrafficType;
    IMtsTrafficListener& m_objMtsTrafficListener;
    ITimer* m_piRadioGuardTimer;
};

#endif
