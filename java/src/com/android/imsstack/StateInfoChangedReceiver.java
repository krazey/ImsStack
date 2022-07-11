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
package com.android.imsstack;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.telephony.TelephonyIntents;

public class StateInfoChangedReceiver {
    private final IntentReceiver mIntentReceiver = new IntentReceiver();
    private IStateInfoChangedObserver mObserver;

    public StateInfoChangedReceiver() {
    }

    public void init(Context context, IStateInfoChangedObserver observer) {
        mObserver = observer;

        mIntentReceiver.register(context);
    }

    private final class IntentReceiver extends BroadcastReceiver {
        public void register(Context c) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);

            if (MSimUtils.isMultiSimEnabled()) {
                filter.addAction(TelephonyManager.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
            }

            filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);

            c.registerReceiver(this, filter, null,
                    AppContext.getInstance().getMainHandler(), Context.RECEIVER_EXPORTED);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(Log.TAG, "Received : " + intent);

            if (mObserver == null) {
                return;
            }

            String action = intent.getAction();

            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                mObserver.notifyStateInfoChanged(intent);
            } else if (TelephonyManager.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED.equals(action)) {
                mObserver.notifyStateInfoChanged(intent);
            } else if (CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED.equals(action)) {
                mObserver.notifyStateInfoChanged(intent);
            } else {
                Log.w(Log.TAG, "Unexpected Intent : " + intent);
            }
        }
    }
}
