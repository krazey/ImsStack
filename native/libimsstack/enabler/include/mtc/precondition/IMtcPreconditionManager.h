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

#ifndef INTERFACE_MTC_PRECONDITION_MANAGER_H_
#define INTERFACE_MTC_PRECONDITION_MANAGER_H_

#include "ImsTypeDef.h"
#include "precondition/QosDef.h"

class ISession;
class IMessage;
class IMtcPreconditionListener;

class IMtcPreconditionManager
{
public:
    virtual ~IMtcPreconditionManager(){};

    /**
     * @brief Create QosTimer, QosData, QosStatusTable to handle the behavior related to
     *        precondition.
     * @param piSession ISession instance is used for managing the QosTimer, QosData, and
     *                  QosStatusTable.
     */
    virtual void CreateQos(IN ISession* piSession) = 0;

    /**
     * @brief Destroy the QosTimer, QosData, QosStatusTable matched with the param. Remove them
     *        from the list or map managed in the PreconditionManager.
     * @param piSession Isession instance is used to find the QosTimer, QosData and QosStatusTable
     *                  should be deleted.
     */
    virtual void DestroyQos(IN ISession* piSession) = 0;

    /**
     * @brief Set callback listener from MtcCall.
     * @param pListener callback listener to set.
     */
    virtual void SetListener(IN IMtcPreconditionListener* pListener) = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsPreconditionSupportedInLocal() const = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     * @param eMediaType
     * @return IMS_BOOL
     */
    virtual IMS_BOOL IsDedicatedBearerAllocated(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) const = 0;

    /**
     * @brief Checks
     *
     * @return IMS_BOOL
     */
    virtual IMS_BOOL IsCheckingResourcesRequiredToAlertUser() const = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     * @return IMS_BOOL
     */
    virtual IMS_BOOL IsAvailableToAlertUser(IN ISession* piSession) const = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     * @return IMS_BOOL
     */
    virtual IMS_BOOL IsLocalResourceConfirmationRequired(IN ISession* piSession) const = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     * @return IMS_BOOL
     */
    virtual IMS_BOOL IsAvailableToSendLocalResourceConfirmation(IN ISession* piSession) const = 0;

    /**
     * @brief To form the precondition attributes of SDP.
     * @param piSession ISession instance to get the QosStatusTable and the SDP body of the
     *                  specific session.
     * @param bFailure Flag to form the SDP for failure case or not.
     */
    virtual void FormPreconditionSdp(IN ISession* piSession, IN IMS_BOOL bFailure) = 0;

    /**
     * @brief Handles
     *
     */
    virtual void HandleQosOnIpcanChanged() = 0;

    /**
     * @brief Updates
     *
     * @param piSession
     * @param piMessage
     */
    virtual void OnSdpReceived(IN ISession* piSession, IN IMessage* piMessage) = 0;

    /**
     * @brief Updates
     *
     * @param piSession
     * @param piMessage
     */
    virtual void OnMessageReceived(IN ISession* piSession, IN IMessage* piMessage) = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     */
    virtual void OnCallEstablished(IN ISession* piSession) = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     */
    virtual void OnCallModified(IN ISession* piSession) = 0;
};

#endif
