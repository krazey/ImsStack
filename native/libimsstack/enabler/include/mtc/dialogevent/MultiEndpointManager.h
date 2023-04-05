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

#ifndef MULTI_ENDPOINT_MANAGER_H_
#define MULTI_ENDPOINT_MANAGER_H_

#include "ICarrierConfigListener.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "dialogevent/IDialogInfoManager.h"
#include "dialogevent/IDialogSubscription.h"
#include "dialogevent/IMultiEndpointManager.h"
#include "dialogevent/MultiEndpointFactory.h"
#include "helper/IMtcAosStateListener.h"
#include <memory>

class AString;
class IMtcContext;
class IMtcService;
struct JniExternalCall;

class MultiEndpointManager final :
        public IMultiEndpointManager,
        public IMtcAosStateListener,
        public ICarrierConfigListener,
        public IDialogSubscriptionListener
{
public:
    explicit MultiEndpointManager(
            IN IMtcContext& objContext, IN std::unique_ptr<MultiEndpointFactory> pFactory);
    virtual ~MultiEndpointManager();
    MultiEndpointManager(IN const MultiEndpointManager&) = delete;
    MultiEndpointManager& operator=(IN const MultiEndpointManager&) = delete;

    static IMS_BOOL IsRequired(IN const MtcConfigurationProxy& objConfigProxy);

    PullingDialogInfo GetDialogInfo(IN const AString& strTarget) const override;

    void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason) override;
    void OnIpcanChanged(IN IMtcService& objMtcService, IN IMS_UINT32 eIpcan) override;

    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    void OnSubscriptionStarted() override;
    void OnSubscriptionStartFailed() override;
    void OnSubscriptionTerminated() override;
    void OnSubscriptionNotified(IN const AString& strBody) override;

    // Visible for test.
    IMS_BOOL IsRunning() const;

private:
    void HandleConditionChanged();
    IMS_BOOL IsReady() const;
    void Start();
    void Stop();
    void NotifyExternalCalls() const;

    ImsList<IN const JniExternalCall*> GetJniExternalCalls() const;
    IMS_BOOL IsPullable(IN const Dialog& objDialog) const;
    CallType GetCallType(IN const Dialog& objDialog) const;
    IMS_BOOL IsHeld(IN const Dialog& objDialog) const;
    IMS_BOOL IsOwnDialog(IN const Dialog& objDialog) const;
    IMS_BOOL IsEarlyState(IN const Dialog& objDialog) const;

    IMtcContext& m_objContext;
    std::unique_ptr<MultiEndpointFactory> m_pFactory;
    std::unique_ptr<IDialogInfoManager> m_piDialogInfoManager;
    std::unique_ptr<IDialogSubscription> m_piDialogSubscription;
};

#endif
