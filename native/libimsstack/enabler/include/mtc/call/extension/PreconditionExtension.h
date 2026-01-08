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

#ifndef PRECONDITION_EXTENSION_H_
#define PRECONDITION_EXTENSION_H_

#include "ImsTypeDef.h"
#include "call/extension/MtcExtension.h"

class IMessage;
class IMtcCallContext;

/**
 * This class represents the precondition extension.
 */
class PreconditionExtension final : public MtcExtension
{
public:
    explicit PreconditionExtension(IN IMtcCallContext& objContext);
    PreconditionExtension(IN const PreconditionExtension& objRhs);
    virtual ~PreconditionExtension() override;
    PreconditionExtension& operator=(IN const PreconditionExtension&) = delete;

    IMtcExtension* Clone() const override;

    void FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest) override;
    void FormatResponse(IN ResponseType eType, IN_OUT IMessage& objResponse) override;
    void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) override;
    void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) override;

private:
    IMS_BOOL IsRequestIncludingOffer() const;
    IMS_BOOL HasSdpWithPrecondition(IN const IMessage& objMessage) const;
};

#endif
