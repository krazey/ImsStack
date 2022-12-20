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

import android.os.Looper;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

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
            ImsLog.w("SscServiceState for#" + slotId + " is not set...");
            return null;
        }

        return mSscServiceState.get(slotId);
    }

    @VisibleForTesting
    public void setSscServiceState(int slotId, SscServiceState sscServiceState) {
        deInit(slotId);
        sscServiceState.init();
        mSscServiceState.put(slotId, sscServiceState);
    }

    private void removeSscServiceState(int slotId) {
        mSscServiceState.remove(slotId);
    }

    protected boolean isUtAvailable(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("isUtAvailable()");
            return false;
        }

        return serviceState.isUtAvailable();
    }

    protected void setErrorResponseCode(int slotId, int responseCode) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setErrorResponseCode()");
            return;
        }

        serviceState.setErrorResponseCode(responseCode);
    }

    protected void setPdnConnectionFailed(int slotId, int smCause) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setPdnConnectionFailed()");
            return;
        }

        serviceState.setPdnConnectionFailed(smCause);
    }

    protected void setDnsQueryFailed(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setDnsQueryFailed()");
            return;
        }

        serviceState.setDnsQueryFailed(input);
    }

    protected void setGbaRequestFailed(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setGbaRequestFailed()");
            return;
        }

        serviceState.setGbaRequestFailed(input);
    }

    protected void setPdnConnectionTimeout(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setPdnConnectionTimeout()");
            return;
        }

        serviceState.setPdnConnectionTimeout(input);
    }

    protected void setSocketConnectionExpired(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setSocketConnectionExpired()");
            return;
        }

        serviceState.setSocketConnectionExpired(input);
    }

    protected void setAllSrvAddrTried(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setAllSrvAddrTried()");
            return;
        }

        serviceState.setAllSrvAddrTried(input);
    }

    protected boolean getDnsQueryFailed(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getDnsQueryFailed()");
            return true;
        }

        return serviceState.getDnsQueryFailed();
    }

    protected boolean getGbaRequestFailed(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getGbaRequestFailed()");
            return true;
        }

        return serviceState.getGbaRequestFailed();
    }

    protected boolean getPdnConnectionTimeout(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getPdnConnectionTimeout()");
            return true;
        }

        return serviceState.getPdnConnectionTimeout();
    }

    protected boolean getSocketConnectionExpired(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getSocketConnectionExpired()");
            return true;
        }

        return serviceState.getSocketConnectionExpired();
    }

    protected boolean getAllSrvAddrTried(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getAllSrvAddrTried()");
            return false;
        }

        return serviceState.getAllSrvAddrTried();
    }

    protected boolean getPdnConnectionFailed(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getPdnConnectionFailed()");
            return true;
        }

        return serviceState.getPdnConnectionFailed();
    }
}