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

/**
 * The interface used to get the SipMessage related update.
 */
public interface ISipTransportRemoteMessageListener {
    /**
     * Sends a new incoming SIP message to the message application
     * @param message received message from network
     * @param subId subscription Id.
     */
    public void onMessageReceived(@NonNull SipMessage message, int subId);

    /**
     * Notifies the remote application that a previous request to send a SIP
     * message using {@link sendMessage} has succeeded.
     * @param viaTransactionId The transaction ID found in the via header field of
     * the previously sent {@link SipMessage}
     * @param subId subscription Id.
     */
    public void onMessageSent(@NonNull String viaTransactionId, int subId);

    /**
     * Notifies the remote application that a previous request to send a SIP
     * message using {@link sendMessage} has failed.
     * @param viaTransactionId The Transaction ID found in the via header field of
     * the previously sent {@link SipMessage}.
     * @param sipReason The reason for the sip server connection failure.
     * @param subId subscription Id.
     */
    public void onMessageSendFailure(@NonNull String viaTransactionId, int sipReason, int subId);

}
