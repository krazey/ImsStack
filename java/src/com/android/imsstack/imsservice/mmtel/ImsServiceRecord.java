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

package com.android.imsstack.imsservice.mmtel;

import android.content.Context;

import androidx.annotation.NonNull;

import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

public class ImsServiceRecord {
    public static interface Listener {
        public void onServiceRecordStateChanged();
    };

    private final Object mLock = new Object();
    private final ImsContext mContext;
    private Listener mListener = null;
    private ImsConfigImpl mConfig = null;
    private ImsRegistrationImpl mRegistration = null;
    private ImsRegistrationTracker mRegTracker = null;
    private ImsCallApp mCallApp = null;
    private boolean mServiceUp = false;
    private ImsServiceRegistry mImsServiceRegistry;
    private MessageExecutor mExecutor = null;
    private int mSlotId = -1;


    public ImsServiceRecord(Context context, MessageExecutor executor, int phoneId) {
        log("ImsServiceRecord :: phoneId=" + phoneId);

        mContext = new ImsContext(context, executor, phoneId);

        mExecutor = executor;
        mImsServiceRegistry = ImsServiceRegistry.getInstance(mContext.getSlotId());
        mSlotId = phoneId;//NOTE: Slot id is used at ImsService to get service records mapped
        // phone id.

        // Create ImsRegistrationImplBase object
        getRegistration();
        getRegistrationTracker();
     }

    @VisibleForTesting
    public void setImsService(ImsServiceRegistry imsServiceRegistry) {
        mImsServiceRegistry = imsServiceRegistry;
    }

    /**
     * Broadcasts the IMS service up.
     */
    public void broadcastServiceUp() {
        Listener listener = null;

        synchronized (mLock) {
            if (mServiceUp) {
                log("Ims service is already up");
                return;
            }

            mServiceUp = true;

            listener = mListener;
        }

        logi("broadcastServiceUp :: phoneId=" + mContext.getPhoneId());

        if (listener != null) {
            listener.onServiceRecordStateChanged();
        }
    }

    /**
     * Broadcasts the IMS service down.
     */
    public void broadcastServiceDown() {
        Listener listener = null;

        synchronized (mLock) {
            if (!mServiceUp) {
                return;
            }

            mServiceUp = false;

            listener = mListener;
        }

        logi("broadcastServiceDown :: phoneId=" + mContext.getPhoneId());

        if (listener != null) {
            listener.onServiceRecordStateChanged();
        }
    }

    public ImsCallApp getCallApp() {
        synchronized (mLock) {
            return mCallApp;
        }
    }

    public ImsConfigImpl getConfig() {
        logi("getConfig :: slotid =" + mSlotId);

        synchronized (mLock) {
            if (mConfig == null) {
                mConfig = new ImsConfigImpl(mContext);
            }

            return mConfig;
        }
    }

    public ImsRegistrationImpl getRegistration() {
        logi("getRegistration :: slotid =" + mSlotId);
        synchronized (mLock) {
            if (mRegistration == null) {
                mRegistration = new ImsRegistrationImpl();
            }

            return mRegistration;
        }
    }

    public ImsRegistrationTracker getRegistrationTracker() {
        synchronized (mLock) {
            if (mRegTracker == null) {
                mRegTracker = new ImsRegistrationTracker(mContext, getRegistration());
            }

            return mRegTracker;
        }
    }

    public boolean isServiceUp() {
        synchronized (mLock) {
            return mServiceUp;
        }
    }

    public void reconfigure() {
        synchronized (mLock) {
            if (mConfig != null) {
                logi("reconfigure");
                mConfig.clear();
                mConfig.init();
            }
        }
    }

    public void setCallApp(ImsCallApp callApp) {
        synchronized (mLock) {
            mCallApp = callApp;
        }
    }

    public void setListener(Listener listener) {
        synchronized (mLock) {
            mListener = listener;
        }
    }

    public void disableIms() {
        mImsServiceRegistry.setImsEnabled(false);
    }

    public void enableIms() {
        mImsServiceRegistry.setImsEnabled(true);
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        if (mRegTracker != null) {
            mRegTracker.dump(pw);
        }

        if (mCallApp != null) {
            mCallApp.dump(pw);
        }
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }
}
