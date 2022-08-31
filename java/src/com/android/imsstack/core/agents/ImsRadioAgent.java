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
package com.android.imsstack.core.agents;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

import com.android.imsstack.util.ImsLog;

import java.util.HashMap;
import java.util.Map;

/**
 * This provides the implementation of the ims radio interface for notifying the IMS traffic
 * activities to modem, getting NAS/RRC connection setup result details from modem and
 * triggering EPS fallback to modem.
 */
public class ImsRadioAgent implements ImsRadioInterface {
    private final int mSlotId;
    private ImsRadioHandler mHandler;
    private Map<Integer, ConnectionListener> mConnectionListeners =
                new HashMap<Integer, ConnectionListener>();

    private static final int EVENT_CONNECTION_SETUP_PREPARED = 100;

    public ImsRadioAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        mHandler = new ImsRadioHandler();
    }

    @Override
    public void cleanup() {
        mHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public boolean isImsTrafficAllowed(int trafficType) {
        return true;
    }

    @Override
    public void startImsTraffic(int trafficType, int accessNetworkType,
            ConnectionListener listener) {

        mConnectionListeners.put(trafficType, listener);

        ImsLog.d(mSlotId, "startImsTraffic");

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                ConnectionListener listener = mConnectionListeners.get(trafficType);

                if (listener != null) {
                    listener.onConnectionSetupPrepared();
                }
            }
        });
    }

    @Override
    public void stopImsTraffic(int trafficType) {
    }

    @Override
    public void addListenerForTrafficPriority(TrafficPriorityListener listener) {
    }

    @Override
    public void removeListenerForTrafficPriority(TrafficPriorityListener listener) {
    }

    private final class ImsRadioHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
        }
    }
}
