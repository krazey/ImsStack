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

#ifndef CONSULTATIVE_TRANSFER_CONTROLLER_H_
#define CONSULTATIVE_TRANSFER_CONTROLLER_H_

#include "ect/EctController.h"

class IMtcContext;
class IMtcCall;
class IEctControllerListener;

class ConsultativeTransferController : public EctController
{
public:
    explicit ConsultativeTransferController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory);
    virtual ~ConsultativeTransferController();
    ConsultativeTransferController(IN const ConsultativeTransferController&) = delete;
    ConsultativeTransferController& operator=(IN const ConsultativeTransferController&) = delete;

    void Transfer() override;

protected:
    void OnCompleted() override;

private:
    IMS_BOOL IsValid() const;
    void FindTransferTarget();
    void TerminateTransferTargetCall();

    CallKey m_nTransferTargetKey;
};

#endif
