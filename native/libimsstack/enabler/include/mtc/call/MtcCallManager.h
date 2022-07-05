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

#ifndef MTC_CALL_MANAGER_H_
#define MTC_CALL_MANAGER_H_

#include "ImsList.h"
#include "IMSTypeDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"
#include "IMtcService.h"
#include "IuMtcService.h"
#include "IMtcCallStateListener.h"

class ICoreService;
class IMtcContext;
class MtcCall;
class NullCall;

class MtcCallManager final : public IMtcCallManager, public IMtcCallStateListener
{
public:
    static NullCall* const s_pNullCall;

    MtcCallManager(IN IMtcContext& objContext);
    virtual ~MtcCallManager();
    MtcCallManager(IN const MtcCallManager&) = delete;
    MtcCallManager& operator=(IN const MtcCallManager&) = delete;

    void Init();

    IMtcCall* CreateCall(IN ServiceType eServiceType, IN CallInfo& objCallInfo) override;
    void RemoveCall(IN CallKey nCallKey) override;

    IMtcCall* GetCallByCallKey(IN CallKey nCallKey) override;

    IMSList<IMtcCall*> GetCalls() override;
    IMSList<IMtcCall*> GetCallsExcluding(IN CallKey nExcludingCallKey) override;
    IMSList<IMtcCall*> GetCallsByType(IN CallType eCallType) override;
    IMSList<IMtcCall*> GetCallsByServiceType(IN ServiceType eServiceType) override;
    IMSList<IMtcCall*> GetCallsInConference() override;
    IMSList<IMtcCall*> GetCallsByState(IN State eState) override;

    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State eState) override;

private:
    IMS_SINT32 GetFirstIndexByFilter(IN std::function<IMS_BOOL(MtcCall*)> objFilter);
    IMSList<IMtcCall*> GetCallsByFilter(IN std::function<IMS_BOOL(MtcCall*)> objFilter);

    IMtcContext& m_objContext;
    IMSList<MtcCall*> m_lstCalls;
};

#endif
