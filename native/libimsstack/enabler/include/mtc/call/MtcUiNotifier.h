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

#ifndef MTC_UI_NOTIFIER_H_
#define MTC_UI_NOTIFIER_H_

#include "CallReasonInfo.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcUiNotifier.h"
#include <functional>

class AString;
class IMtcCallContext;
class IJniMtcCallThread;

class MtcUiNotifier final : public IMtcUiNotifier
{
public:
    explicit MtcUiNotifier(IN IMtcCallContext& objContext);
    virtual ~MtcUiNotifier();
    MtcUiNotifier(IN const MtcUiNotifier&) = delete;
    MtcUiNotifier& operator=(IN const MtcUiNotifier&) = delete;

    void SendPreIncomingCallReceived() override;
    void SendIncomingCallReceived() override;
    void SendIncomingCallRejected(IN const CallReasonInfo& objReason) override;
    void SendStarted() override;
    void SendStartFailed(IN const CallReasonInfo& objReason) override;
    void SendInitiating() override;
    void SendProgressing() override;
    void SendHeld() override;
    void SendHoldFailed(IN const CallReasonInfo& objReason) override;
    void SendResumed() override;
    void SendResumeFailed(IN const CallReasonInfo& objReason) override;
    void SendHeldBy() override;
    void SendResumedBy() override;
    void SendTerminated(IN const CallReasonInfo& objReason) override;
    void SendIncomingResume() override;
    void SendIncomingUpdate(IN CallType eCallTypeToUpdate) override;
    void SendUpdated() override;
    void SendUpdateFailed(IN const CallReasonInfo& objReason) override;
    void SendUpdatedBy() override;
    void SendNotifyInfo(IN IMS_UINT32 eType, IN const AString& strValue, IN IMS_SINT32 nValue,
            IN IMS_BOOL bValue) override;
    void SendReplacedBy(IN IMS_SINTP nKey, IN IMS_UINTP nType) override;
    void SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) override;
    void SendCallPushCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) override;
    void SendRatChanged(IN IMS_SINT32 eRatType) override;
    void OnCallSessionReleased() override;
    const CallReasonInfo GetStartFailedReason() const override;

private:
    IJniMtcCallThread* GetCallThread() const;

    IMtcCallContext& m_objContext;
    std::function<void()> m_objBlockedNotification;
    CallReasonInfo m_objStartFailedReason;
};

#endif
