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

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "ect/EctController.h"

class AString;
class EctFactory;
class IEctControllerListener;
class IMtcContext;

/**
 * @brief This class is responsible for handling the blind call transfer.
 *
 * It inherits from EctController and implements the logic for a blind transfer.
 */
class BlindTransferController : public EctController
{
public:
    /**
     * @brief Constructs a new {@link BlindTransferController} object
     *
     * @param objContext The MTC context.
     * @param nCallKey The key of the call to be transferred.
     * @param objListener The listener for ECT controller events.
     * @param objFactory The factory to create ECT related objects.
     */
    explicit BlindTransferController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory);
    virtual ~BlindTransferController() override;
    BlindTransferController(IN const BlindTransferController&) = delete;
    BlindTransferController& operator=(IN const BlindTransferController&) = delete;

    /** See {@link IEctReferenceListener#OnReferenceStarted}. */
    void OnReferenceStarted() override;

    /**
     * @brief Initiates a blind transfer of the call to the specified number.
     *
     * @param strNumber The target number for the transfer.
     */
    void Transfer(IN const AString& strNumber) override;

private:
    IMS_BOOL IsValid() const;
};

#endif
