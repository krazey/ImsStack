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

package com.android.imsstack.enabler.ssc;

import android.os.Handler;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

public class SscNetConnectionGov implements ISscNetConnectionGov {
    private static SscNetConnectionGov sSscNetConnectionGov = new SscNetConnectionGov();

    private HashMap<Integer, ISscNetConnection> mSscNetConnection = new HashMap<>();

    public static ISscNetConnectionGov getInstance() {
        return sSscNetConnectionGov;
    }

    @Override
    public void init(int slotId, EApnType apnType) {
        SscNetConnection sscNetConnection = new SscNetConnection(slotId);
        sscNetConnection.init(apnType);
        setSscNetConnection(slotId, sscNetConnection);
    }

    @Override
    public void cleanup(int slotId) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        if (netConnection != null) {
            netConnection.cleanup();
            mSscNetConnection.remove(slotId);
        }
    }

    @Override
    public boolean isConnected(int slotId) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        return (netConnection != null) ? netConnection.isConnected() : false;
    }

    @Override
    public boolean connect(int slotId) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        if (netConnection == null) {
            return false;
        }

        if (!netConnection.isPdnAvailable()) {
            int otherSlotId = slotId == 0 ? 1 : 0;
            ISscNetConnection otherNetConnection = mSscNetConnection.get(otherSlotId);
            if (otherNetConnection != null) {
                otherNetConnection.disconnect();
                return true; // it is to retry PDN connect after XCAP PDN release for other slot
            }
        }

        return netConnection.connect();
    }

    @Override
    public void disconnect(int slotId) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        if (netConnection != null) {
            netConnection.disconnect();
        }
    }

    @Override
    public int getNetworkType(int slotId) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        if (netConnection != null) {
            return netConnection.getNetworkType();
        }

        return TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    @Override
    public void setCallbackHandler(int slotId, Handler handler) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        if (netConnection != null) {
            netConnection.setCallbackHandler(handler);
        }
    }

    @Override
    public void refreshConnectionTimer(int slotId) {
        ISscNetConnection netConnection = mSscNetConnection.get(slotId);
        if (netConnection != null) {
            netConnection.refreshConnectionTimer();
        }
    }

    @VisibleForTesting
    protected void setSscNetConnection(int slotId, SscNetConnection sscNetConnection) {
        mSscNetConnection.put(slotId, sscNetConnection);
    }
}
