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
     * @brief Get QoS status according to the specific ISession and check type - local, remote,
     *        or all.
     * @param piSession ISession instance to find QosStatus for the specific session.
     * @param eType The type to check for the QosStatus in the side of local, remote, or all
     * @return returns QosStatus in the given ISession.
     */
    virtual IMS_BOOL IsResourceReserved(IN ISession* piSession, IN QosCheckType eType) = 0;

    /**
     * @brief Start the timer related to QoS.
     * @param piSession ISession instance to find QoS timer for the specific session.
     * @param eType The type of QoS timer to start.
     */
    virtual void StartQosTimer(
            IN ISession* piSession, IN QosTimerType eType = QosTimerType::WAIT_AVAILABLE) = 0;

    /**
     * @brief Stop the timer related to QoS.
     *
     * @param piSession ISession instance to find QoS timer for the specific session.
     * @param eType The type of QoS timer to stop.
     */
    virtual void StopQosTimer(
            IN ISession* piSession, IN QosTimerType eType = QosTimerType::WAIT_AVAILABLE) = 0;

    /**
     * @brief Stop all the timers related to QoS.
     *
     * @param piSession ISession instance to find QoS timer for the specific session.
     */
    virtual void StopAllQosTimer(IN ISession* piSession) = 0;

    /**
     * @brief Update the capability for precondition through the value of Supported and Required
     *        header field from the received SIP message from the specific session.
     * @param piSession ISession instance to manage the capability for the specific session.
     * @param bCapability Flag to IMS_TRUE if there is precondition value from Supported or
     *                    Required header field.
     */
    virtual void UpdatePreconditionCapability(IN ISession* piSession, IN IMS_BOOL bCapability) = 0;

    /**
     * @brief To check if there are capability to use precondition or not.
     * @return If there is capability, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL HasPreconditionCapability(IN ISession* piSession) = 0;

    virtual IMS_BOOL IsPreconditionSupportedInLocal() = 0;

    /**
     * @brief This method is to update precondition attributes from the QosData and the received
     *        SDP. Also if there is additional media line in the SDP, it creates QosStatusRecord
     *        for that. And it removes QosStatusRecord of unused media.
     * @param piSession ISession instance to find the QosData and QosStatusTable.
     */
    virtual void UpdateQosAttributesFromSdp(IN ISession* piSession) = 0;

    /**
     * @brief To form the precondition attributes of SDP.
     * @param piSession ISession instance to get the QosStatusTable and the SDP body of the
     *                  specific session.
     * @param bFailure Flag to form the SDP for failure case or not.
     */
    virtual void FormPreconditionSdp(IN ISession* piSession, IN IMS_BOOL bFailure) = 0;

    /**
     * @brief Remove precondition attributes in the SDP body.
     * @param piSession ISession instance to get SDP body for the specific session.
     */
    virtual void RemovePreconditionSdp(IN ISession* piSession) = 0;

    /**
     * @brief Enable the current status of the local side by force. It can be called if the call is
     *        on WiFi because it couldn't receive the QoS status from the network.
     * @param piSession ISession instance to find the QosStatusTable.
     * @return Returns the media types which are enabled.
     */
    virtual IMS_UINT32 SetLocalResourceAvailable(IN ISession* piSession) = 0;

    /**
     * @brief Enable the current status of the remote side. It should be called when the UE
     *        receives 180 Ringing or 200 OK response to INVITE SIP request without received
     *        precondition information in the past.
     * @param piSession ISession instance to find the QosStatusTable.
     */
    virtual void SetRemoteResourceAvailable(IN ISession* piSession) = 0;
};

#endif
