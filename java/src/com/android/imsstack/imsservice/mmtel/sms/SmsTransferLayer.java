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

package com.android.imsstack.imsservice.mmtel.sms;

import android.telephony.SmsMessage;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.internal.annotations.VisibleForTesting;
import com.android.telephony.Rlog;

import java.io.ByteArrayOutputStream;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Provides the functionality of SMS Transfer Layer as per 3GPP TS 23.040
 **/
public final class SmsTransferLayer {
    private final Object mLock = new Object();
    private static final String TAG = "[GII-SmsTL] ";
    private final ImsCallContext mCallContext;
    private SmsRelayLayer mSmsRL = null;
    private final SmsRLListenerProxy mSmsRLListener = new SmsRLListenerProxy();
    private SmsTransferLayer.Listener mListener = null;
    public Map<Integer, Integer> mTpMrTracker = new ConcurrentHashMap<>();

    /**
     * Listener to handle the events sent from SmsTransferLayer
     */
    public interface Listener {
        /**
         * notifies the receipt of report for SMS-SUBMIT sent
         * @param token provided in {@link #sendMoTPdu(int, int, String, byte[])}
         * @param result result of sending the SMS.
         * @param reason reason in case status is failure.
         */
        void notifySmsResult(int token, int result, int reason);

        /**
         * notifies ImsSmsImpl about the Incoming SMS-DELIVER or SMS-STATUS-REPORT message
         * @param token unique token generated to trigger callbacks for this message.
         * @param format the format of the message.
         * @param messageType Message Type with respect to TPDU
         * @param pdu PDU representing the contents of the message.
         *
         * @return the result if handling incoming RP-Data received
         */
        int notifySmsReceived(int token, int format, int messageType, byte[] pdu);
    }

    public SmsTransferLayer(ImsCallContext callContext) {
        mCallContext = callContext;
        mSmsRL = new SmsRelayLayer(mCallContext);
        if (mSmsRL != null) {
            mSmsRL.setListener(mSmsRLListener);
        }
    }

    @VisibleForTesting
    public SmsTransferLayer(ImsCallContext callContext, SmsRelayLayer smsRL) {
        mCallContext = callContext;
        if (smsRL == null) {
            mSmsRL = new SmsRelayLayer(mCallContext);
        } else {
            mSmsRL = smsRL;
        }

        if (mSmsRL != null) {
            mSmsRL.setListener(mSmsRLListener);
        }
    }

    public void setListener(SmsTransferLayer.Listener listener) {
        mListener = listener;
    }

    /**
     * Handles SMS-SUBMIT Message at Transfer Layer and notifies Relay Layer to send RP-DATA
     * @param token sent from framework to track callback for each SMS-SUBMIT message
     * @param smsFormat format of the message
     * @parm tpMessageRef the TP-MR passed for the SMS-SUBMIT message
     * @param smsc the Short Message Service Center address
     * @param pdu PDU representing the contents of the message.
     *
     * @return result of processing of outgoing SMS's TPDU
     */
    public int sendMoTPdu(int token, int smsFormat, int tpMessageRef, String smsc, byte[] pdu) {
        Rlog.d(TAG, "sendMoTPdu");
        try {
            int result;
            synchronized (mLock) {
                mTpMrTracker.put(tpMessageRef, token);
            }
            /* Framework's TPdu Parser expects the TPdu be prepended with SC-Address.
             * else the parser will throw exception. So prepending TPdu with 0,
             * which indicates that there is no SC address and its length is 0.
             * This way Parser will skip parsing for SC-Address
             */
            ByteArrayOutputStream bo = new ByteArrayOutputStream(pdu.length + 1);
            bo.write(0x00);
            bo.write(pdu, 0, pdu.length);
            byte[] frameworkPdu = bo.toByteArray();
            SmsMessage message = SmsMessage.createFromPdu(frameworkPdu, SmsMessage.FORMAT_3GPP);
            String address = message.getRecipientAddress();
            return mSmsRL.sendRPMessage(token, SmsUtils.RP_DATA, smsc, address, pdu);
        } catch (RuntimeException e) {
            Rlog.e(TAG, "SendSms Failed at SmsTransferLayer: " + e.getMessage());
            return SmsUtils.SMSTL_RESULT_EXCEPTION;
        }
    }

    /**
     * Notifies Relay Layer to Send RP-Ack or RP-Error based on result
     * @param token same as that used when SMS-DELIVER Message is
     * @param tlMessageType indicates the Message Type to be sent in TPdu
     * @param messageRef indicates the TP-MR with respect to this token
     * @param result indicates the result of SMS-DELIVER receipt
     *
     * @return result indicating if TPDU is sent successfully to relay layer
     */
    public int sendReportTPdu(int token, int tlMessageType, int messageRef, int result) {
        Rlog.d(TAG, "sendReportTPdu");
        try {
            int messageType = SmsUtils.RP_ERROR;
            if (result == ImsSmsImplBase.DELIVER_STATUS_OK) {
                messageType = SmsUtils.RP_ACK;
            }
            synchronized (mLock) {
                if (tlMessageType == SmsUtils.TP_SMS_STATUS_REPORT) {
                    if (mTpMrTracker.containsKey(messageRef)) {
                        mTpMrTracker.remove(messageRef);
                    }
                }
            }
            return mSmsRL.sendRPMessage(token, messageType, null, null, null);
        } catch (RuntimeException e) {
            Rlog.e(TAG, "sendReportTPdu Failed: " + e.getMessage());
            return SmsUtils.SMSTL_RESULT_EXCEPTION;
        }
    }

    private class SmsRLListenerProxy implements SmsRelayLayer.Listener {

        public int notifyRLDataIndication(
                int token, int smsFormat, int rpMessageType, byte[] pdu) {
            Rlog.d(TAG, "notifyRLDataIndication");
            int result = SmsUtils.SMSTL_RESULT_FAILURE;
            try {
                SmsTransferLayer.Listener listener = mListener;
                if (listener == null) {
                    Rlog.d(TAG, "Listener is null");
                    return result;
                }
                int tpMessageType = SmsUtils.TP_SMS_DELIVER;
                int originAddressLength = pdu[SmsUtils.MODIFIED_TPDU_ORIGIN_ADDR_LENGTH_INDEX];
                int modifiedTpduStartIndex = 1 + originAddressLength; //length byte + Value length
                int tpMr = pdu[modifiedTpduStartIndex + SmsUtils.TPDU_MR_INDEX] & 0xff;
                int tpMti = pdu[modifiedTpduStartIndex + SmsUtils.TPDU_MTI_INDEX] & 0x3;
                if (tpMti == SmsUtils.TPDU_MTI_SMS_STATUS_REPORT) {
                    Rlog.i(TAG, "notifyRLDataIndication: received SMS_STATUS_REPORT");
                    tpMessageType = SmsUtils.TP_SMS_STATUS_REPORT;
                    synchronized (mLock) {
                        if (mTpMrTracker.containsKey(tpMr)) {
                            token = mTpMrTracker.get(tpMr);
                        } else {
                            Rlog.e(TAG, "No matching TpMR ");
                            return result;
                        }
                    }
                }
                synchronized (mLock) {
                    return listener.notifySmsReceived(token, smsFormat, tpMessageType, pdu);
                }
            } catch (RuntimeException e) {
                Rlog.e(TAG, "notifyRLDataIndication Failed: " + e.getMessage());
                return SmsUtils.SMSTL_RESULT_EXCEPTION;
            }
        }

        public void notifyRLReportIndication(int token, int result, int reason) {
            try {
                Rlog.d(TAG, "notifyRLReportIndication");
                SmsTransferLayer.Listener listener = mListener;
                // TODO: map result and reason
                if (listener == null) {
                    return;
                }
                synchronized (mLock) {
                    listener.notifySmsResult(token, result, reason);
                }
            } catch (RuntimeException e) {
                Rlog.e(TAG, "notifyRLReportIndication Failed: " + e.getMessage());
            }
        }
    }
}
