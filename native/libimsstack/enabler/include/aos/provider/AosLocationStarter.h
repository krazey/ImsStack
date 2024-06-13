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

class AosLocationStarter :
        public AosFeature,
        public ITimerListener,
        public IEventListener,
        public IAosLocationStarter,
        public IAosBlockListener
{
public:
    AosLocationStarter();
    virtual ~AosLocationStarter();

    IMS_SINT32 GetSlotId() const override;
    void SetSlotId(IN IMS_SINT32 nSlotId) override;

    void Init(IN IAosAppContext* piContext,
            IN IMS_UINT32 nPolicy = POLICY_START_ON_WFC_AVAILABILITY) override;
    IMS_BOOL SetPolicy(
            IN IMS_UINT32 nPolicy, IN IMS_SINT32 nOperation = 0 /* (0:add, 1:remove) */) override;
    IMS_BOOL IsPolicyEnabled(IN IMS_UINT32 nPolicy) override;

    void AddBlockReason(IN BLOCK_REASON eReason,
            IN IMS_SINT32 nType = TYPE_VOLTE /* (0: VoLTE, 1: WFC) */) override;
    IMS_BOOL SetUpdateInterval(IN IMS_UINT32 nInterval) override;
    void StartLocationInfoUpdate() override;
    void StopLocationInfoUpdate() override;

protected:
    void Timer_TimerExpired(IN ITimer* piTimer) override;
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    void Block_Changed(IN IMS_UINT32 nType = 0, IN IMS_UINT32 nParam = 0) override;

    void OnFeatureEnabled(IN IMS_UINT32 nFeature) override;
    void OnFeatureDisabled(IN IMS_UINT32 nFeature) override;

    void HandleStartConditionChanged();
    void Start();
    void Stop(IN IMS_UINT32 nDelayTime);
    IMS_BOOL StartDelayTimer(IN IMS_UINT32 nDuration);
    IMS_BOOL StopDelayTimer();

public:
    static const IMS_UINT32 DEFAULT_SHORT_UPDATE_INTERVAL = 300;  // 5min
    static const IMS_UINT32 DEFAULT_STOP_DELAY = 30;              // 30s

protected:
    IMS_SINT32 m_nSlotId;
    IMS_BOOL m_bInitialized;
    IMS_BOOL m_bWfcSetting;
    IMS_UINT32 m_nDefaultUpdateInterval;
    ImsList<IMS_UINT32> m_objVolteBlockReasons;
    ImsList<IMS_UINT32> m_objWfcBlockReasons;
    ITimer* m_piStopDelayTimer;
    IAosAppContext* m_piAppContext;

    AString m_strTag;
};

#endif  // AOS_LOCATION_STARTER_H_
