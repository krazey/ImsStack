/*
 * Copyright (C) 2025 The Android Open Source Project
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
package com.android.imsstack.core;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.TelephonyManager;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.util.ImsLog;

/**
 * Receives and processes stk setup event list intent.
 */
public class StkSetupEventReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        ImsLog.d("onReceive: " + intent);

        int subId = intent.getIntExtra(TelephonyManager.EXTRA_SUBSCRIPTION_ID,
                MSimUtils.INVALID_SUB_ID);
        SimInterface sim = AgentFactory.getInstance()
                .getAgent(SimInterface.class, MSimUtils.getSlotId(subId));

        UsatInterface usat = (sim != null) ? sim.getUsatInterface() : null;
        if (usat != null) {
            int[] setupEventList = intent.getIntArrayExtra(TelephonyManager.EXTRA_SETUP_EVENT_LIST);
            if (setupEventList != null) {
                usat.updateSetupEventList(setupEventList);
            }
        }
    }
}
