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

import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_INVALID_HEADER_FIELDS;

import android.annotation.NonNull;
import android.telephony.ims.DelegateRegistrationState;
import android.telephony.ims.SipDelegateConfiguration;
import android.telephony.ims.SipMessage;
import android.util.Log;

import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.imsservice.sipcontroller.sipdelegate.ActiveSipDelegateManager;
import com.android.imsstack.imsservice.sipcontroller.sipdelegate.SipDelegateImpl;
import com.android.internal.telephony.SipMessageParsingUtils;

import java.util.Set;

/**
 * The sip transport listener used to listen events from the IMS core.
 */
public class SipTransportRemoteListener implements ISipTransportRemoteMessageListener,
        ISipTransportRemoteConfigListener, ISipTransportRemoteRegistrationListener {
    public static final String LOG_TAG = "SipTransportRemoteListener";
    /**
     * Sends a new incoming SIP message to the message application
     * @param message received message from network
     * @param subId subscription Id.
     */
    @Override
    public void onMessageReceived(@NonNull SipMessage message, int subId) {
        Log.i(LOG_TAG, "onMessageReceived: " + subId);
        //How to know for which sip delegate this message belongs.
        //Case 1: Use the call id for ongoing active dialog session.
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(subId);
        ActiveSipDelegateManager activeDelegateManager;
        if (serviceRecord != null && message != null) {
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            //Try to get sip delegate for ongoing session for this callid
            SipDelegateImpl sipDelegateToBeNotified =
                    activeDelegateManager.getSipDelegateForCallId(message.getCallIdParameter());

            if( sipDelegateToBeNotified == null ) {
                Log.i(LOG_TAG, "Not found sip delegate for callid: OutOfDialogMsg handling");
                //Case 2: Try to get the sip delegate which can handle this feature tag
                Set<String> featureTag = SipMessageParsingUtils.
                        getAcceptContactFeatureTags(message.getHeaderSection());
                Log.i(LOG_TAG, "Check feature tag:" + featureTag);
                sipDelegateToBeNotified =
                        activeDelegateManager.getSipDelegateForSupportedFeatureTag(featureTag);
            }
            if( sipDelegateToBeNotified != null ) {
                Log.i(LOG_TAG, "Found the sip delegate to notify received message"
                        + sipDelegateToBeNotified);
                //Notify the sip delegate that message is received.
                sipDelegateToBeNotified.onMessageReceived(message);
            } else {
                Log.e(LOG_TAG, " Not Found the sip delegate to notify received message");
                ISipTransportRemote nativeSipController =
                        serviceRecord.getSipTransport().getISipTransportRemote();
                if (nativeSipController != null) {
                    nativeSipController.notifyMessageReceiveError(message.getViaBranchParameter(),
                            MESSAGE_FAILURE_REASON_INVALID_HEADER_FIELDS);
                } else {
                    Log.d(LOG_TAG, "nativeSipController is null");
                }
            }
        } else {
            Log.e(LOG_TAG, "onSipMessageReceived:service record not present for slot:"
                    + subId);
        }
    }
    /**
     * Notifies the remote application that a previous request to send a SIP
     * message using {@link android.telephony.ims.stub.SipDelegate#sendMessage} has succeeded.
     * @param viaTransactionId The transaction ID found in the via header field of
     * the previously sent {@link SipMessage}
     * @param subId subscription Id.
     */
    @Override
    public void onMessageSent(@NonNull String viaTransactionId, int subId) {
        Log.i(LOG_TAG, "onMessageSent:" + viaTransactionId);
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(subId);
        ActiveSipDelegateManager activeDelegateManager;
        if (serviceRecord != null) {
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            //Try to get sip delegate for ongoing session for this callid
            SipDelegateImpl sipDelegateToBeNotified = activeDelegateManager.
                    getSipDelegateForTransactionId(viaTransactionId);
            if(sipDelegateToBeNotified != null) {
                Log.i(LOG_TAG, "Found the sip delegate to notify received message"
                        + sipDelegateToBeNotified);
                //Notify the sip delegate that message is received.
                 sipDelegateToBeNotified.onMessageSent(viaTransactionId);
            } else {
                Log.e(LOG_TAG, " Not Found the sip delegate to notify send msg status");
            }
        } else {
            Log.e(LOG_TAG, "onSipMessageSendStatus:service record not present for subId:"
                    + subId);
        }
    }
    /**
     * Notifies the remote application that a previous request to send a SIP
     * message using {@link android.telephony.ims.stub.SipDelegate#sendMessage} has failed.
     * @param viaTransactionId The Transaction ID found in the via header field of
     * the previously sent {@link SipMessage}.
     * @param sipReason The reason for the sip server connection failure.
     * @param subId subscription Id.
     */
    @Override
    public void onMessageSendFailure(@NonNull String viaTransactionId, int sipReason, int subId) {
        Log.i(LOG_TAG, "onMessageSendFailure:" + viaTransactionId);
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(subId);
        ActiveSipDelegateManager activeDelegateManager;
        if (serviceRecord != null) {
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            //Try to get sip delegate for ongoing session for this callid
            SipDelegateImpl sipDelegateToBeNotified = activeDelegateManager.
                    getSipDelegateForTransactionId(viaTransactionId);
            if( sipDelegateToBeNotified !=null ) {
                Log.i(LOG_TAG, "Found the sip delegate to notify received message"
                        + sipDelegateToBeNotified);
                //Notify the sip delegate that message is received.
                sipDelegateToBeNotified.onMessageSendFailure(viaTransactionId, sipReason);
            } else {
                Log.e(LOG_TAG, " Not Found the sip delegate to notify send msg status");
            }
        } else {
            Log.e(LOG_TAG, "onSipMessageSendStatus:service record not present for subId:"
                    + subId);
        }
    }
    /**
     * Update the ims registration with new values
     * @param registrationState the current network IMS registration state for all
     * feature tags associated with this SipDelegate.
     * @param subId subscription Id.
     */
    @Override
    public void updateRegistration(@NonNull DelegateRegistrationState registrationState,
            int subId) {
        Log.i(LOG_TAG, "updateImsRegistration: received from IMS stack for subId:" + subId);
        //Get all the active sip delegate and update then with the latest registration state.
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(subId);
        ActiveSipDelegateManager activeDelegateManager;
        if (serviceRecord != null) {
            serviceRecord.getSipTransport().
                    updatedSipTransportFeatureTagsStates(registrationState);
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            for (SipDelegateImpl sipDelegate : activeDelegateManager.getActiveSipDelegateList()) {
                sipDelegate.updateFeatureTagRegistrationChangedEvent(registrationState);
            }
        } else {
            Log.e(LOG_TAG, "updateImsRegistration:service record not present for subId:"
                    + subId);
        }
    }
    /**
     * Shared the updated configuration values
     * @param configuration updated sip delegate Registration Configuration
     * @param subId subscription Id.
     */
    @Override
    public void updateConfiguration(@NonNull SipDelegateConfiguration configuration, int subId) {
        Log.i(LOG_TAG, "updateImsConfigs: received from IMS stack for subId:" + subId);
        //Get all the active sip delegate and update then with the latest config value.
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(subId);
        ActiveSipDelegateManager activeDelegateManager;
        if (serviceRecord != null) {
            serviceRecord.getSipTransport().updateSipTransportConfig(configuration);
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            for (SipDelegateImpl sipDelegate : activeDelegateManager.getActiveSipDelegateList()) {
                sipDelegate.updateSipDelegateConfig(configuration);
            }
        } else {
            Log.e(LOG_TAG, "updateImsConfigs: service record not present for subId:" + subId);
        }
    }
}
