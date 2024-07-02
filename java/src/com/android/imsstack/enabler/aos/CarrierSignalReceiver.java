/*
 * Copyright (C) 2023 The Android Open Source Project
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
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * Receives and processes carrier signal PCO value intents.
 */
public class CarrierSignalReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();
        final int nSlotId = getSlotId(intent.getIntExtra(
                SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID));

        ImsLog.i("[" + nSlotId + "]" + ImsLog.lastSubString(action, "."));

        if (!TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE.equals(action)) {
            return;
        }

        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(nSlotId);
        if (aosInfo != null) {
            aosInfo.notifyCarrierSignalPcoValueChanged(intent);
        }
    }

    @VisibleForTesting
    int getSlotId(int subId) {
        return MSimUtils.getSlotId(subId);
    }
}
