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
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

public class MtsJni {

    /**
     * Wrapper for JNI calls to MTS.
     * Can be mocked for testing.
     */
    @VisibleForTesting
    public static class JniImsProxyWrapper {
        /**
         * Gets the native interface for a given object and slot.
         * @param objectId The ID of the object.
         * @param slotId The slot ID.
         * @return A handle to the native object.
         */
        public long getInterface(int objectId, int slotId) {
            return JniImsProxy.getInterface(objectId, slotId);
        }

        /**
         * Sets a listener on the native object to receive callbacks.
         * @param nativeObj The handle to the native object.
         * @param listener The listener to set.
         */
        public void setListener(long nativeObj, JniImsListener listener) {
            JniImsProxy.setListener(nativeObj, listener);
        }

        /**
         * Releases the native interface.
         * @param nativeObj The handle to the native object.
         */
        public void releaseInterface(long nativeObj) {
            JniImsProxy.releaseInterface(nativeObj);
        }

        /**
         * Removes the listener from the native object.
         * @param nativeObj The handle to the native object.
         */
        public void removeListener(long nativeObj) {
            JniImsProxy.removeListener(nativeObj);
        }

        /**
         * Sends a byte array to the native layer.
         * @param nativeObj The handle to the native object.
         * @param data The data to send.
         */
        public void sendData(long nativeObj, byte[] data) {
            JniImsProxy.sendData(nativeObj, data);
        }
    }

    /* JNI Message */
    private static final int JAVA2MTSENABLER = 1000;
    private static final int MTSENABLER2JAVA = 1050;

    public static final int NOTI_MTSENABLER_SEND_MO_SMS = JAVA2MTSENABLER + 1;
    public static final int NOTI_MTSENABLER_MO_SMS_TIMED_OUT = JAVA2MTSENABLER + 2;
    public static final int NOTI_MTSENABLER_MT_SMS_TIMED_OUT = JAVA2MTSENABLER + 3;

    public static final int REPORT_MTS_MO_STATUS = MTSENABLER2JAVA + 1;
    public static final int REPORT_MTS_MT_SMS = MTSENABLER2JAVA + 2;

    private static MtsJni mMtsJni = null;
    private final HashMap<Integer, MtsJniImsListener> mMtsJniImsListenerMap =
            new HashMap<Integer, MtsJniImsListener>();
    private final HashMap<Integer, Long> mNativeObjMap = new HashMap<Integer, Long>();
    private final JniImsProxyWrapper mJniProxy;

    @VisibleForTesting
    public MtsJni() {
        mJniProxy = new JniImsProxyWrapper();
    }

    @VisibleForTesting
    public MtsJni(JniImsProxyWrapper jniProxy) {
        mJniProxy = jniProxy;
    }

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
            long nativeObj = mJniProxy.getInterface(JniObjectId.MTS, slotId);
            ImsLog.i(slotId, "mNativeObj: " + nativeObj);
            mNativeObjMap.put(slotId, nativeObj);
            mMtsJniImsListenerMap.put(slotId, new MtsJniImsListener());
            mJniProxy.setListener(nativeObj, mMtsJniImsListenerMap.get(slotId));
        }
        mMtsJniImsListenerMap.get(slotId).setHandler(handler);
    }

    public void release(int slotId) {
        ImsLog.d(slotId, "");

        if (mMtsJniImsListenerMap.containsKey(slotId)) {
            long nativeObj = mNativeObjMap.get(slotId);
            ImsLog.i(slotId, "mNativeObj: " + nativeObj);
            mJniProxy.releaseInterface(nativeObj);
            mJniProxy.removeListener(nativeObj);
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
            mJniProxy.sendData(mNativeObjMap.get(slotId), baData);
        }

        parcel.recycle();
        parcel = null;
    }

    public static class MtsJniImsListener implements JniImsListener {
        private Handler mHandler;

        @VisibleForTesting
        public void setHandler(Handler handler) {
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
                    msg.what = MtsController.REQUEST_REPORT_MT_SMS;
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
