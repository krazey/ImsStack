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
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"

class ICoreService;
class IMtcContext;
class MtcCall;
class NullCall;
enum class ServiceType;

class MtcCallManager final : public IMtcCallManager
{
public:
    static NullCall* const s_pNullCall;

    explicit MtcCallManager(IN IMtcContext& objContext);
    virtual ~MtcCallManager() override;
    MtcCallManager(IN const MtcCallManager&) = delete;
    MtcCallManager& operator=(IN const MtcCallManager&) = delete;

    void Init();
    void DeInit();

    IMtcCall* CreateCall(IN ServiceType eServiceType, IN CallInfo& objCallInfo) override;
    void RemoveCall(IN CallKey nCallKey) override;

    IMtcCall* GetCallByCallKey(IN CallKey nCallKey) override;

    ImsList<IMtcCall*> GetCalls() override;
    ImsList<IMtcCall*> GetCallsExcluding(IN CallKey nExcludingCallKey) override;
    ImsList<IMtcCall*> GetCallsByType(IN CallType eCallType) override;
    ImsList<IMtcCall*> GetCallsByServiceType(IN ServiceType eServiceType) override;
    ImsList<IMtcCall*> GetCallsInConference() override;
    ImsList<IMtcCall*> GetCallsByState(IN State eState) override;

private:
    IMS_SINT32 GetFirstIndexByFilter(IN const std::function<IMS_BOOL(MtcCall*)>& objFilter);
    ImsList<IMtcCall*> GetCallsByFilter(IN const std::function<IMS_BOOL(MtcCall*)>& objFilter);

    IMtcContext& m_objContext;
    ImsList<MtcCall*> m_lstCalls;
};

#endif
