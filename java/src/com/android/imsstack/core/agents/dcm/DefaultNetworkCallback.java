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

package com.android.imsstack.core.agents.dcm;

import android.annotation.NonNull;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkSpecifier;
import android.net.TelephonyNetworkSpecifier;
import android.os.Handler;
import android.os.Message;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.util.ImsLog;

/**
 * This is NetworkCallback to receive notifications about changes of default network
 */
public class DefaultNetworkCallback extends ConnectivityManager.NetworkCallback {
    protected final int mSlotId;
    protected final Handler mHandler;
    protected boolean mIsOtherSimCellularAvailable = false;

    DefaultNetworkCallback(int slotId, @NonNull Handler target) {
        mSlotId = slotId;
        mHandler = target;
    }

    @Override
    public void onLost(Network network) {
        ImsLog.i(mSlotId, "DefaultNetworkCallback::onLost=" + network);
        if (mIsOtherSimCellularAvailable) {
            mIsOtherSimCellularAvailable = false;
            Message.obtain(mHandler, Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED,
                    mIsOtherSimCellularAvailable).sendToTarget();
        }
    }

    @Override
    public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
        ImsLog.i(mSlotId, "DefaultNetworkCallback::onCapabilitiesChanged=" + network
                + ", capabilities=" + networkCapabilities);

        boolean isAvailable = isOtherSimCellularAvailable(networkCapabilities);

        if (mIsOtherSimCellularAvailable != isAvailable) {
            mIsOtherSimCellularAvailable = isAvailable;
            Message.obtain(mHandler, Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED,
                    mIsOtherSimCellularAvailable).sendToTarget();
        }
    }

    protected boolean isOtherSimCellularAvailable(NetworkCapabilities networkCapabilities) {
        if (!networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR)) {
            return false;
        }

        NetworkSpecifier specifier = networkCapabilities.getNetworkSpecifier();
        if (!(specifier instanceof TelephonyNetworkSpecifier)) {
            return false;
        }

        int connectedSubId = ((TelephonyNetworkSpecifier) specifier).getSubscriptionId();
        if (connectedSubId == MSimUtils.getSubId(mSlotId)) {
            return false;
        }

        ImsLog.i(mSlotId, "Cellular network of other SIM is available");
        return true;
    }
}
