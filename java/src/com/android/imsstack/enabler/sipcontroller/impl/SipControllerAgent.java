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

package com.android.imsstack.enabler.sipcontroller.impl;

import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_DELEGATE_CLOSED;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_DELEGATE_DEAD;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_INTERNAL_DELEGATE_STATE_TRANSITION;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_INVALID_BODY_CONTENT;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_INVALID_FEATURE_TAG;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_INVALID_HEADER_FIELDS;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_INVALID_START_LINE;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_NETWORK_NOT_AVAILABLE;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_NOT_REGISTERED;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_STALE_IMS_CONFIGURATION;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_TAG_NOT_ENABLED_FOR_DELEGATE;
import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_UNKNOWN;

import android.annotation.NonNull;
import android.os.Parcel;
import android.telephony.ims.DelegateRegistrationState;
import android.telephony.ims.SipDelegateConfiguration;
import android.telephony.ims.SipMessage;
import android.util.SparseArray;

import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.imsservice.sipcontroller.remote.ISipTransportRemote;
import com.android.imsstack.imsservice.sipcontroller.remote.SipTransportRemoteListener;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.SipMessageParsingUtils;

import java.util.HashMap;
import java.util.Set;
import java.util.stream.IntStream;

/**
 * SipControllerAgent
 * Connect SipController and SipNativeController using IsipTransportRemote
 * Start SipNativeController service through JNI
 */
public class SipControllerAgent implements ISipTransportRemote, JniImsListener {
    private long mNativeObj = 0;
    private int mSlotId = MSimUtils.INVALID_SLOT_ID;
    private int mSubId = MSimUtils.INVALID_SUB_ID;
    private SipTransportRemoteListener mListener = null;
    private static final SparseArray<SipControllerAgent> sAgentArray = new SparseArray<>();
    private static String sStartLineString;
    private static String sHeaderSectionString;
    private static byte[] sConBytes;

    // HashMap stores viaTransactionId(key)/SipMessage for every sip message to send
    // The Message in the map is removed
    // when either success or failure response that message is received
    public HashMap<String, SipMessage> mSipMsgMap;

    SipControllerAgent(int slotId) {
        if (slotId == MSimUtils.INVALID_SLOT_ID) {
            ImsLog.i("slotId is invalid");
            return;
        }
        mSlotId = slotId;
        mSubId = MSimUtils.getSubId(slotId);
        initService(slotId);
        mSipMsgMap = new HashMap<>();
    }

    /**
     * @param slotId SIM slot id for instance of SipControllerAgent
     * @return Instance of SipControllerAgent associated with SIM slot ID.
     */
    public static SipControllerAgent getInstance(int slotId) {
        synchronized (sAgentArray) {
            if (sAgentArray.indexOfKey(slotId) < 0) {
                sAgentArray.put(slotId, SipControllerAgent.createSipControllerAgent(slotId));
            }
            return sAgentArray.get(slotId);
        }
    }

    private static SipControllerAgent createSipControllerAgent(int slotId) {
        return new SipControllerAgent(slotId);
    }

    /**
     * @param slotId SIM slot id for instance of SipControllerAgent
     * @param subId The ID of the Subscription associated with slot ID.
     * @return Instance of SipControllerAgent associated with SIM slot ID.
     */
    @VisibleForTesting
    public static SipControllerAgent getInstance(int slotId, int subId) {
        synchronized (sAgentArray) {
            if (sAgentArray.indexOfKey(slotId) < 0) {
                sAgentArray.put(slotId, SipControllerAgent.createSipControllerAgent(slotId, subId));
            }
            return sAgentArray.get(slotId);
        }
    }

    @VisibleForTesting
    private static SipControllerAgent createSipControllerAgent(int slotId, int subId) {
        return new SipControllerAgent(slotId, subId);
    }

    @VisibleForTesting
    SipControllerAgent(int slotId, int subId) {
        mSlotId = slotId;
        mSubId = subId;
        mSipMsgMap = new HashMap<>();
    }

    private void initService(int slotId) {
        long nativeObj = JniImsProxy.getInterface(IUIMS.APP_SIP_DELEGATE, slotId);
        mNativeObj = nativeObj;
        if (nativeObj != 0) {
            JniImsProxy.setListener(nativeObj, this);
        } else {
            ImsLog.e(mSlotId, "nativeObj is 0");
        }
    }

    /**
     * Release from SipController(Java) if the delegate is terminated or Jni is not used.
     * @param slotId The slot ID to be removed
     */
    @Override
    public void release(int slotId) {
        if (mNativeObj != 0) {
            sAgentArray.remove(slotId);
            setSipTransportListener(null);
            JniImsProxy.removeListener(mNativeObj);
            JniImsProxy.releaseInterface(mNativeObj);
            mNativeObj = 0;
        }
    }

    @Override
    public void sendMessage(@NonNull SipMessage message, long configVersion) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerConstants.SEND_MESSAGE_CMD);
        convertSipMessageToParcel(message, parcel);
        String branch = SipMessageParsingUtils.getTransactionId(message.getHeaderSection());
        mSipMsgMap.put(branch, message);
        sendMessageToJNI(parcel);
    }

    @Override
    public void closeOngoingSession(@NonNull String callId) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerConstants.CLOSE_SESSION_CMD);
        parcel.writeString(callId);
        sendMessageToJNI(parcel);
    }

    @Override
    public void notifyMessageReceived(@NonNull String viaTransactionId) {
        if (validateNotifyMessageReceived(viaTransactionId)) {
            mSipMsgMap.remove(viaTransactionId);
        }
    }

    private Boolean validateNotifyMessageReceived(String viaTransactionId) {
        if (viaTransactionId == null) {
            ImsLog.e(mSlotId, "viaTransactionId == null");
            return false;
        }
        if (!mSipMsgMap.containsKey(viaTransactionId)) {
            ImsLog.e(mSlotId, "Message is not contain in Map" + viaTransactionId);
            return false;
        }
        return true;
    }

    @Override
    public void notifyMessageReceiveError(@NonNull String viaTransactionId, int reason) {
        if (validateNotifyMessageReceiveError(viaTransactionId, reason)) {

            Parcel parcel = Parcel.obtain();
            parcel.writeInt(SipControllerConstants.NOTIFY_MESSAGE_RECEIVE_ERROR_CMD);
            parcel.writeString(viaTransactionId);
            SipMessage message = mSipMsgMap.get(viaTransactionId);
            mSipMsgMap.remove(viaTransactionId);

            if (message == null) {
                ImsLog.e(mSlotId, "SipMessage Does not exist in mSipMsgMap");
                return;
            }
            // ToDo
            // need to make the response error message param
            sendMessageToJNI(parcel);
        }
    }

    private Boolean validateNotifyMessageReceiveError(String viaTransactionId, int reason) {
        if (!validateNotifyMessageReceived(viaTransactionId)) {
            return false;
        }

        int[] Reasons = new int[] {
            MESSAGE_FAILURE_REASON_UNKNOWN,
            MESSAGE_FAILURE_REASON_DELEGATE_DEAD,
            MESSAGE_FAILURE_REASON_DELEGATE_CLOSED,
            MESSAGE_FAILURE_REASON_INVALID_START_LINE,
            MESSAGE_FAILURE_REASON_INVALID_HEADER_FIELDS,
            MESSAGE_FAILURE_REASON_INVALID_BODY_CONTENT,
            MESSAGE_FAILURE_REASON_INVALID_FEATURE_TAG,
            MESSAGE_FAILURE_REASON_TAG_NOT_ENABLED_FOR_DELEGATE,
            MESSAGE_FAILURE_REASON_NETWORK_NOT_AVAILABLE,
            MESSAGE_FAILURE_REASON_NOT_REGISTERED,
            MESSAGE_FAILURE_REASON_STALE_IMS_CONFIGURATION,
            MESSAGE_FAILURE_REASON_INTERNAL_DELEGATE_STATE_TRANSITION
        };
        if (IntStream.of(Reasons).noneMatch(x -> x == reason)) {
            ImsLog.e(mSlotId, "Message failure reason: " + reason);
            return false;
        }
        return true;
    }

    @Override
    public void setSipTransportListener(SipTransportRemoteListener listener) {
        mListener = listener;
        if (listener != null && mNativeObj == 0) {
            ImsLog.i(mSlotId, "NativeObj of JniImsProxy is Null");
        }
    }

    @Override
    public void updateSipDelegateRegistration(@NonNull Set<String> featureTags) {
        ImsLog.i(mSlotId, "updateSipDelegateRegistration");
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerConstants.UPDATE_DELEGATE_REGISTRATION_CMD);
        parcel.writeStringArray((String[]) featureTags.toArray());
        sendMessageToJNI(parcel);
    }

    @Override
    public void triggerSipDelegateDeRegistration() {
        ImsLog.i(mSlotId, "triggerSipDelegateDeRegistration");
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerConstants.TRIGGER_DELEGATEDE_REGISTRATION_CMD);
        sendMessageToJNI(parcel);
    }

    private void sendMessageToJNI(Parcel parcel) {

        if (parcel == null) {
            return;
        }

        byte[] baData = parcel.marshall();
        parcel.recycle();
        parcel = null;

        if (mNativeObj != 0) {
            JniImsProxy.sendData(mNativeObj, baData);
        }
    }

    @Override
    public void onMessage(Parcel parcel) {
        int msg = parcel.readInt();
        if (mListener == null) {
            ImsLog.e(mSlotId, "NO listener!!!");
            return;
        }

        try {
            switch (msg) {
                case SipControllerConstants.MESSAGE_RECEIVED_IND: {
                    messageReceived(parcel);
                    break;
                }
                case SipControllerConstants.MESSAGE_SENT_IND: {
                    messageSent(parcel);
                    break;
                }
                case SipControllerConstants.SEND_MESSAGE_FAILURE_IND: {
                    messageSendFailure(parcel);
                    break;
                }
                case SipControllerConstants.ONREGISTRATION_UPDATED_IND: {
                    registrationUpdated(parcel);
                    break;
                }
                case SipControllerConstants.ONCONFIGURATION_UPDATED_IND: {
                    configurationUpdated(parcel);
                    break;
                }
                default:
                    ImsLog.e(mSlotId, "NOT HANDLED!!! [" + msg + "]");
                    break;
            }
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception: " + e);
        }
    }

    private void messageReceived(Parcel parcel) throws Exception {
        ImsLog.i(mSlotId, "messageReceived");
        convertParcelToString(parcel);
        SipMessage message = new SipMessage(sStartLineString, sHeaderSectionString, sConBytes);

        String branch = SipMessageParsingUtils.getTransactionId(message.getHeaderSection());
        String callId = SipMessageParsingUtils.getCallId(message.getHeaderSection());

        if (branch != null && callId != null) {
            mSipMsgMap.put(branch, message);
            mListener.onMessageReceived(message, mSubId);
        }
    }

    private void messageSent(Parcel parcel) throws Exception {
        ImsLog.i(mSlotId, "messageSent");
        String transactionId = parcel.readString();
        SipMessage message = mSipMsgMap.remove(transactionId);
        if (message == null) {
            ImsLog.e(mSlotId, "SipMessage Does not exist in mSipMsgMap");
            return;
        }

        mListener.onMessageSent(transactionId, mSubId);
    }

    private void messageSendFailure(Parcel parcel) throws Exception {
        ImsLog.i(mSlotId, "messageSendFailure");
        int sipReason = parcel.readInt();
        String transactionId = parcel.readString();

        SipMessage message = mSipMsgMap.remove(transactionId);
        if (message == null) {
            ImsLog.e(mSlotId, "SipMessage Does not exist in mSipMsgMap");
            return;
        }

        mListener.onMessageSendFailure(transactionId, sipReason, mSubId);
    }

    private void registrationUpdated(Parcel parcel) throws Exception {
        ImsLog.i(mSlotId, "registrationUpdated");
        DelegateRegistrationState registrationState =
                DelegateRegistrationState.CREATOR.createFromParcel(parcel);
        mListener.updateRegistration(registrationState, mSubId);
    }

    private void configurationUpdated(Parcel parcel) throws Exception {
        ImsLog.i(mSlotId, "configurationUpdated");
        SipDelegateConfiguration configuration =
                SipDelegateConfiguration.CREATOR.createFromParcel(parcel);
        mListener.updateConfiguration(configuration, mSubId);
    }

    private static void convertSipMessageToParcel(SipMessage message, Parcel parcel) {
        parcel.writeString(message.getStartLine());
        parcel.writeString(message.getHeaderSection());
        parcel.writeInt(message.getContent().length);
        parcel.writeByteArray(message.getContent());
        String[] method = message.getStartLine().split(" ");
        parcel.writeString(method[0]);
        parcel.writeString(SipMessageParsingUtils.getFromTag(message.getHeaderSection()));
        parcel.writeString(SipMessageParsingUtils.getToTag(message.getHeaderSection()));

        if (SipMessageParsingUtils.isSipRequest(message.getStartLine())) {
            parcel.writeInt(0);
        } else if (SipMessageParsingUtils.isSipResponse(message.getStartLine())) {
            parcel.writeInt(1);
        } else {
            ImsLog.e("Method is invalid");
        }
    }

    private static void convertParcelToString(Parcel parcel) {
        sStartLineString = parcel.readString();
        sHeaderSectionString = parcel.readString();
        sConBytes = new byte[parcel.readInt()];
        parcel.readByteArray(sConBytes);
    }
}
