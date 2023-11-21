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
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyCallback;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public class AosSettingService {

    private final Object mLock = new Object();
    @VisibleForTesting
    protected static final int EVENT_MOBILE_DATA_STATE_CHANGED = 1001;
    @VisibleForTesting
    protected static final int EVENT_REBOOT = 1002;
    @VisibleForTesting
    protected static final int EVENT_SHUTDOWN = 1003;

    @VisibleForTesting
    protected UserMobileDataStateListener mUserMobileDataStateListener = null;
    @VisibleForTesting
    protected Handler mHandler = null;

    private int mSlotId = 0;
    private int mSubId = MSimUtils.INVALID_SUB_ID;
    private boolean mMobileDataEnabled = false;
    @VisibleForTesting
    protected IntentReceiverListener mIntentReceiverListener = null;
    private Sim.Listener mSimListener;
    private NativeStateListener mNativeStateListener;

    public AosSettingService(int slotId) {
        ImsLog.d(slotId, "");
        mSlotId = slotId;
    }

    public void init() {
        ImsLog.d(mSlotId, "");

        mHandler = new SettingServiceHandler(Looper.myLooper());

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        if (nsi != null) {
            mNativeStateListener = new NativeStateListener();
            nsi.addListener(mNativeStateListener);
        }

        mIntentReceiverListener = new IntentReceiverListener();
        mIntentReceiverListener.register();

        mSimListener = new Sim.Listener() {
                @Override
                public void onSimStateChanged() {
                    updateSubscription();
                }
            };
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mSimListener);
        }

        mSubId = MSimUtils.getSubId(mSlotId);
        if (mSubId != MSimUtils.INVALID_SUB_ID) {
            mUserMobileDataStateListener = new UserMobileDataStateListener(mSubId);
            setListener(mUserMobileDataStateListener);
        }
    }

    public void cleanup() {
        ImsLog.d(mSlotId, "");

        if (mUserMobileDataStateListener != null) {
            removeListener(mUserMobileDataStateListener);
            mUserMobileDataStateListener = null;
        }

        if (mSimListener != null) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (sim != null) {
                sim.removeListener(mSimListener);
            }
            mSimListener = null;
        }

        if (mIntentReceiverListener != null) {
            mIntentReceiverListener.unregister();
            mIntentReceiverListener = null;
        }

        if (mNativeStateListener != null) {
            NativeStateInterface nsi =
                    AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
    }

    private void setListener(UserMobileDataStateListener listener) {
        ImsLog.d(mSlotId, "");

        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(listener.getSubId());
        tmp.registerTelephonyCallback(AppContext.getInstance().getMainExecutor(), listener);
    }

    private void removeListener(UserMobileDataStateListener listener) {
        ImsLog.d(mSlotId, "");

        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(listener.getSubId());
        tmp.unregisterTelephonyCallback(listener);
    }

    private void updateSubscription() {
        ImsLog.d(mSlotId, "");

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim == null || !sim.isSimLoadCompleted()) {
            return;
        }

        synchronized (mLock) {
            int subId = sim.getSubId();

            if (mSubId == subId || subId == MSimUtils.INVALID_SUB_ID) {
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

    private void handleMobileDataStateChanged(Message msg) {
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
        private SettingServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg != null) {
                ImsLog.i(mSlotId, "handleMessage :: msg= " + msg.what);

                switch (msg.what) {
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
        }
    }

    @VisibleForTesting
    protected final class UserMobileDataStateListener extends TelephonyCallback implements
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

    @VisibleForTesting
    protected class IntentReceiverListener extends BroadcastReceiver {
        public void register() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_REBOOT);
            filter.addAction(Intent.ACTION_SHUTDOWN);

            AppContext.getInstance().getBroadcastReceiverProxy().registerReceiver(this, filter);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            ImsLog.d(mSlotId, printIntent(intent));

            if (intent == null || mHandler == null) {
                return;
            }

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

            return sb.toString();
        }
    }

    private final class NativeStateListener implements NativeStateInterface.Listener {
        @Override
        public void onNativeServiceReady() {
            ImsLog.i(mSlotId, "NativeState: service ready.");
            IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
            if (aosInfo != null) {
                aosInfo.notifyMobileDataSetting(mMobileDataEnabled);
            }
        }
    }
}