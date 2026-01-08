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

#ifndef MTS_ERROR_HANDLER_H_
#define MTS_ERROR_HANDLER_H_

#include "message/IMtsErrorHandler.h"

class ICarrierConfig;
class IMessage;
class IMtsContext;
class IMtsDynamicLoader;
class IMtsService;

class MtsErrorHandler final : public IMtsErrorHandler
{
public:
    explicit MtsErrorHandler(IN IMtsContext& objContext);
    ~MtsErrorHandler() override;

    IMS_SINT32 Handle(IN const IMtsService& objMtsService, IN IMtsMessage* piMtsMessage) override;
    inline IMS_SINT32 GetRetryAfterValue() const override { return m_nRetryAfterValue; }
    inline void ResetRetryAfterStatus() override
    {
        m_nCumulativeDuration = 0;
        m_nCurrentRetryCount = 0;
        m_nRetryAfterValue = 0;
    }

private:
    IMS_SINT32 GetRegistrationRecoveryPolicy(IN const IMessage* piMessage) const;

    IMS_SINT32 GetExpiryTimerFPolicy() const;

    IMS_SINT32 Get4xxResponsePolicy(IN const IMessage* piMessage) const;
    IMS_SINT32 Get403ResponsePolicy() const;
    IMS_SINT32 Get404ResponsePolicy() const;
    IMS_SINT32 Get406ResponsePolicy() const;
    IMS_SINT32 Get408ResponsePolicy() const;

    IMS_SINT32 Get5xxResponsePolicy(IN const IMessage* piMessage) const;
    IMS_SINT32 Get500ResponsePolicy() const;
    IMS_SINT32 Get503ResponsePolicy(IN const IMessage* piMessage) const;
    IMS_SINT32 Get504ResponsePolicy() const;

    void SetRetryAfterStatus(IN const IMS_SINT32 nRetryAfterValue);
    IMS_BOOL IsRetryPossible() const;
    IMS_BOOL IsRegisterWithNextPcscfRequired(IN const IMessage* piMessage) const;
    IMS_BOOL NeedToCheckRadioStatusForRetry(IN IMS_UINT32 nRetryCount) const;
    IMS_SINT32 EvaluateNetworkStatusForErrorCode(
            IN const IMtsService& objMtsService, IN const IMessage* piMessage) const;

private:
    IMtsContext& m_objContext;
    IMS_SINT32 m_nCumulativeDuration;
    IMS_SINT32 m_nCurrentRetryCount;
    IMS_SINT32 m_nRetryAfterValue;
    IMS_SINT32 m_nSlotId;
    ICarrierConfig* m_piCarrierConfig;
};

#endif
