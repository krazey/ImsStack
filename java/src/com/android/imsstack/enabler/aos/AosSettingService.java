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
package com.android.imsstack.enabler.aos;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.SubscriptionListener;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

public class AosSettingService {

    private final Object mLock = new Object();
    private static final int EVENT_NATIVE_BOOT_COMPLETED = 1000;
    private static final int EVENT_MOBILE_DATA_STATE_CHANGED = 1001;
    private static final int EVENT_REBOOT = 1002;
    private static final int EVENT_SHUTDOWN = 1003;

    private UserMobileDataStateListener mUserMobileDataStateListener = null;
    private Handler mHandler;

    private int mSlotId = 0;
    private int mSubId = MSimUtils.INVALID_SUB_ID;
    private boolean mMobileDataEnabled = false;
    private SubscriptionListenerProxy mSubscriptionListener;
    private IntentReceiverListener mIntentReceiverListener = null;

    public AosSettingService(int slotId) {
        ImsLog.d(slotId, "");
        mSlotId = slotId;
    }

    public void init() {
        ImsLog.d(mSlotId, "");

        mHandler = new SettingServiceHandler();

        IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
        if (jniEvt != null) {
            jniEvt.registerForNativeBootComplete(mHandler, EVENT_NATIVE_BOOT_COMPLETED, null);
        }

        mIntentReceiverListener = new IntentReceiverListener();
        AppContext.getInstance().registerReceiver(mIntentReceiverListener,
                mIntentReceiverListener.getFilter(), Context.RECEIVER_EXPORTED);

        mSubscriptionListener = new SubscriptionListenerProxy();
        ISubscription isub = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        if (isub != null) {
            isub.addListener(mSubscriptionListener);
        }

        mSubId = MSimUtils.getSubId(mSlotId);
        if (mSubId == MSimUtils.INVALID_SUB_ID) {
            return;
        }

        mUserMobileDataStateListener = new UserMobileDataStateListener(mSubId);
        setListener(mUserMobileDataStateListener);
    }

    public void cleanup() {
        ImsLog.d(mSlotId, "");

        if (mUserMobileDataStateListener != null) {
            removeListener(mUserMobileDataStateListener);
            mUserMobileDataStateListener = null;
        }

        if (mSubscriptionListener != null) {
            ISubscription isub = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (isub != null) {
                isub.removeListener(mSubscriptionListener);
            }
            mSubscriptionListener = null;
        }

        if (mIntentReceiverListener != null) {
            AppContext.getInstance().unregisterReceiver(mIntentReceiverListener);
            mIntentReceiverListener = null;
        }

        if (mHandler != null) {
            IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
            if (jniEvt != null) {
                jniEvt.unregisterForNativeBootComplete(mHandler);
            }

            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
    }

    private void setListener(UserMobileDataStateListener listener) {
        ImsLog.d(mSlotId, "");

        TelephonyManager tm = AppContext.getTelephonyManager(listener.getSubId());
        if (tm != null) {
            tm.registerTelephonyCallback(AppContext.getInstance().getMainExecutor(), listener);
        }
    }

    private void removeListener(UserMobileDataStateListener listener) {
        ImsLog.d(mSlotId, "");

        TelephonyManager tm = AppContext.getTelephonyManager(listener.getSubId());
        if (tm != null) {
            tm.unregisterTelephonyCallback(listener);
        }
    }

    private void updateSubscription(int subId) {
        ImsLog.d(mSlotId, "");

        synchronized (mLock) {
            int slotId = MSimUtils.getSlotId(subId);
            if (mSlotId != slotId) {
                ISubscription isub = (ISubscription)AgentFactory.getAgent(
                        AgentFactory.SUBSCRIPTION);
                if (isub == null) {
                    return;
                }

                subId = isub.getSubId(mSlotId);
            }

            if (mSubId == subId || (subId == MSimUtils.INVALID_SUB_ID)) {
                return;
            }

            mSubId = subId;
            ImsLog.i(mSlotId, "updateSubscription :: subId=" + subId);

            if (mUserMobileDataStateListener != null) {
                removeListener(mUserMobileDataStateListener);
                mUserMobileDataStateListener = new UserMobileDataStateListener(mSubId);
                setListener(mUserMobileDataStateListener);
            }
        }
    }

    public void handleMobileDataStateChanged(Message msg) {
        boolean enabled = (boolean) msg.obj;

        if (mMobileDataEnabled == enabled) {
            return;
        }

        ImsLog.d(mSlotId, "enabled : " + enabled);
        mMobileDataEnabled = enabled;

        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo != null) {
            aosInfo.notifyMobileDataSetting(enabled);
        }
    }

    private void notifyPowerOff() {
        ImsLog.d(mSlotId, "");
        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo != null) {
            aosInfo.notifyPowerOff();
        }
    }

    private final class SettingServiceHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                ImsLog.d(mSlotId, "msg is null");
                return;
            }

            ImsLog.i(mSlotId, "handleMessage :: msg= " + msg.what);

            switch (msg.what) {
                case EVENT_NATIVE_BOOT_COMPLETED:
                    handleNativeBootCompleted();
                    break;

                case EVENT_MOBILE_DATA_STATE_CHANGED:
                    handleMobileDataStateChanged(msg);
                    break;

                case EVENT_REBOOT: // FALL-THROUGH
                case EVENT_SHUTDOWN:
                    notifyPowerOff();
                    break;

                default:
                    break;
            }
        }

        private void handleNativeBootCompleted() {
            ImsLog.d(mSlotId, "");
            IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
            if (aosInfo != null) {
                aosInfo.notifyMobileDataSetting(mMobileDataEnabled);
            }
        }
    }

    private final class SubscriptionListenerProxy extends SubscriptionListener {

        public SubscriptionListenerProxy() {
        }

        @Override
        public void onSimLoadCompleted(int slotId) {
            if (mSlotId != slotId) {
                return;
            }

            int subId = MSimUtils.getSubId(mSlotId);
            updateSubscription(subId);
        }

        @Override
        public void onDefaultSubscriptionChanged(int subId) {
            updateSubscription(subId);
        }

        @Override
        public void onDefaultDataSubscriptionChanged(int subId) {
            updateSubscription(subId);
        }
    }

    private final class UserMobileDataStateListener extends TelephonyCallback implements
            TelephonyCallback.UserMobileDataStateListener {

        private final int mSubId;

        public UserMobileDataStateListener(int subId) {
            mSubId = subId;
            ImsLog.i(mSlotId, "UserMobileDataStateListener :: subId=" + subId);
        }

        public int getSubId() {
            return mSubId;
        }

        @Override
        public void onUserMobileDataStateChanged(boolean enabled) {
            ImsLog.d(mSlotId, "");
            Message.obtain(mHandler, EVENT_MOBILE_DATA_STATE_CHANGED, enabled).sendToTarget();
        }
    }

    private class IntentReceiverListener extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public IntentReceiverListener() {
            mIntentFilter.addAction(Intent.ACTION_REBOOT);
            mIntentFilter.addAction(Intent.ACTION_SHUTDOWN);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            if (intent == null || mHandler == null) {
                return;
            }

            ImsLog.d(mSlotId, printIntent(intent));

            String action = intent.getAction();
            Message msg = Message.obtain();

            if (Intent.ACTION_REBOOT.equals(action)) {
                msg.what = EVENT_REBOOT;
            } else if (Intent.ACTION_SHUTDOWN.equals(action)) {
                msg.what = EVENT_SHUTDOWN;
            } else {
                return;
            }

            mHandler.sendMessage(msg);
        }

        private String printIntent(Intent intent) {
            if (intent == null) {
                return "null";
            }

            StringBuilder sb = new StringBuilder();
            sb.append("intent=" + intent.getAction());
            sb.append(", extra=[");

            Bundle extras = intent.getExtras();
            if (extras != null) {
                for (String key : extras.keySet()) {
                    sb.append(key + "=" + extras.get(key) + " ");
                }
            }
            sb.append("]");

            return sb.toString();
        }
    }
}