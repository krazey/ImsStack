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

#ifndef CALL_STATE_PROXY_H_
#define CALL_STATE_PROXY_H_

#include "IMtcCallStateListener.h"
#include "ImsActivity.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "helper/ICallStateProxy.h"

class IMtcCallManager;
struct CallInfo;

struct CallStateDetails
{
public:
    inline CallStateDetails(IN CallKey _nCallKey, IN IMtcCallStateListener::State _eState,
            IN IMtcCallStateListener::Type _eType, IN IMS_BOOL _bEmergency,
            IN IMS_SINT32 _nReason) :
            nCallKey(_nCallKey),
            eState(_eState),
            eType(_eType),  // for Aos, java.
            bEmergency(_bEmergency),
            nReason(_nReason)  // for CallInfoService.java
    {
    }
    inline virtual ~CallStateDetails() {}

private:
    CallStateDetails(IN const CallStateDetails&) = delete;
    CallStateDetails& operator=(IN const CallStateDetails&) = delete;

public:
    CallKey nCallKey;
    IMtcCallStateListener::State eState;
    IMtcCallStateListener::Type eType;
    IMS_BOOL bEmergency;
    IMS_SINT32 nReason;  // enum class????
};

class CallStateProxy final : public ImsActivity, public ICallStateProxy
{
public:
    CallStateProxy(IN IMtcCallManager& objCallManager);
    virtual ~CallStateProxy();
    CallStateProxy(IN const CallStateProxy&) = delete;
    CallStateProxy& operator=(IN const CallStateProxy&) = delete;

public:
    // ImsActivity implementation
    IImsActivityController* GetController() override { return IMS_NULL; }

    void AddListener(IN IMtcCallStateListener* pListener) override;
    void RemoveListener(IN IMtcCallStateListener* pListener) override;

    void UpdateCallState(IN CallKey nCallkey, IN IMtcCall::State eState, IN CallType eCallType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason = CODE_NONE) override;

    IMS_BOOL DispatchMessage(IN IMSMSG& objMsg) override;

private:
    IMS_BOOL UpdateTotalCallState();
    IMtcCall::State CalculateTotalCallState();

    void NotifyToListeners(IN IMS_BOOL bSynchronous, IN CallStateDetails* pDetails,
            IN IMS_BOOL bTotalCallStateUpdated);
    void NotifyCallState(
            IN ImsList<IMtcCallStateListener*> objListeners, IN CallStateDetails* pDetails);
    void NotifyTotalCallState(IN ImsList<IMtcCallStateListener*> objListeners);

private:
    ImsList<IMtcCallStateListener*> m_objSynchronousListeners;
    ImsList<IMtcCallStateListener*> m_objAsynchronousListeners;
    IMtcCallManager& m_objCallManager;
    IMtcCall::State m_eTotalState;
    static const IMS_UINT32 MESSAGE_ASYNC_NOTIFY = 0;
};

#endif
