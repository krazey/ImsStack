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

#ifndef MTS_TRM_H_
#define MTS_TRM_H_

#include "ITimer.h"
#include "ITrm.h"

class MtsTrmListener
{
public:
    virtual void Trm_PriorityChanged() = 0;
};

class MtsTrm final : public ITrmListener, public ITimerListener
{
public:
    MtsTrm(IN IMS_SINT32 nSlotId_);
    ~MtsTrm();
    static MtsTrm* GetInstance(IN IMS_SINT32 nSlotId);
    static void DestroyMtsTrm(IN IMS_SINT32 nSlotId);

    void AddListener(IN MtsTrmListener* piListener);
    void RemoveListener(IN MtsTrmListener* piListener);
    IMS_BOOL IsReady();
    IMS_BOOL IsTRMSupported();
    void Set(IN IMS_BOOL bStart);

private:
    void StartTimer(IN IMS_UINT32 nDuration);
    void StopTimer();

    void ProcessTimerExpired();

    // ITrmListener
    void Trm_NotifyServicePriorityChanged();

    // ITimerListener Interface
    void Timer_TimerExpired(IN ITimer* piTimer);

private:
    IMS_SINT32 m_nSlotId;
    IMS_BOOL m_bIsTrmSet;
    ITrm* m_piTrm;
    ITimer* m_piMtsTrmTimer;
    IMSList<MtsTrmListener*> m_objTrmListeners;

    static const IMS_UINT32 MTS_TRM_TIME_STOP_DELAY = 500;
};

#endif
