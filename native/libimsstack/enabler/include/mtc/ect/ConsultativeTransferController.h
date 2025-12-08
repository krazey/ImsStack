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

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "ect/EctController.h"

class EctFactory;
class IEctControllerListener;
class IMtcContext;

/**
 * @brief This class is responsible for handling the consultative call transfer.
 * It inherits from EctController and implements the logic for a consultative transfer,
 * where a call is established with the target party before the transfer is completed.
 */
class ConsultativeTransferController : public EctController
{
public:
    /**
     * @brief Constructs a new {@link ConsultativeTransferController} object.
     *
     * @param objContext The MTC context.
     * @param nCallKey The key of the call to be transferred (transferee).
     * @param objListener The listener for ECT controller events.
     * @param objFactory The factory to create ECT related objects.
     */
    explicit ConsultativeTransferController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory);
    virtual ~ConsultativeTransferController() override;
    ConsultativeTransferController(IN const ConsultativeTransferController&) = delete;
    ConsultativeTransferController& operator=(IN const ConsultativeTransferController&) = delete;

    /**
     * @brief Initiates the consultative transfer process.
     */
    void Transfer() override;

protected:
    /**
     * @brief Overrides the base class method to handle successful transfer completion.
     */
    void OnSuccess() override;

private:
    /**
     * @brief Checks if the controller is in a valid state to perform the transfer.
     * @return IMS_BOOL Returns {@link IMS_TRUE) if valid, {@link IMS_FALSE} otherwise.
     */
    IMS_BOOL IsValid() const;

    /**
     * @brief Finds the transfer target call.
     *
     * This updates m_nTransferTargetKey with the found call key.
     */
    void FindTransferTarget();

    /**
     * @brief Terminates the call with the transfer target call after the transfer is complete.
     */
    void TerminateTransferTargetCall();

    CallKey m_nTransferTargetKey;  // The key for the call with the transfer target.
};

#endif
