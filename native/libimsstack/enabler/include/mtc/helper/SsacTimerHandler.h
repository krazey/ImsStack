/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef SSAC_TIMER_HANDLER_H_
#define SSAC_TIMER_HANDLER_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "helper/IMtcNetworkWatcherListener.h"
#include "helper/IPassiveTimerListener.h"
#include "helper/ISsacTimerHandler.h"

class IMtcContext;
enum class CallType;

class SsacTimerHandler final :
        public IPassiveTimerListener,
        public IMtcNetworkWatcherListener,
        public ISsacTimerHandler
{
public:
    explicit SsacTimerHandler(IN IMtcContext& objContext);
    virtual ~SsacTimerHandler();
    SsacTimerHandler(IN const SsacTimerHandler&) = delete;
    SsacTimerHandler& operator=(IN const SsacTimerHandler&) = delete;

    // IPassiveTimerListener implementation
    void OnPassiveTimerExpired(IN IPassiveTimerHolder::Type eType) override;

    // IMtcNetworkWatcherListener implementation
    void OnRatChanged(IN ServiceType eServiceType, IN IMS_SINT32 eOldRatType,
            IN IMS_SINT32 eRatType) override;

    // ISsacTimerHandler implementation
    IMS_BOOL IsSsacTimerRunning(IN CallType eCallType) const override;
    void StartBarringTimer(IN CallType eCallType) override;

private:
    void Clear();
    IMS_BOOL IsSsacTimerRunning() const;

    IMtcContext& m_objContext;
};

#endif
