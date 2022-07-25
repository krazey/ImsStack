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

package com.android.imsstack.enabler.sipnativecontroller;

import android.annotation.NonNull;
import android.os.Parcel;
import android.telephony.ims.SipMessage;
import android.util.SparseArray;

import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.imsservice.sipcontroller.remote.ISipTransportRemote;
import com.android.imsstack.imsservice.sipcontroller.remote.SipTransportRemoteListener;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.Set;

/**
 * SipControllerAgent
 * Connect SipController and SipNativeController using IsipTransportRemote
 * Start SipNativeController service through JNI
 */
public class SipControllerAgent implements ISipTransportRemote, JniImsListener {

    private long mNativeObj = 0;
    private int mSlotId = MSimUtils.INVALID_SLOT_ID;
    private SipTransportRemoteListener mListener = null;
    private static final SparseArray<SipControllerAgent> sAgentArray = new SparseArray<>();

    SipControllerAgent(int slotId) {
        mSlotId = slotId;
        initService(slotId);
    }

    /**
     * @param slotId This id is intended to manage Sip Messages for multi-SIMs
     * @return To deliver the object for each sim to the sipcontroller
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

    private void initService(int slotId) {

        long nativeObj = JniImsProxy.getInterface(IUIMS.APP_SIP_DELEGATE, slotId);
        mNativeObj = nativeObj;
        if (nativeObj != 0) {
            JniImsProxy.setListener(nativeObj, this);
        } else {
            ImsLog.e("nativeObj is 0");
        }
    }

    /**
     * Release from SipController(Java) if the delegate is terminated or Jni is not used.
     * @param slotId The slot ID to be removed
     */
    @Override
    public void release(int slotId) {
        sAgentArray.remove(slotId);
        JniImsProxy.removeListener(mNativeObj);
        JniImsProxy.releaseInterface(mNativeObj);
    }

    @Override
    public void sendMessage(@NonNull SipMessage message, long configVersion, int subId) {

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerInternalMsgDef.SENDMESSAGE_CMD);
        convertSipMessageToParcel(message, parcel);
        sendMessageToJNI(parcel);
    }

    @Override
    public void closeOngoingSession(@NonNull String callId, int subId) {

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerInternalMsgDef.CLOSESESSION_CMD);
        parcel.writeString(callId);
        sendMessageToJNI(parcel);
    }

    @Override
    public void notifyMessageReceiveError(@NonNull String viaTransactionId, int subId) {

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerInternalMsgDef.NOTIFYMESSAGERECEIVEERROR_CMD);
        parcel.writeString(viaTransactionId);
        sendMessageToJNI(parcel);
    }

    @Override
    public void setSipTransportListener(SipTransportRemoteListener listener, int slotId) {

        mListener = listener;
        if (listener != null && mNativeObj == 0) {
            initService(slotId);
        }
    }

    @Override
    public void updateSipDelegateRegistration(@NonNull Set<String> featureTags, int subId) {
        //TODO

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
            ImsLog.e("NO listener!!!");
            return;
        }

        try {
            switch (msg) {
                case SipControllerInternalMsgDef.MESSAGERECEIVED_IND: {
                    messageReceived(parcel);
                    break;
                }
                case SipControllerInternalMsgDef.MESSAGESENT_IND: {
                    messageSent(parcel);
                    break;
                }
                case SipControllerInternalMsgDef.SENDMESSAGEFAILURE_IND: {
                    messageSendFailure(parcel);
                    break;
                }
                default:
                    ImsLog.e("NOT HANDLED!!! [" + msg + "]");
                    break;
            }
        } catch (Exception e) {
            ImsLog.e("Exception: " + e);
        }
    }

    private void messageReceived(Parcel parcel) throws Exception {

        if (mSlotId == MSimUtils.INVALID_SLOT_ID) {
            ImsLog.i("slotId is invalid");
            return;
        }
        int subId = MSimUtils.getSubId(mSlotId);
        SipMessage message = SipMessage.CREATOR.createFromParcel(parcel);

        mListener.onMessageReceived(message, subId);
    }

    private void messageSent(Parcel parcel) throws Exception {

        if (mSlotId == MSimUtils.INVALID_SLOT_ID) {
            ImsLog.i("slotId is invalid");
            return;
        }
        int subId = MSimUtils.getSubId(mSlotId);
        String transactionId = parcel.readString();

        mListener.onMessageSent(transactionId, subId);
    }

    private void messageSendFailure(Parcel parcel) throws Exception {

        if (mSlotId == MSimUtils.INVALID_SLOT_ID) {
            ImsLog.i("slotId is invalid");
            return;
        }
        int subId = MSimUtils.getSubId(mSlotId);
        int sipReason = parcel.readInt();
        String transactionId = parcel.readString();

        mListener.onMessageSendFailure(transactionId, sipReason, subId);
    }

    private static void convertSipMessageToParcel(SipMessage message, Parcel parcel) {

        parcel.writeString(message.getStartLine());
        parcel.writeString(message.getHeaderSection());
        parcel.writeInt(message.getContent().length);
        parcel.writeByteArray(message.getContent());
        parcel.writeString(message.getViaBranchParameter());
        parcel.writeString(message.getCallIdParameter());
    }
}
