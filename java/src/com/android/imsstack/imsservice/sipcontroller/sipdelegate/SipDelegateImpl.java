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

package com.android.imsstack.imsservice.sipcontroller.sipdelegate;

import android.annotation.NonNull;
import android.telephony.ims.DelegateMessageCallback;
import android.telephony.ims.DelegateRegistrationState;
import android.telephony.ims.FeatureTagState;
import android.telephony.ims.SipDelegateConfiguration;
import android.telephony.ims.SipDelegateConnection;
import android.telephony.ims.SipDelegateManager;
import android.telephony.ims.SipMessage;
import android.telephony.ims.stub.SipDelegate;
import android.util.ArraySet;
import android.util.Log;

import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.imsservice.sipcontroller.remote.ISipTransportRemote;
import com.android.internal.telephony.SipMessageParsingUtils;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;

/**
 * The{@link SipDelegate} interface implementation to interact with framework.
 */
public class SipDelegateImpl implements SipDelegate, ISipDelegateIncomingMessageNotifier {
    public static final String LOG_TAG = "SipDelegateImp";
    //Slot if for which this sip delegate is implemented.
    private int mSlotId = -1;
    /**
     * This object holds the requested feature tag and callback associated with this sip delegate
     */
    private SipDelegateRequestData mDelegateRequestData;
    /**
     * Below tags information is extracted from the {@link SipDelegateConfiguration}
     */
    private ArraySet<String> mSipDelegateRegisteredTags = new ArraySet<>();
    /**
     * Stores the call-ID header value associated with the ongoing SIP Session.
     */
    private final Set<String> mOngoingsipsessions = new HashSet<>();
    /**
     * Stores the viaTransactionId  value associated with the ongoing SIP Session.
     */
    private final Set<String> mViaTransactionIds = new HashSet<>();
    /**
     * Stores the viaTransactionId  value associated with the ongoing SIP Session.
     */
    private final HashMap<String, String> mViaTransactionIdAndCallId = new HashMap<>();
    /**
     * Below tags information is extracted from the {@link SipDelegateConfiguration}
     */
    private final ArraySet<FeatureTagState> mSipDelegateDeregisteringTags = new ArraySet<>();
    private final ArraySet<FeatureTagState> mSipDelegateDeregisteredTags = new ArraySet<>();
    private Executor mCallBackExecutor;

    /**
     * Constructor for sip delegate
     *
     * @param delegateRequestData provide the data required to create sip delegate
     */
    public SipDelegateImpl(SipDelegateRequestData delegateRequestData, int slodId,
            Executor callBackExecutor) {
        mSlotId = slodId;
        mDelegateRequestData = delegateRequestData;
        mCallBackExecutor = callBackExecutor;
    }
    //START SipDelegate interface implementation

    /**
     * Send the sip message requested by framework to IMS network through native/sip stack.
     *
     * @param message The SIP message to be sent over the operator’s network.
     * @param configVersion The SipDelegateImsConfiguration version used to construct the
     * SipMessage. See {@link SipDelegateConfiguration} for more information.
     * If the version specified here does not match the most recently constructed
     * {@link SipDelegateConfiguration}, this message should fail validation checks and
     * {@link DelegateMessageCallback#onMessageSendFailure} should be called with code
     */
    @Override
    public void sendMessage(@NonNull SipMessage message, long configVersion) {
        Log.i(LOG_TAG, "sendMessage");
        String viaTransactionId = message.getViaBranchParameter();
        String callId = message.getCallIdParameter();
        mOngoingsipsessions.add(callId);
        mViaTransactionIds.add(viaTransactionId);
        mViaTransactionIdAndCallId.put(viaTransactionId, callId);

        //TODO check RCS is registered or not. Get native API to know RCS is registered or not.
        if (false) {

            //RCS is not registered then
            onMessageSendFailure(viaTransactionId,
                    SipDelegateManager.MESSAGE_FAILURE_REASON_NOT_REGISTERED);
            //TODO since its temporary failure as per reason.
            // Who will request sending of this message again ?.
            return;
        }
        //outgoing sip message validation done at SipMessageValidator
        //added only validation which are not handled at SipMessageValidator or some race condition
        if (!isFeatureTagSupported(
                SipMessageParsingUtils.getAcceptContactFeatureTags(message.getHeaderSection()))) {
            Log.e(LOG_TAG, "sendMessage: Feature tag not supported buy this sip delegate");
            onMessageSendFailure(viaTransactionId,
                    SipDelegateManager.MESSAGE_FAILURE_REASON_TAG_NOT_ENABLED_FOR_DELEGATE);
            return;
        }
        //As per the sequence flow check for the config version is proper or not.
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        if (serviceRecord != null) {
            long latestConfig = serviceRecord.getSipTransport().getLatestConfigurationVersion();
            //This validation is required to handle any race condition
            Log.e(LOG_TAG, "latestConfig:" + latestConfig
                    + " Config used to send msg:" + configVersion);
            if (configVersion != latestConfig) {
                Log.e(LOG_TAG, "sendMessage: client is not using latest configuration:"
                        + latestConfig);
                onMessageSendFailure(viaTransactionId,
                        SipDelegateManager.MESSAGE_FAILURE_REASON_STALE_IMS_CONFIGURATION);
                return;
            }
            //Check if this delegate handles the feature tags requested in the sip message.
            Set<String> sendRequestTags = SipMessageParsingUtils.
                    getAcceptContactFeatureTags(message.getHeaderSection());
            if (getSupportedFeatureTags().stream().anyMatch(sendRequestTags::contains)) {
                Log.i(LOG_TAG, "sendMessage to native");
                mOngoingsipsessions.add(message.getCallIdParameter());
                mViaTransactionIds.add(viaTransactionId);
                //Keep track of this sip delegate and call id mapping.
                serviceRecord.getSipTransport().getActiveSipDelegateManager().
                        addActiveSipDelegate(message.getCallIdParameter(), this);
                //make native API call to send the message.
                serviceRecord.getSipTransport().getISipTransportRemote().
                        sendMessage(message, configVersion);
            }else {
                Log.i(LOG_TAG, "SipDelegate does not support requested feature tag");
            }
        }
    }

    /**
     * The remote IMS application has closed a SIP session and the routing resources associated
     * with the SIP session using the provided Call-ID may now be cleaned up.
     * <p>
     * Typically, a SIP session will be considered closed when all associated dialogs receive a
     * BYE request. After the session has been closed, the IMS application will call
     * {@link SipDelegateConnection#cleanupSession(String)} to signal to the framework that
     * resources can be released. In some cases, the framework will request that the ImsService
     * close the session due to the open SIP session holding up an event such as applying a
     * provisioning change or handing over to another transport type. See
     * {@link DelegateRegistrationState}.
     *
     * @param callId The call-ID header value associated with the ongoing SIP Session that the
     * framework is requesting be cleaned up.
     */
    @Override
    public void cleanupSession(@NonNull String callId) {
        Log.i(LOG_TAG, "cleanupSession");
        Log.i(LOG_TAG, "cleanupSession for callid: " + callId);

        if (mOngoingsipsessions != null && !mOngoingsipsessions.isEmpty()) {
            mOngoingsipsessions.remove(callId);
            removeTransactionIds(callId);
        }
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        ActiveSipDelegateManager activeDelegateManager;
        ISipTransportRemote remoteSipTransport;
        if (serviceRecord != null) {
            remoteSipTransport = serviceRecord.getSipTransport().getISipTransportRemote();
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            activeDelegateManager.removeActiveSipDelegate(callId);
            //call native API to clean up ongoing session
            if (remoteSipTransport != null) {
                remoteSipTransport.closeOngoingSession(callId);
            } else {
                Log.e(LOG_TAG, "remoteSipTransport is null");
            }
        } else {
            Log.e(LOG_TAG, "serviceRecord is null");
        }
    }

    /**
     * The remote application has received the incoming SIP message.
     * As message is received clean up the resources related to viaTransactionId.
     *
     * @param viaTransactionId The Transaction ID found in the via header field of the
     */
    @Override
    public void notifyMessageReceived(@NonNull String viaTransactionId) {
        Log.i(LOG_TAG, "notifyMessageReceived");
        if (mViaTransactionIds != null) {
            mViaTransactionIds.remove(viaTransactionId);
            mViaTransactionIdAndCallId.remove(viaTransactionId);
        }
        //make native call successful received message response from app.
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        ISipTransportRemote remoteSipTransport;
        if (serviceRecord != null) {
            remoteSipTransport = serviceRecord.getSipTransport().getISipTransportRemote();
            if (remoteSipTransport != null) {
                remoteSipTransport.notifyMessageReceived(viaTransactionId);
            } else {
                Log.e(LOG_TAG, "remoteSipTransport is null");
            }
        } else {
            Log.e(LOG_TAG, "serviceRecord is null");
        }
    }

    /**
     * The remote application not received message due to some error.
     * Handles the error and clean up resources related to viaTransactionId.
     *
     * @param viaTransactionId The Transaction ID found in the via header field of the
     * previously sent {@link SipMessage}.
     * @param reason The reason why the message was not correctly received.
     */
    @Override
    public void notifyMessageReceiveError(@NonNull String viaTransactionId, int reason) {
        Log.i(LOG_TAG, "notifyMessageReceiveError reason:" + reason);
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        if (serviceRecord != null) {
            ISipTransportRemote nativeSipController =
                    serviceRecord.getSipTransport().getISipTransportRemote();
            if (nativeSipController != null) {
                nativeSipController.notifyMessageReceiveError(viaTransactionId, reason);
            } else {
                Log.d(LOG_TAG, "nativeSipController is null");
            }
        }
    }
    // END SipDelegate interface implementation

    // START ISipTransportIncomingMessageNotifier interface implementation

    /**
     * Notify the apps for the SipMessage received
     *
     * @param message received from IMS network
     */
    @Override
    public void onMessageReceived(SipMessage message) {
        Log.i(LOG_TAG, "onMessageReceived callId:" + message.getCallIdParameter());
        mCallBackExecutor.execute(() -> {
            mDelegateRequestData.getDelegateMessageCallback().onMessageReceived(message);
        });
    }

    /**
     * Notify the app that message sent successful
     *
     * @param viaTransactionId The transaction ID found in the via header field of the
     */
    @Override
    public void onMessageSent(String viaTransactionId) {
        Log.i(LOG_TAG, "onMessageSent:" + viaTransactionId);
        mCallBackExecutor.execute(() -> {
            mDelegateRequestData.getDelegateMessageCallback().onMessageSent(viaTransactionId);
        });
        mViaTransactionIds.remove(viaTransactionId);
        mViaTransactionIdAndCallId.remove(viaTransactionId);
    }

    /**
     * Notify the app that message send is failed
     *
     * @param viaTransactionId The Transaction ID found in the via header field of the previously
     * sent {@link SipMessage}.
     * @param reason The reason for the failure.
     */
    @Override
    public void onMessageSendFailure(String viaTransactionId, int reason) {
        Log.i(LOG_TAG, "onMessageSendFailure:" + viaTransactionId);
        mCallBackExecutor.execute(() -> {
            mDelegateRequestData.getDelegateMessageCallback().onMessageSendFailure(viaTransactionId,
                    reason);
        });
    }
    // END ISipTransportIncomingMessageNotifier interface implementation
    //START Public methods

    /**
     * This will update the latest sip configuration values to app
     *
     * @param updatedConfig received from the ims core
     */
    public void updateSipDelegateConfig(@NonNull SipDelegateConfiguration updatedConfig){
        mCallBackExecutor.execute(() -> {
            mDelegateRequestData.getDelegateStateCallback().onConfigurationChanged(updatedConfig);
        });
    }

    /**
     * Update the feature tag changed registration to app/framework
     *
     * @param registrationState updated registration state of requested feature tags
     */
    public void updateFeatureTagRegistrationChangedEvent(@NonNull DelegateRegistrationState
            registrationState) {

        Log.i(LOG_TAG, "updateFeatureTagRegistrationChangedEvent");
        if (registrationState != null) {
            Log.i(LOG_TAG, "Before Registered:"
                    + registrationState.getRegisteredFeatureTags());
            Log.i(LOG_TAG, "Before De-Registered:"
                    + registrationState.getDeregisteredFeatureTags());
            Log.i(LOG_TAG, "Before Registering:"
                    + registrationState.getDeregisteringFeatureTags());
        }

        //Extract the feature tags related to this sip delegate then notify the
        //delegate registration change if any change with current state.
        //Extract the tags information from the registration change event
        DelegateRegistrationState sipDelegateRegistrationState =
                getSipDelegateRegistrationStateObject(registrationState);

        if (sipDelegateRegistrationState != null) {
            Log.i(LOG_TAG, "Registered:"
                    + sipDelegateRegistrationState.getRegisteredFeatureTags());
            Log.i(LOG_TAG, "De-Registered:"
                    + sipDelegateRegistrationState.getDeregisteredFeatureTags());
            Log.i(LOG_TAG, "Registering:"
                    + sipDelegateRegistrationState.getDeregisteringFeatureTags());
        }
        //update the registration change related to this sip delegate.
        mCallBackExecutor.execute(() -> {
            mDelegateRequestData.getDelegateStateCallback()
                    .onFeatureTagRegistrationChanged(sipDelegateRegistrationState);
        });
    }
    //END Public methods
    //START Getter methods

    /**
     * Get the delegate request data
     *
     * @return app delegate request data
     */
    public SipDelegateRequestData getDelegateRequestData() {
        return mDelegateRequestData;
    }

    /**
     * Get the supported feature list by this sip delegate
     *
     * @return supported feature tags by this sip delegate
     */
    public Set<String> getSupportedFeatureTags() {
        return mSipDelegateRegisteredTags;
    }

    /**
     * Get the all the on going call Id of the current active sip sessions
     *
     * @return active sip session call ids
     */
    public Set<String> getOngoingsipsessions() {
        return mOngoingsipsessions;
    }

    /**
     * Get the transactionids handled by this Sip Delegate
     */

    public Set<String> getViaTransactionIds() {
        return mViaTransactionIds;
    }

    /**
     * Get the TransactionId and CallId handled by this Sip Delegate
     */

    public HashMap<String, String> getViaTransactionIdsAndCallId() {
        return mViaTransactionIdAndCallId;
    }
    //END Getter methods
    //START Setter methods
    //END Setter methods
    //START Private methods

    /**
     * Check requested feature tag is registered by this sip delegate or not
     *
     * @param validateFeatureTag feature tags to be validated
     * @return true if sip delegate supports else false
     */
    private boolean isFeatureTagSupported(Set<String> validateFeatureTag) {
        boolean isFeatureTagSupportedByThisDelegate = false;
        for (String featureTag : validateFeatureTag) {
            if (mSipDelegateRegisteredTags.contains(featureTag)) {
                isFeatureTagSupportedByThisDelegate = true;
                continue;
            } else {
                isFeatureTagSupportedByThisDelegate = false;
                break;
            }
        }
        return isFeatureTagSupportedByThisDelegate;
    }

    /**
     * Get the sip delegate registration object to be notified to apps/framework
     *
     * @param registrationState to be notified to app requested for this sip delegate.
     */
    private DelegateRegistrationState getSipDelegateRegistrationStateObject(
            DelegateRegistrationState registrationState) {
        DelegateRegistrationState.Builder sipDelegateRegistrationStateBuilder =
                new DelegateRegistrationState.Builder();

        //Extract the tags information from the registration change event
        //1. Loop through feature tags requested for this sip delegate to be updated
        // registered/de registered/de registering.
        //2. Compare the feature tags requested as part of delegate request
        // This confirms that this delegate is interested in this feature tag updates.
        //3. Notify update ONLY to those feature tags associated with this sip delegate.

        //Registered tags
        for (String registeredFeatureTag : registrationState.getRegisteredFeatureTags()) {
            Log.i(LOG_TAG, "registeredFeatureTag:" + registeredFeatureTag);
            if (mDelegateRequestData.getRequestedFeatureTags().contains(registeredFeatureTag)) {
                mSipDelegateRegisteredTags.add(registeredFeatureTag);
                //update the builder object which needs to be notified to app
                sipDelegateRegistrationStateBuilder.addRegisteredFeatureTag(registeredFeatureTag);
            }
        }
        //De registered tags
        for (FeatureTagState deregisteredFeatureTag :
                registrationState.getDeregisteredFeatureTags()) {
            String featureTag = deregisteredFeatureTag.getFeatureTag();
            Log.i(LOG_TAG, "de-registeredFeatureTag:" + deregisteredFeatureTag);
            if (mDelegateRequestData.getRequestedFeatureTags().contains(featureTag)) {
                mSipDelegateDeregisteredTags.add(deregisteredFeatureTag);
                //update the builder object which needs to be notified to app
                int reason = deregisteredFeatureTag.getState();
                sipDelegateRegistrationStateBuilder.addDeregisteredFeatureTag(featureTag, reason);
            }
        }
        // De registering tags
        for (FeatureTagState deregisteringTag : registrationState.getDeregisteringFeatureTags()) {
            String featureTag = deregisteringTag.getFeatureTag();
            Log.i(LOG_TAG, "de-registeringTag:" + deregisteringTag);
            if (mDelegateRequestData.getRequestedFeatureTags().contains(featureTag)) {
                mSipDelegateDeregisteringTags.add(deregisteringTag);
                //update the builder object which needs to be notified to app
                //TODO check the default value
                int reason = deregisteringTag.getState();
                sipDelegateRegistrationStateBuilder.addDeregisteringFeatureTag(featureTag, reason);
            }
        }
        return sipDelegateRegistrationStateBuilder.build();
    }

    /**
     * Remove the transaction ids belong to dialog with provided callid
     *
     * @param callId for which we need to remove transaction ids
     */
    private void removeTransactionIds(String callId) {
        if (mViaTransactionIds != null && mViaTransactionIdAndCallId != null) {
            mViaTransactionIdAndCallId.forEach((key, value) -> {
                if (callId.equals(value)) {
                    mViaTransactionIds.remove(key);
                    mViaTransactionIdAndCallId.remove(key);
                }
            });
        }
    }
    //END Private methods
}
