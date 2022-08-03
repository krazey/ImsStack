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

package com.android.imsstack.enabler.uce.impl.jni;

import android.os.Parcel;

import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

interface UceJNIMessageHandlerFunc {
    public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel);
}

public class UceJNI {
    public static final int INTERFACETYPE_IMS_UCE = IUIMS.APP_UCE;
    private static UceJNI mUceJni               = null;
    private HashMap<Integer, UceJNIImsListener> mUceJNIImsListenerMap =
        new HashMap<Integer, UceJNIImsListener>();
    private HashMap<Integer, Long> mNativeObjMap = new HashMap<Integer, Long>();

    static public UceJNI getInstance() {
        synchronized (UceJNI.class) {
            if (mUceJni == null) {
                mUceJni = new UceJNI();
            }
        }
        return mUceJni;
    }

    public void init(int nSimSlot) {
        ImsLog.d(nSimSlot, "");
        if (!mUceJNIImsListenerMap.containsKey(nSimSlot)) {
            long nativeObj = JniIms.getInterface(INTERFACETYPE_IMS_UCE, nSimSlot);
            ImsLog.i(nSimSlot, "mNativeObj: " + nativeObj);
            mNativeObjMap.put(nSimSlot, nativeObj);
            mUceJNIImsListenerMap.put(nSimSlot, new UceJNIImsListener());
            JniIms.setListener(nativeObj, mUceJNIImsListenerMap.get(nSimSlot));
        }
    }

    public void release(int nSimSlot) {
        ImsLog.d(nSimSlot, "");
        if (mUceJNIImsListenerMap.containsKey(nSimSlot)) {
            UceJNIImsListener mUceJNIImsListener = mUceJNIImsListenerMap.get(nSimSlot);
            long nativeObj = mNativeObjMap.get(nSimSlot);
            ImsLog.i(nSimSlot, "mNativeObj: " + nativeObj);
            JniIms.releaseInterface(nativeObj);
            JniIms.removeListener(nativeObj);
        }
        mNativeObjMap.remove(nSimSlot);
        mUceJNIImsListenerMap.remove(nSimSlot);
    }

    public void addListener(int nSimSlot, IUceJNIListener mListener, int nMsgType){
        ImsLog.d(nSimSlot, "");
        UceJNIImsListener mUceJNIImsListener = mUceJNIImsListenerMap.get(nSimSlot);
        if (mUceJNIImsListener == null) {
            ImsLog.i(nSimSlot, "mUceJNIImsListener is null");
            return;
        }
        mUceJNIImsListener.addListener(mListener, nMsgType);
    }

    public void sendMessage(int nSimSlot, Parcel parcel) {
        ImsLog.d(nSimSlot, "");
        if (parcel == null) {
            ImsLog.i(nSimSlot, "parcel is null");
            return;
        }
        ImsLog.i(nSimSlot, "send Message to native.");
        byte[] baData = parcel.marshall();

        if (mNativeObjMap.containsKey(nSimSlot)) {
            JniIms.sendData(mNativeObjMap.get(nSimSlot), baData);
        }

        parcel.recycle();
        parcel = null;
    }

    private class UceJNIImsListener implements JniImsListener{
        private HashMap<Integer, ArrayList<IUceJNIListener>> mListenersMap =
            new HashMap<Integer, ArrayList<IUceJNIListener>>();
        private Map<Integer, UceJNIMessageHandlerFunc> mMessageHandler =
            new HashMap<Integer, UceJNIMessageHandlerFunc>();

        public UceJNIImsListener() {
            mMessageHandler.put(UceMessage.UCE_IMS_AGENT_CONNECTED_IND, mServiceStatusHandler);
            mMessageHandler.put(UceMessage.UCE_IMS_AGENT_DISCONNECTED_IND, mServiceStatusHandler);
            mMessageHandler.put(UceMessage.UCE_IMS_AGENT_REFRESHED_IND, mServiceStatusHandler);

            mMessageHandler.put(UceMessage.UCE_NETWORK_CHANGED, mNetworkStatusHandler);

            mMessageHandler.put(UceMessage.UCE_PUBLISH_UPDATED_IND, mPublishStatusHandler);
            mMessageHandler.put(UceMessage.UCE_UNPUBLISHED_IND, mPublishStatusHandler);

            mMessageHandler.put(UceMessage.UCE_OPTIONS_RECEIVED_IND, mReceivedRemoteOptionsHandler);

            mMessageHandler.put(UceMessage.UCE_PUBLISH_RESPONSE_IND, mPublishControllerHandler);
            mMessageHandler.put(UceMessage.UCE_PUBLISH_CMD_ERROR_IND, mPublishControllerHandler);

            mMessageHandler.put(UceMessage.UCE_SUBSCRIBE_RESPONSE_IND, mSubscribeControllerHandler);
            mMessageHandler.put(UceMessage.UCE_PRESENCE_NOTIFY_IND, mSubscribeControllerHandler);
            mMessageHandler.put(UceMessage.UCE_SUBSCRIBE_CMD_ERROR_IND,
                mSubscribeControllerHandler);
            mMessageHandler.put(UceMessage.UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND,
                mSubscribeControllerHandler);
            mMessageHandler.put(UceMessage.UCE_SUBSCRIBE_TERMINATED_IND,
                mSubscribeControllerHandler);

            mMessageHandler.put(UceMessage.UCE_OPTIONS_RESPONSE_IND, mOptionsControllerHandler);
            mMessageHandler.put(UceMessage.UCE_OPTIONS_CMD_ERROR_IND, mOptionsControllerHandler);
        }

        public void addListener(IUceJNIListener mListener, int nMsgType) {
            if (mListener == null) {
                ImsLog.i("ISPIJNIListener is null. MsgType : " + nMsgType);
                return;
            }
            ImsLog.i("( Listener : " + mListener + ", MsgType : " + nMsgType + " )");
            synchronized (mListenersMap) {
                if (mListenersMap.containsKey(nMsgType)) {
                    ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(nMsgType);
                    if (!mListenerList.contains(mListener)) {
                        mListenerList.add(mListener);
                    }
                } else {
                    ArrayList<IUceJNIListener> mListenerList = new ArrayList<IUceJNIListener>();
                    mListenerList.add(mListener);
                    mListenersMap.put(nMsgType, mListenerList);
                }
            }
        }

        @Override
        public void onMessage(Parcel parcel) {
            byte[] baData = parcel.marshall();
            Parcel mCopyParcel = Parcel.obtain();
            int nMsgType = parcel.readInt();
            synchronized (mListenersMap) {
                if (mListenersMap.containsKey(nMsgType)) {
                    UceJNIMessageHandlerFunc objMsgHandler = mMessageHandler.get(nMsgType);
                    if (objMsgHandler == null) {
                        ImsLog.i("message can not be handled.");
                        return;
                    }
                    objMsgHandler.onMessage(nMsgType, baData, mCopyParcel);
                } else {
                    ImsLog.i("do not contain this msg type : " + nMsgType);
                }
            }
            ImsLog.i("nMsgType : " + nMsgType);
            mCopyParcel.recycle();
            return;
        }

        private UceJNIMessageHandlerFunc mServiceStatusHandler = new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onServiceStatusMessage(mCopyParcel);
                }
            }
        };

        private UceJNIMessageHandlerFunc mNetworkStatusHandler = new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onNetworkStatusMessage(mCopyParcel);
                }
            }
        };

        private UceJNIMessageHandlerFunc mPublishStatusHandler = new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onPublishStatusMessage(mCopyParcel);
                }
            }
        };

        private UceJNIMessageHandlerFunc mSubscribeControllerHandler =
            new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onSubscribeResponseMessage(mCopyParcel);
                }
            }
        };

        private UceJNIMessageHandlerFunc mReceivedRemoteOptionsHandler =
            new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onReceivedRemoteOptionsMessage(mCopyParcel);
                }
            }
        };

        private UceJNIMessageHandlerFunc mPublishControllerHandler =
            new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onPublishResponseMessage(mCopyParcel);
                }
            }
        };

        private UceJNIMessageHandlerFunc mOptionsControllerHandler =
            new UceJNIMessageHandlerFunc() {
            @Override
            public void onMessage(int messageType, byte[] baData, Parcel mCopyParcel) {
                ImsLog.i("- messageType:" + messageType);
                ArrayList<IUceJNIListener> mListenerList = mListenersMap.get(messageType);
                for (int index = 0; index < mListenerList.size(); index++) {
                    mCopyParcel.unmarshall(baData, 0, baData.length);
                    mCopyParcel.setDataPosition(0);
                    mListenerList.get(index).onOptionsResponseMessage(mCopyParcel);
                }
            }
        };
    }
}
