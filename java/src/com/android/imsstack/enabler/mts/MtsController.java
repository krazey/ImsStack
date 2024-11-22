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

package com.android.imsstack.enabler.mts;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.util.Base64;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public class MtsController {
    /* MtsController Message */
    // 1 : Request Report Mo Status
    public static final int REQUEST_REPORT_MO_STATUS = 1;
    // 2 : Request Report Mt SMS
    public static final int REQUEST_REPORT_MT_SMS = 2;
    public static final int NOTIFICATION_MTS_SEND_MO_SMS = 101;

    /* GII-SMSImpl Message */
    public static final int MO_INVALID = 0;
    public static final int MO_SUCCESS = 1;
    public static final int MO_ERROR_GENERIC = 2;
    public static final int MO_ERROR_RETRY = 3;
    public static final int MO_ERROR_FALLBACK = 4;

    public static final int SMS_FORMAT_INVALID = 0;
    public static final int SMS_FORMAT_3GPP = 1;
    public static final int SMS_FORMAT_3GPP2 = 2;

    /* REQUEST_REPORT_MO_STATUS param(bundle) name */
    public static final String REPORTMOSTATUS_REASON = "ReportMOStatus_reason";
    public static final String REPORTMOSTATUS_SMSFORMAT = "ReportMOStatus_smsFormat";
    public static final String REPORTMOSTATUS_SEQID = "ReportMOStatus_seqId";

    public static class Listener {
        /**
         * Invokes when SMS enabler gets a response to the sent SIP MESSAGE(SMS-SUBMIT) from
         * IP-SM-GW.
         *
         * @param reason
         *     {@link MtsController#MO_SUCCESS} The message was sent successfully
         *     {@link MtsController#MO_ERROR_GENERIC} The message was not sent successfully
         *     UE needs to show the failure to the user
         *     {@link MtsController#MO_ERROR_RETRY} The message was not sent successfully, and
         *     it can be retryable
         *     {@link MtsController#MO_ERROR_FALLBACK} The message was not sent successfully, and
         *     it shall be sent by using other method
         * @param format
         *     {@link MtsController#SMS_FORMAT_3GPP}
         *     {@link MtsController#SMS_FORMAT_3GPP2}
         * @param messageReference the message reference, which may be 1 byte if it is in 3GPP
         *     format (see TS.123.040) or 2 bytes if it is in 3GPP2 format (see 3GPP2 C.S0015-B).
         */
        public void notifyStatusForOutgoingMessage(int reason, int format, int messageReference) {
            // no-op
        }

        /**
         * Invokes when SMS enabler receives a SIP MESSAGE(SMS-DELIVERY) from IP-SM-GW.
         *
         * @param smsFormat
         *     {@link MtsController#SMS_FORMAT_3GPP}
         *     {@link MtsController#SMS_FORMAT_3GPP2}
         * @param pduData PDU representing the content of the received SIP MESSAGE.
         */
        public void notifyIncomingMessage(int smsFormat, byte[] pduData) {
            // no-op
        }
    }

    private IBaseContext mContext = null;
    private int mSlotId = -1;
    private MessageHandler mHandler = null;
    private Listener mListener = null;
    private MtsJni mMtsJni;
    private boolean mUseDialedNumber = false;

    public MtsController(IBaseContext context) {
        ImsLog.d(context.getSlotId(), "");

        mContext = context;
        mSlotId = mContext.getSlotId();

        mHandler = new MessageHandler(mContext.getCallLooper());

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);

        if (config != null) {
            CarrierConfig cc = config.getCarrierConfig();

            if (cc == null) {
                ImsLog.w(mSlotId, "CarrierConfig is null");
                return;
            }

            mUseDialedNumber = cc.getBoolean(
                    CarrierConfig.ImsSms.KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL);
        } else {
            ImsLog.w(mSlotId, "config is null");
            return;
        }
    }

    @VisibleForTesting
    public MtsController(IBaseContext context, Looper looper) {
        ImsLog.d(context.getSlotId(), "");

        mContext = context;
        mSlotId = mContext.getSlotId();

        mHandler = new MessageHandler(looper);
    }

    public void cleanup() {
        ImsLog.d(mSlotId, "");

        mMtsJni.release(mSlotId);

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }

        mListener = null;
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public void startNativeConnection() {
        ImsLog.d(mSlotId, "");

        mMtsJni = MtsJni.getInstance();
        mMtsJni.init(mHandler, mSlotId);
    }

    @VisibleForTesting
    public void startNativeConnection(MtsJni mtsJni) {
        mMtsJni = mtsJni;
        mMtsJni.init(mHandler, mSlotId);
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

    public boolean sendMessage(
            int smsFormat, byte[] smsData, String psiSmsc, String dialedNumber, int seqId) {
        ImsLog.d(mSlotId, "smsFormat : " + smsFormat + ", smsDataLength = " + smsData.length
                + ", psiSmsc = " + psiSmsc + ", dialedNumber = " + dialedNumber
                + ", seqId = " + seqId);
        String encodedPdu = Base64.encodeToString(smsData, Base64.DEFAULT);
        if (encodedPdu == null || psiSmsc == null) {
            processNotifySendMoSmsError(smsFormat, seqId);
            return false;
        }

        Parcel parcel = Parcel.obtain();
        if (parcel == null) {
            ImsLog.e(mSlotId, "parcel is null");
            processNotifySendMoSmsError(smsFormat, seqId);
            return false;
        }

        String targetAddress = psiSmsc;
        if (mUseDialedNumber) {
            if (dialedNumber == null) {
                processNotifySendMoSmsError(smsFormat, seqId);
                return false;
            } else {
                targetAddress = dialedNumber;
            }
        }

        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);

        parcel.writeInt(MtsJni.NOTI_MTSENABLER_SEND_MO_SMS);
        parcel.writeInt(smsFormat);
        parcel.writeString(encodedPdu);
        parcel.writeString(targetAddress);
        parcel.writeInt(seqId);
        parcel.writeBoolean((telephony != null)
                ? telephony.isEmergencyNumber(dialedNumber) : false);
        mMtsJni.sendMessage(parcel, mSlotId);
        return true;
    }

    private void processNotifySendMoSmsError(int smsFormat, int seqId) {
        ImsLog.d(mSlotId, "");

        if (mHandler == null) {
            ImsLog.e(mSlotId, "mHandler is null");
            return;
        }

        Bundle bundle = new Bundle();
        bundle.putInt(REPORTMOSTATUS_REASON, MO_ERROR_RETRY);
        bundle.putInt(REPORTMOSTATUS_SMSFORMAT, smsFormat);
        bundle.putInt(REPORTMOSTATUS_SEQID, seqId);

        Message msg = Message.obtain();
        msg.what = REQUEST_REPORT_MO_STATUS;
        msg.obj = bundle;
        mHandler.sendMessage(msg);
    }

    private class MessageHandler extends Handler {
        public MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.i(mSlotId, "MessageHandler - what=" + msg.what);

            if (mListener == null) {
                ImsLog.e(mSlotId, "mListener is null");
                return;
            }

            switch (msg.what) {
                case REQUEST_REPORT_MO_STATUS:
                    Bundle bundle = (Bundle)msg.obj;
                    int reason = bundle.getInt(REPORTMOSTATUS_REASON, 0);
                    int format = bundle.getInt(REPORTMOSTATUS_SMSFORMAT, 0);
                    int seqId = bundle.getInt(REPORTMOSTATUS_SEQID, -1);
                    mListener.notifyStatusForOutgoingMessage(reason, format, seqId);
                    break;

                case REQUEST_REPORT_MT_SMS:
                    int smsFormat = msg.arg1;
                    String encodedData = (String)msg.obj;
                    encodedData = encodedData.replaceAll("\\s", "");
                    byte[] pduData = Base64.decode(encodedData, Base64.NO_PADDING);
                    mListener.notifyIncomingMessage(smsFormat, pduData);
                    break;

                default :
                    break;
            }
        }
    }
}
