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
#ifndef AOS_TRM_H_
#define AOS_TRM_H_

#include "ITimer.h"
#include "ITrm.h"
#include "interface/IAosTrm.h"

class AosTrm
    : public IAosTrm
    , public ITrmListener
    , public ITimerListener
{
public:
    AosTrm(IN IMS_SINT32 nSlotId_);
    virtual ~AosTrm();

    virtual void SetListener(IN IAosTrmListener *piListener);
    virtual void RemoveListener(IN IAosTrmListener *piListener);

    virtual IMS_BOOL IsReady();
    virtual IMS_BOOL IsTRMSupported();
    virtual void Set(IN IMS_UINT32 nType, IN IMS_BOOL bStart);
    virtual void SetEmegency(IN IMS_UINT32 nType, IN IMS_BOOL bStart);
    virtual void SetIPCAN(IN IN IMS_UINT32 nCategory);

private:
    void Start(IN IMS_UINT32 nType);
    IMS_BOOL IsStarted();
    IMS_BOOL IsStarted(IN IMS_UINT32 nType);
    void Stop(IN IMS_UINT32 nType);

    void StartTimer(IN IMS_UINT32 nDuration);
    void StopTimer();

    virtual void ProcessTimerExpired();

    // ITrmListener
    virtual void Trm_NotifyServicePriorityChanged();

    // ITimerListener Interface
    virtual void Timer_TimerExpired(IN ITimer *piTimer);

private:
    IMS_SINT32 nSlotId;
    IMS_UINT32 nServices;
    IMS_UINT32 nIPCANCategory;
    IMS_BOOL bIsStartUpdated;
    IMS_BOOL bIsEmergencyStartUpdated;

    ITrm *piTRM;
    ITimer *piStopTimer;
    IMSList<IAosTrmListener*> objListeners;

    AString strTag;

    static const IMS_UINT32 TIME_STOP_DELAY = 1000;
};
#endif // AOS_TRM_H_
