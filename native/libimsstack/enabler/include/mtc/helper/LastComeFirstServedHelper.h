/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef LAST_COME_FIRST_SERVED_HELPER_H_
#define LAST_COME_FIRST_SERVED_HELPER_H_

#include "ImsTypeDef.h"
#include "helper/ILastComeFirstServedHelper.h"

class IMtcCallContext;
class IMtcContext;
class MtcConfigurationProxy;

class LastComeFirstServedHelper final : public ILastComeFirstServedHelper
{
public:
    explicit LastComeFirstServedHelper(IN IMtcContext& objContext);
    virtual ~LastComeFirstServedHelper();
    LastComeFirstServedHelper(IN const LastComeFirstServedHelper&) = delete;
    LastComeFirstServedHelper& operator=(IN const LastComeFirstServedHelper&) = delete;

    static IMS_BOOL IsSupported(IN const MtcConfigurationProxy& objConfigurationProxy);
    void OnCallReceived(IN CallKey nIncomingCallKey) override;

private:
    IMS_BOOL IsNormalCall(IN CallKey nKey) const;
    IMtcCallContext& GetCallContext(IN CallKey nKey) const;
    IMtcCall* GetPreExistingIncomingCall() const;
    IMS_SINT32 GetRejectReasonCode(IN IMtcCallContext& objCallContext) const;
    void StartPreAlertingGuardTimer() const;
    inline void SetNetworkInfoOfIncomingCall(IN IMS_BOOL bOnNr)
    {
        m_bPreExistingIncomingCallOnNr = bOnNr;
    }

    IMtcContext& m_objContext;
    IMS_BOOL m_bPreExistingIncomingCallOnNr;
};

#endif
