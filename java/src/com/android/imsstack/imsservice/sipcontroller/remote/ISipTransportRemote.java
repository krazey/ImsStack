/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.android.imsstack.imsservice.sipcontroller.remote;

import android.annotation.NonNull;
import android.telephony.ims.SipMessage;

import java.util.Set;

/**
 * The interface to send the sip transport requests to IMS stack.
 */
public interface ISipTransportRemote {

    /**
     * Send the SipMessage requested by app
     * @param message sip message to send
     * @param configVersion configuration version used by client
     */
    void sendMessage(@NonNull SipMessage message, long configVersion);

    /**
     * Close the ongoing session
     * @param callId id for which session to be closed
     */
    void closeOngoingSession(@NonNull String callId);

    /**
     * The remote application has received the SIP message and is processing it.
     * @param viaTransactionId The Transaction ID found in the via header field of the
     *                         previously sent {@link SipMessage}.
     */
    void notifyMessageReceived(@NonNull String viaTransactionId);

    /**
     * Sip message is not received due to some error
     * @param viaTransactionId of the message which is not received by the app
     * @param reason The reason why the message was not correctly received.
     */
    void notifyMessageReceiveError(@NonNull String viaTransactionId, int reason);

    /**
     * Add sip transport listener to get the update from IMS core.
     * @param listener object used to notify the events
     */
    void setSipTransportListener(@NonNull SipTransportRemoteListener listener);

     /**
     * Called by the framework to request that the ImsService perform the network
     * registration of all SIP delegates associated with this ImsService.
     * @param featureTags The RCS FeatureTags for Registration
     */
    void updateSipDelegateRegistration(@NonNull Set<String> featureTags);

    /**
     * Called by the framework to request that the ImsService perform the network
     * deregistration of all SIP delegates associated with this ImsService.
     * <p>
     * This is typically called in situations where the user has changed the
     * configuration of the device (for example, the default messaging application)
     * and the framework is reconfiguring the tags associated with each IMS
     * application.
     * <p>
     * This should not affect the registration of features managed by the
     * ImsService itself, such as feature tags related to MMTEL registration.
     */
    void triggerSipDelegateDeRegistration();

    /**
     * Release from SipController(Java) if the delegate is terminated or Jni is not used.
     * @param slotId The slot ID to be removed
     */
    void release(int slotId);
}
