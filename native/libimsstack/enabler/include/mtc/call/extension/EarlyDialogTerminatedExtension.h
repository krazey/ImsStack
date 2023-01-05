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

#ifndef EARLY_DIALOG_TERMINATED_EXTENSION_H_
#define EARLY_DIALOG_TERMINATED_EXTENSION_H_

#include "ImsTypeDef.h"
#include "call/extension/MtcExtension.h"

class IMessage;

/**
 * This class represents the 199 extension.
 */
class EarlyDialogTerminatedExtension final : public MtcExtension
{
public:
    explicit EarlyDialogTerminatedExtension();
    explicit EarlyDialogTerminatedExtension(IN const EarlyDialogTerminatedExtension& objRhs);
    virtual ~EarlyDialogTerminatedExtension();
    EarlyDialogTerminatedExtension& operator=(IN const EarlyDialogTerminatedExtension&) = delete;

    IMtcExtension* Clone() const override;

    void FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest) override;
};

#endif
