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
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.util.ImsLog;

import java.util.HashMap;

public class MtsJni {
    /* JNI Message */
    private static final int JAVA2MTSENABLER = 1000;
    private static final int MTSENABLER2JAVA = 1050;

    public static final int NOTI_MTSENABLER_SEND_MO_SMS = JAVA2MTSENABLER + 1;
    public static final int NOTI_MTSENABLER_MO_SMS_TIMED_OUT = JAVA2MTSENABLER + 2;

    private static final int REPORT_MTS_MO_STATUS = MTSENABLER2JAVA + 1;
    private static final int REPORT_MTS_MT_SMS = MTSENABLER2JAVA + 2;

    private static MtsJni mMtsJni = null;
    private HashMap<Integer, MtsJniImsListener> mMtsJniImsListenerMap =
            new HashMap<Integer, MtsJniImsListener>();
    private HashMap<Integer, Long> mNativeObjMap = new HashMap<Integer, Long>();

    static public MtsJni getInstance() {
        synchronized (MtsJni.class) {
            if (mMtsJni == null) {
                mMtsJni = new MtsJni();
            }
        }
        return mMtsJni;
    }

    public void init(Handler handler, int slotId) {
        ImsLog.d(slotId, "");
        if (!mMtsJniImsListenerMap.containsKey(slotId)) {
            long nativeObj = JniImsProxy.getInterface(JniObjectId.MTS, slotId);
            ImsLog.i(slotId, "mNativeObj: " + nativeObj);
            mNativeObjMap.put(slotId, nativeObj);
            mMtsJniImsListenerMap.put(slotId, new MtsJniImsListener());
            JniImsProxy.setListener(nativeObj, mMtsJniImsListenerMap.get(slotId));
        }
        mMtsJniImsListenerMap.get(slotId).setHandler(handler);
    }

    public void release(int slotId) {
        ImsLog.d(slotId, "");

        if (mMtsJniImsListenerMap.containsKey(slotId)) {
            long nativeObj = mNativeObjMap.get(slotId);
            ImsLog.i(slotId, "mNativeObj: " + nativeObj);
            JniImsProxy.releaseInterface(nativeObj);
            JniImsProxy.removeListener(nativeObj);
        }

        mNativeObjMap.remove(slotId);
        mMtsJniImsListenerMap.remove(slotId);
    }

    public void sendMessage(Parcel parcel, int slotId) {
        ImsLog.d(slotId, "");
        if (parcel == null) {
            ImsLog.i("parcel is null");
            return;
        }
        ImsLog.i("send Message to native.");
        byte[] baData = parcel.marshall();

        if (mNativeObjMap.containsKey(slotId)) {
            JniImsProxy.sendData(mNativeObjMap.get(slotId), baData);
        }

        parcel.recycle();
        parcel = null;
    }

    private class MtsJniImsListener implements JniImsListener {
        private Handler mHandler;

        private void setHandler(Handler handler) {
            mHandler = handler;
        }

        @Override
        public void onMessage(Parcel parcel) {
            int msgName = parcel.readInt();
            ImsLog.d("msg=" + msgName);

            switch (msgName) {
                case REPORT_MTS_MO_STATUS: {
                    Bundle bundle = new Bundle();
                    bundle.putInt(MtsController.REPORTMOSTATUS_REASON, parcel.readInt());
                    bundle.putInt(MtsController.REPORTMOSTATUS_SMSFORMAT, parcel.readInt());
                    bundle.putInt(MtsController.REPORTMOSTATUS_SEQID, parcel.readInt());

                    Message msg = Message.obtain();
                    msg.what = MtsController.REQUEST_REPORT_MO_STATUS;
                    msg.obj = bundle;
                    mHandler.sendMessage(msg);
                    break;
                }

                case REPORT_MTS_MT_SMS: {
                    int smsformat = parcel.readInt();
                    String encodedData = parcel.readString();
                    Message msg = Message.obtain();
                    msg.what =  MtsController.REQUEST_REPORT_MT_SMS;
                    msg.arg1 = smsformat;
                    msg.obj = encodedData;
                    mHandler.sendMessage(msg);
                    break;
                }

                default:
                    ImsLog.e("OnMessage : no handle message");
                    break;
            }
        }
    }
}
