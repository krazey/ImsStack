/*
 * Copyright (C) 2024 The Android Open Source Project
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

package com.android.imsstack.enabler.ssc;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.util.ImsLog;

public class SscNetConnectionWifi extends SscNetConnection {
    protected SscNetConnectionWifi(int slotId) {
        super(slotId);
        ImsLog.d("");
    }

    @Override
    public void init(EApnType apnType) {
        mApnType = apnType;
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "cleanup");
    }

    @Override
    public boolean isConnected() {
        ImsLog.d("");
        WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
        boolean wifiConnected = (wifi != null && wifi.isWifiConnected());
        ImsLog.d("wifi connected: " + wifiConnected);

        return wifiConnected;
    }

    @Override
    public boolean connect() {
        if (isConnected()) {
            ImsLog.e(mSlotId, "Wifi is already connected");
            return true;
        }

        return false;
    }

    @Override
    public void disconnect() {
        ImsLog.d("");
    }
}
