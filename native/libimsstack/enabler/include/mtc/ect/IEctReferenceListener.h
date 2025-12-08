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

#ifndef INTERFACE_ECT_REFERENCE_LISTENER_H_
#define INTERFACE_ECT_REFERENCE_LISTENER_H_

/**
 * @brief Listener interface for events from an EctReference.
 *
 * This interface is implemented by classes that need to monitor the status of a SIP REFER
 * transaction initiated for an Explicit Call Transfer (ECT).
 */
class IEctReferenceListener
{
public:
    virtual ~IEctReferenceListener() = default;

    /**
     * @brief Notifies the listener that the REFER transaction has started successfully.
     *
     * This is typically called after a successful response (e.g., 202 Accepted) to the initial
     * REFER request.
     */
    virtual void OnReferenceStarted() = 0;

    /**
     * @brief Notifies the listener that the REFER transaction failed to start.
     *
     * This can happen if the REFER request couldn't get a response or an immediate failure response
     * was received.
     */
    virtual void OnReferenceStartFailed() = 0;

    /**
     * @brief Notifies the listener of an update in the REFER subscription status.
     *
     * This is called when a NOTIFY message is received with a SIP fragment body.
     *
     * @param nSipFragCode The SIP status code from the body of the NOTIFY message.
     */
    virtual void OnReferenceUpdated(IN IMS_SINT32 nSipFragCode) = 0;
};

#endif
