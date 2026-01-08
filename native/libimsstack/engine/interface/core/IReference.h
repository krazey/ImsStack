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
#ifndef INTERFACE_REFERENCE_H_
#define INTERFACE_REFERENCE_H_

#include "ByteArray.h"

#include "IServiceMethod.h"
#include "ISubscriptionState.h"
#include "SipStatusCode.h"

class INotificationListener;
class IReferenceListener;

/**
 * @brief This class provides an interface for referring a remote endpoint
 *        to a 3rd-party user or service.
 *
 * It can be created and received both inside and outside of a session.
 * For example, Call Transerfer, Call Forwarding, ...
 *
 * @see IServiceMethod, IReferenceListener
 */
class IReference : public IServiceMethod
{
protected:
    ~IReference() override = default;

public:
    /**
     * @brief Accepts an incoming reference request.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Accept() = 0;

    /**
     * @brief It connects a service method with this reference and allows the IMS engine to send
     *        notifications regarding this reference to the endpoint that initiated this reference.
     *
     * @param piServiceMethod Service method to be referred
     * @return If the method is successfully connected, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT ConnectReferMethod(IN IServiceMethod* piServiceMethod) = 0;

    /**
     * @brief Returns the reference method to be used.
     *
     * @return The reference method.
     */
    virtual const AString& GetReferMethod() const = 0;

    /**
     * @brief Returns the URI to refer to, optionally with a display name.
     *
     * @return The URI, optionally with a display name.
     */
    virtual const AString& GetReferToUserId() const = 0;

    /**
     * @brief Returns the value of the replaces parameter of the Refer-To header.
     *
     * The return value is the session-id in the contents of the o-line of the SDP (Session).
     *
     * @return The identifier for Replaces or null string if the value is not available.
     */
    virtual const AString& GetReplaces() const = 0;

    /**
     * @brief Returns the current state of this IReference.
     *
     * @return The current state of this IReference.\n
     *         #STATE_INITIATED\n
     *         #STATE_PROCEEDING\n
     *         #STATE_REFERRING\n
     *         #STATE_TERMINATED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Sends the reference request to the remote endpoint.
     *
     * The bImplicit argument indicates if this request should generate an implicit subscription.\n
     * If IMS_TRUE, the IReference transits to STATE_REFERRING and the sender gets reports
     * on the progress of the reference processing at the remote.\n
     * If IMS_FALSE, the sender will only know if the request was received.\n
     * The IReference transits to STATE_TERMINATED when the delivery report is received.
     *
     * @param bImplicitSubscription IMS_TRUE if this IReference should
     *                              generate an implicit subscription
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Refer(IN IMS_BOOL bImplicitSubscription) = 0;

    /**
     * @brief Rejects an incoming reference request.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Reject() = 0;

    /**
     * @brief Sets a listener for this IReference, replacing any previous IReferenceListener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN IReferenceListener* piListener) = 0;

    /**
     * @brief Sets the replaces parameter of the Refer-To header to the given identity.
     *
     * The application SHALL use the value of ISessionDescriptor::GetSessionId().
     *
     * @param strSessionId The identity of the ISession to be replaced
     * @return If it is successfully set, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetReplaces(IN const AString& strSessionId) = 0;

    /**
     * @brief Sends a successful final response to an incoming reference from a remote endpoint.
     *
     * The application SHALL use the value of ISessionDescriptor::GetSessionId().
     *
     * @param nStatusCode SIP status code
     * @param b100Trying Flag to indicate if the initial NOTIFY with 100 Trying response
                         needs to be sent or not
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AcceptEx(IN IMS_SINT32 nStatusCode = SipStatusCode::SC_202,
            IN IMS_BOOL b100Trying = IMS_TRUE) = 0;

    /**
     * @brief Sends the reference request to the remote endpoint.
     *
     * The bImplicit argument indicates if this request should generate an implicit subscription.\n
     * If IMS_TRUE, the IReference transits to STATE_REFERRING and the sender gets reports
     * on the progress of the reference processing at the remote.\n
     * If IMS_FALSE, the sender will only know if the request was received.\n
     * The IReference transits to STATE_TERMINATED when the delivery report is received.
     *
     * @param bImplicitSubscription IMS_TRUE if this IReference should
     *                              generate an implicit subscription
     * @param strHeadersForReferTo Additional header parameters
     *                             which are included in Refer-To header
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT ReferEx(IN IMS_BOOL bImplicitSubscription,
            IN const AString& strHeadersForReferTo = AString::ConstNull()) = 0;

    /**
     * @brief Sends a failure final response to an incoming reference from a remote endpoint.
     *
     * @param nStatusCode SIP status code
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode) = 0;

    /**
     * @brief Sends the notification (NOTIFY request w/ sipfrag)
     *        using the implicit subscription dialog.
     *
     * @param nSubState Subscription state\n
     *                  #ISubscriptionState#STATE_INIT\n
     *                  #ISubscriptionState#STATE_ACTIVE\n
     *                  #ISubscriptionState#STATE_PENDING\n
     *                  #ISubscriptionState#STATE_TERMINATED
     * @param objContent SIP message to be sent ("sipfrag")
     * @param nReason Reason code for "terminated" state\n
     *                #ISubscriptionState#REASON_DEACTIVATED\n
     *                #ISubscriptionState#REASON_PROBATION\n
     *                #ISubscriptionState#REASON_REJECTED\n
     *                #ISubscriptionState#REASON_TIMEOUT\n
     *                #ISubscriptionState#REASON_GIVEUP\n
     *                #ISubscriptionState#REASON_NORESOURCE
     * @param nExpires Expires parameter if required
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendNotification(IN IMS_SINT32 nSubState, IN const ByteArray& objContent,
            IN IMS_SINT32 nReason = ISubscriptionState::REASON_NONE,
            IN IMS_SINT32 nExpires = (-1)) = 0;

    /**
     * @brief Sets a notification listener for this IReference.
     *
     * @param piListener Listener to be set
     */
    virtual void SetNotificationListener(IN INotificationListener* piListener) = 0;

    /**
     * @brief Sets the flag if the mid-dialog request needs to be implicitly routed
     *        to the preloaded topmost route address.
     *
     * NOTE: It MUST be invoked when the route set (R-R headers) is not present,
     * and the mid-dialog request needs to be routed to the preloaded topmost
     * address (i.e. P-CSCF or SBC address).
     * This option will be applied when no Route header exists in the request.
     *
     * @param bFlag Indicates the flag if the implicit routing is required or not
     */
    virtual void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) = 0;

public:
    /// States of IReference
    enum
    {
        STATE_INITIATED = 1,
        STATE_PROCEEDING = 2,
        STATE_REFERRING = 3,
        STATE_TERMINATED = 4
    };
};

#endif
