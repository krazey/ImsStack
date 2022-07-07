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
     * @param subId subscription Id.
     */
    void sendMessage(@NonNull SipMessage message, long configVersion, int subId);
    /**
     * Close the ongoing session
     * @param callId id for which session to be closed
     * @param subId subscription Id.
     */
    void closeOngoingSession(@NonNull String callId, int subId);
    /**
     * Sip message is not received due to some error
     * @param viaTransactionId of the message which is not received by the app
     * @param subId subscription Id.
     */
    void notifyMessageReceiveError(@NonNull String viaTransactionId, int subId);
    /**
     * Add sip transport listener to get the update from IMS core.
     * @param listener object used to notify the events
     * @param subId subscription Id.
     */
    void setSipTransportListener(@NonNull SipTransportRemoteListener listener ,
            int subId);

     /**
     * Called by the framework to request that the ImsService perform the network
     * registration of all SIP delegates associated with this ImsService.
     * @param featureTags The RCS FeatureTags for Registration
     * @param subId subscription Id.
     */
    void updateSipDelegateRegistration(@NonNull Set<String> featureTags, int subId);
}
