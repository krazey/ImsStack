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

package com.android.imsstack.enabler.acs.impl;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyRegistryManager;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;

/**
 * This class handles the Intent receiver and SubscriptionListener.
 */
public class EventReceiver {

    /**
     * Notify Event received (Subscription changed etc)
     */
    public interface EventReceiverCallback {
        /**
         * Notify intent received
         */
        void onReceivedIntent();

        /**
         * Notify subscription changed
         */
        void onSubscriptionChanged();
    }

    private static EventReceiver sInstance;
    private final ArrayList<EventReceiverCallback> mCallbackList =
            new ArrayList<EventReceiverCallback>();
    private final Object mLock = new Object();
    private final Context mContext;
    private final TelephonyRegistryManager mTelephonyRegistryManager;

    private final SubscriptionManager.OnSubscriptionsChangedListener mSubChangedListener =
            new SubscriptionManager.OnSubscriptionsChangedListener() {
                @Override
                public void onSubscriptionsChanged() {
                    synchronized (mLock) {
                        for (EventReceiverCallback cb : mCallbackList) {
                            cb.onSubscriptionChanged();
                        }
                    }
                }
            };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            synchronized (mLock) {
                for (EventReceiverCallback cb : mCallbackList) {
                    cb.onReceivedIntent();
                }
            }
        }
    };

    private boolean mRegistered;

    /**
     * Returns a EventReceiver
     * @param context Context
     * @return Instance of the EventReceiver
     */
    public static EventReceiver getInstance(Context context) {
        synchronized (sInstance) {
            if (sInstance == null) {
                sInstance = new EventReceiver(context);
            }
        }

        return sInstance;
    }

    /**
     * register callback object.
     * @param callback callback instance to be registered
     * @return true if registering is success, otherwise is false
     */
    public boolean registerCallback(EventReceiverCallback callback) {
        synchronized (mLock) {
            if (!mCallbackList.contains(callback)) {
                mCallbackList.add(callback);
            } else {
                ImsLog.i("callback already exist");
                return false;
            }
        }
        checkEvent();
        return true;
    }

    /**
     * unregister callback object. if there is no registered callback
     * @param callback callback instance to be unregistered
     */
    public void unregisterCallback(EventReceiverCallback callback) {
        synchronized (mLock) {
            if (mCallbackList.contains(callback)) {
                mCallbackList.remove(callback);
            } else {
                ImsLog.i("callback is not exist");
                return;
            }
        }
        checkEvent();
    }

    private EventReceiver(Context context) {
        mRegistered = false;
        mContext = context;
        mTelephonyRegistryManager = context.getSystemService(TelephonyRegistryManager.class);
    }

    @VisibleForTesting
    public EventReceiver(Context context, TelephonyRegistryManager telephonyRegistryManager) {
        mRegistered = false;
        mContext = context;
        mTelephonyRegistryManager = telephonyRegistryManager;
    }

    private void checkEvent() {
        int size;
        synchronized (mLock) {
            size = mCallbackList.size();
        }

        if (size == 1 && !mRegistered) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
            mContext.registerReceiver(mReceiver, filter);

            mTelephonyRegistryManager.addOnSubscriptionsChangedListener(
                    mSubChangedListener, mSubChangedListener.getHandlerExecutor());
            mRegistered = true;
        } else if (size == 0) {
            mTelephonyRegistryManager.removeOnSubscriptionsChangedListener(mSubChangedListener);

            mContext.unregisterReceiver(mReceiver);

            mRegistered = false;
        }
    }
}
