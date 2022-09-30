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

class MtsErrorHandler final : public IMtsErrorHandler
{
public:
    MtsErrorHandler(IN ICarrierConfig* piCarrierConfig);
    virtual ~MtsErrorHandler();

    // IMtsErrorHandler
    IMS_SINT32 Handle(IN const IMessage* piMessage = IMS_NULL) override;
    inline void SetListener(IN IMtsErrorHandlerListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline IMtsErrorHandlerListener* GetListener() override { return m_piListener; }

private:
    IMS_SINT32 GetRegistrationRecoveryPolicy(IN const IMessage* piMessage) const;
    void ControlAos(IN IMS_UINT32 nCommand) const;

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

private:
    ICarrierConfig* m_piCarrierConfig;
    IMtsErrorHandlerListener* m_piListener;
};

#endif
