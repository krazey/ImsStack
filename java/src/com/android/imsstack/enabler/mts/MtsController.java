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

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
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
    public static final int MO_IMS_TEMP_FAILURE = 2;
    public static final int MO_IMS_PERM_FAILURE = 3;
    public static final int MO_IMS_LIMITEDSMSSVCREGI = 4;
    public static final int MO_RETRY_CS = 5;
    public static final int MO_RETRY_CS_OR_SGS = 6;

    public static final int MT_INVALID = 0;
    public static final int MT_SUCCESS = 1;
    public static final int MT_FAILURE = 2;
    public static final int MT_SMS_FORMAT_FAILURE = 3;
    public static final int MT_SMS_NODATA_FAILURE = 4;

    /* REQUEST_REPORT_MO_STATUS param(bundle) name */
    public static final String REPORTMOSTATUS_REASON = "ReportMOStatus_reason";
    public static final String REPORTMOSTATUS_SMSFORMAT = "ReportMOStatus_smsFormat";
    public static final String REPORTMOSTATUS_RETRYAFTER = "ReportMOStatus_retryAfter";
    public static final String REPORTMOSTATUS_SEQID = "ReportMOStatus_seqId";

    public static class Listener {
        /*
         * Invokes when SMS enabler gets a response to the sent SIP MESSAGE(SMS-SUBMIT) from
         * IP-SM-GW.
         */
        public void notifyStatusForOutgoingMessage(int reason, int format, int retryAfter,
                int messageReference) {
            // no-op
        }
        /*
         * Invokes when SMS enabler receives a SIP MESSAGE(SMS-DELIVERY) from IP-SM-GW.
         * This method returns:
         * 1 - MT_SUCCESS
         * 2 - MT_FAILURE
         * 3 - MT_SMS_FORMAT_FAILURE
         * 4 - MT_SMS_NODATA_FAILURE
         */
        public int notifyIncomingMessage(int smsFormat, String encodedData) {
            // no-op
            return MT_FAILURE;
        }
    }

    private MtsJni mMtsJni;
    private IBaseContext mContext = null;
    private MessageHandler mHandler = null;
    private Listener mListener = null;
    private boolean mUseDialedNumber = false;

    public MtsController(IBaseContext context) {
        ImsLog.d("");

        mContext = context;

        mHandler = new MessageHandler(mContext.getCallLooper());

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mContext.getSlotId());

        if (config != null) {
            CarrierConfig cc = config.getCarrierConfig();

            if (cc == null) {
                ImsLog.w(mContext.getSlotId(), "CarrierConfig is null");
                return;
            }

            mUseDialedNumber = cc.getBoolean(
                    CarrierConfig.Assets.KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL);
        } else {
            ImsLog.w(mContext.getSlotId(), "config is null");
            return;
        }
    }

    @VisibleForTesting
    public MtsController(IBaseContext context, Looper looper) {
        ImsLog.d("");

        mContext = context;

        mHandler = new MessageHandler(looper);
    }

    public void cleanup() {
        ImsLog.d("");

        mMtsJni.release(mContext.getSlotId());

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
        ImsLog.d("");

        mMtsJni = MtsJni.getInstance();
        mMtsJni.init(mHandler, mContext.getSlotId());
    }

    @VisibleForTesting
    public void startNativeConnection(MtsJni mtsJni) {
        mMtsJni = mtsJni;
        mMtsJni.init(mHandler, mContext.getSlotId());
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

    public boolean sendMessage(int smsFormat, String smsData, String targetAddress, int seqId) {
        ImsLog.d("smsFormat : " + smsFormat + ", encodedDataLength = " + smsData
                + ", complexData = " + targetAddress + ", seqId = " + seqId );

        if (smsData == null || targetAddress == null) {
            processNotifySendMoSmsError(smsFormat, seqId);
            return false;
        }

        Parcel parcel = Parcel.obtain();
        if (parcel == null) {
            ImsLog.e("parcel is null");
            processNotifySendMoSmsError(smsFormat, seqId);
            return false;
        }

        parcel.writeInt(MtsJni.NOTI_MTSENABLER_SEND_MO_SMS);
        parcel.writeInt(smsFormat);
        parcel.writeString(smsData);
        parcel.writeString(targetAddress);
        parcel.writeInt(seqId);
        mMtsJni.sendMessage(parcel, mContext.getSlotId());
        return true;
    }

    public boolean sendMessage(
            int smsFormat, String smsData, String psiSmsc, String dialedNumber, int seqId) {
        ImsLog.d("smsFormat : " + smsFormat + ", encodedDataLength = " + smsData
                + ", psiSmsc = " + psiSmsc + ", dialedNumber = " + dialedNumber
                + ", seqId = " + seqId );

        if (smsData == null || (dialedNumber == null && psiSmsc == null)) {
            processNotifySendMoSmsError(smsFormat, seqId);
            return false;
        }

        Parcel parcel = Parcel.obtain();
        if (parcel == null) {
            ImsLog.e("parcel is null");
            processNotifySendMoSmsError(smsFormat, seqId);
            return false;
        }

        String targetAddress = psiSmsc;

        if (mUseDialedNumber) {
            targetAddress = dialedNumber;
        }

        parcel.writeInt(MtsJni.NOTI_MTSENABLER_SEND_MO_SMS);
        parcel.writeInt(smsFormat);
        parcel.writeString(smsData);
        parcel.writeString(targetAddress);
        parcel.writeInt(seqId);
        mMtsJni.sendMessage(parcel, mContext.getSlotId());
        return true;
    }

    private void processNotifySendMoSmsError(int smsFormat, int seqId) {
        ImsLog.d("");

        if (mHandler == null) {
            ImsLog.e("mHandler is null");
            return;
        }

        Bundle bundle = new Bundle();
        bundle.putInt(REPORTMOSTATUS_REASON, MO_IMS_TEMP_FAILURE);
        bundle.putInt(REPORTMOSTATUS_SMSFORMAT, smsFormat);
        bundle.putInt(REPORTMOSTATUS_RETRYAFTER, 0);
        bundle.putInt(REPORTMOSTATUS_SEQID, seqId);

        Message msg = Message.obtain();
        msg.what = REQUEST_REPORT_MO_STATUS;
        msg.obj = bundle;
        mHandler.sendMessage(msg);
    }

    private int notifySendMtResult(int mtResult) {
        Parcel parcel = Parcel.obtain();
        if (parcel == null) {
            ImsLog.e("parcel is null");
            return (-1);
        }

        parcel.writeInt(MtsJni.NOTI_MTSENABLER_SEND_MT_RESULT);
        parcel.writeInt(mtResult);
        mMtsJni.sendMessage(parcel, mContext.getSlotId());
        return 0;
    }

    private class MessageHandler extends Handler {
        public MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.i("MessageHandler - what=" + msg.what);

            if (mListener == null) {
                ImsLog.e("mListener is null");
                return;
            }

            switch (msg.what) {
                case REQUEST_REPORT_MO_STATUS:
                    Bundle bundle = (Bundle)msg.obj;
                    int reason = bundle.getInt(REPORTMOSTATUS_REASON, 0);
                    int format = bundle.getInt(REPORTMOSTATUS_SMSFORMAT, 0);
                    int retryAfter = bundle.getInt(REPORTMOSTATUS_RETRYAFTER, 0);
                    int seqId = bundle.getInt(REPORTMOSTATUS_SEQID, -1);
                    mListener.notifyStatusForOutgoingMessage(reason, format, retryAfter, seqId);
                    break;

                case REQUEST_REPORT_MT_SMS:
                    int smsFormat = msg.arg1;
                    String encodedData = (String)msg.obj;
                    int mtResult = -1;
                    mtResult = mListener.notifyIncomingMessage(smsFormat, encodedData);
                    notifySendMtResult(mtResult);
                    break;

                default :
                    break;
            }
        }
    }
}
