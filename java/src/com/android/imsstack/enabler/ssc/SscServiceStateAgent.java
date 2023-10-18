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

import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;

import android.annotation.NonNull;
import android.os.Looper;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;
import java.util.List;

public class SscServiceStateAgent {
    private static final SscServiceStateAgent sSscServiceStateAgent = new SscServiceStateAgent();
    private final HashMap<Integer, SscServiceState> mSscServiceState = new HashMap<>();

    protected static SscServiceStateAgent getInstance() {
        return sSscServiceStateAgent;
    }

    protected void init(int slotId, Looper looper) {
        setSscServiceState(slotId, new SscServiceState(slotId, looper));
    }

    protected void deInit(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState != null) {
            serviceState.deInit();
            removeSscServiceState(slotId);
        }
    }

    @VisibleForTesting
    protected SscServiceState getSscServiceState(int slotId) {
        if (!mSscServiceState.containsKey(slotId)) {
            return null;
        }

        return mSscServiceState.get(slotId);
    }

    @VisibleForTesting
    public void setSscServiceState(int slotId, @NonNull SscServiceState sscServiceState) {
        deInit(slotId);
        sscServiceState.init();
        mSscServiceState.put(slotId, sscServiceState);
    }

    @VisibleForTesting
    void removeSscServiceState(int slotId) {
        mSscServiceState.remove(slotId);
    }

    protected boolean isUtAvailable(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return false;
        }

        return serviceState.isUtAvailable();
    }

    protected void changeCapabilities(int slotId, List<CapabilityPair> enabledCaps,
            List<CapabilityPair> disabledCaps) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.changeCapabilities(enabledCaps, disabledCaps);
    }

    protected void setErrorResponseCode(int slotId, int responseCode) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.setErrorResponseCode(responseCode);
    }

    protected void setPdnConnectionFailed(int slotId, int smCause) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.setPdnConnectionFailed(smCause);
    }

    protected void setDnsQueryFailed(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.setDnsQueryFailed(input);
    }

    protected void setGbaRequestFailed(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.setGbaRequestFailed(input);
    }

    protected void setPdnConnectionTimeout(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.setPdnConnectionTimeout(input);
    }

    protected void setSocketConnectionExpired(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.e(slotId, "serviceState is null");
            return;
        }

        serviceState.setSocketConnectionExpired(input);
    }
}