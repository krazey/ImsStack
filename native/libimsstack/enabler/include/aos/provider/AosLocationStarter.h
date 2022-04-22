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
#ifndef AOS_LOCATION_STARTER_H_
#define AOS_LOCATION_STARTER_H_

#include "provider/AosFeature.h"
#include "interface/IAosLocationStarter.h"
#include "interface/IAosBlockListener.h"
#include "ITimer.h"
#include "IEventListener.h"

class IAosAppContext;

class AosLocationStarter
    : public AosFeature
    , public ITimerListener
    , public IEventListener
    , public IAosLocationStarter
    , public IAosBlockListener
{
public:
    AosLocationStarter();
    virtual ~AosLocationStarter();

    virtual IMS_SINT32 GetSlotId() const;
    virtual void SetSlotId(IN IMS_SINT32 nSlotId);

    virtual void Init(IN IAosAppContext* piContext,
            IN IMS_UINT32 nPolicy = POLICY_START_ON_WFC_AVAILABILITY);
    virtual void SetPolicy(IN IMS_UINT32 nPolicy,
            IN IMS_SINT32 nOperation = 0 /* (0: add, 1: remove) */);
    virtual IMS_BOOL IsPolicyEnabled(IN IMS_UINT32 nPolicy);

    virtual void AddBlockReason(IN BLOCK_REASON eReason,
            IN IMS_SINT32 nType = TYPE_VOLTE /* (0: VoLTE, 1: WFC) */);
    virtual void SetUpdateInterval(IN IMS_UINT32 nInterval);
    virtual void StartLocationInfoUpdate();
    virtual void StopLocationInfoUpdate();

private:
    virtual void Timer_TimerExpired(IN ITimer* piTimer);
    virtual void Event_NotifyEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam,
            IN IMS_UINT32 nLParam);

    virtual void Block_Changed(IN IMS_UINT32 nType = 0, IN IMS_UINT32 nParam = 0);

    virtual void OnFeatureEnabled(IN IMS_UINT32 nFeature);
    virtual void OnFeatureDisabled(IN IMS_UINT32 nFeature);

    void HandleStartConditionChanged();
    void Start();
    void Stop(IN IMS_UINT32 nDelayTime);
    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);

    enum
    {
        TIMER_STOP_DELAY = 0
    };

private:
    static const IMS_UINT32 DEFAULT_SHORT_UPDATE_INTERVAL = 300; // 5min
    static const IMS_UINT32 DEFAULT_STOP_DELAY = 30; // 30s

    IMS_SINT32 m_nSlotId;
    IMS_BOOL m_bInitialized;
    IMS_BOOL m_bWfcSetting;
    IMS_UINT32 m_nDefaultUpdateInterval;
    IMSList<IMS_UINT32> m_objVolteBlockReasons;
    IMSList<IMS_UINT32> m_objWfcBlockReasons;
    ITimer* m_piStopDelayTimer;
    IAosAppContext* m_piAppContext;

    AString m_strTag;
};

#endif // AOS_LOCATION_STARTER_H_
