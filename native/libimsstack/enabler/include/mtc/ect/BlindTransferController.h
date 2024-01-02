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

#ifndef BLIND_TRANSFER_CONTROLLER_H_
#define BLIND_TRANSFER_CONTROLLER_H_

#include "ect/EctController.h"

class IMtcContext;
class IMtcCall;
class IEctControllerListener;

class BlindTransferController : public EctController
{
public:
    explicit BlindTransferController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory);
    virtual ~BlindTransferController();
    BlindTransferController(IN const BlindTransferController&) = delete;
    BlindTransferController& operator=(IN const BlindTransferController&) = delete;

    // IEctReferenceListener implementation
    void OnReferenceStarted() override;

    void Transfer(IN const AString& strNumber) override;

private:
    IMS_BOOL IsValid() const;
};

#endif
