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

import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.util.ImsLog;

import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

public class SscServiceStateAgent {
    private static SscServiceStateAgent sSscServiceStateAgent = new SscServiceStateAgent();
    private HashMap<Integer, SscServiceState> mSscServiceState = new HashMap<>();

    public static SscServiceStateAgent getInstance() {
        return sSscServiceStateAgent;
    }

    public void init(int slotId) {
        if (OperatorInfo.isSlotIdValid(slotId) != true) {
            ImsLog.w("Invalid SlotId(" + slotId + ")");
            return;
        }

        setSscServiceState(slotId, new SscServiceState());
    }

    public void deInit(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState != null) {
            serviceState.deInit();
            removeSscServiceState(slotId);
        }
    }

    @VisibleForTesting
    public SscServiceState getSscServiceState(int slotId) {
        if (OperatorInfo.isSlotIdValid(slotId) != true) {
            ImsLog.w("Invalid SlotId(" + slotId + ")");
            return null;
        }

        if (mSscServiceState.containsKey(slotId) != true) {
            ImsLog.w("SscServiceState for#" + slotId + " is not set...");
            return null;
        }

        return mSscServiceState.get(slotId);
    }

    @VisibleForTesting
    public void setSscServiceState(int slotId, SscServiceState sscServiceState) {
        deInit(slotId);
        sscServiceState.init(slotId);
        mSscServiceState.put(slotId, sscServiceState);
    }

    private void removeSscServiceState(int slotId) {
        mSscServiceState.remove(slotId);
    }

    public boolean isUtAvailable(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("isUtAvailable()");
            return false;
        }

        return serviceState.isUtAvailable();
    }

    // This method called by Transaction
    public void setErrorResponseCode(int slotId, int responseCode) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setErrorResponseCode()");
            return;
        }

        serviceState.setErrorResponseCode(responseCode);
    }

    // This method called by HTTPConnection
    public void setDnsQueryFailed(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setDnsQueryFailed()");
            return;
        }

        serviceState.setDnsQueryFailed(input);
    }

    public void setGbaRequestFailed(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setGbaRequestFailed()");
            return;
        }

        serviceState.setGbaRequestFailed(input);
    }

    public void setPdnConnectionTimerExpired(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setPdnConnectionTimerExpired()");
            return;
        }

        serviceState.setPdnConnectionTimerExpired(input);
    }

    public void setSocketConnectionExpired(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setSocketConnectionExpired()");
            return;
        }

        serviceState.setSocketConnectionExpired(input);
    }

    public void setAllSrvAddrTried(int slotId, boolean input) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("setAllSrvAddrTried()");
            return;
        }

        serviceState.setAllSrvAddrTried(input);
    }

    public boolean getDnsQueryFailed(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getDnsQueryFailed()");
            return true;
        }

        return serviceState.getDnsQueryFailed();
    }

    public boolean getGbaRequestFailed(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getGbaRequestFailed()");
            return true;
        }

        return serviceState.getGbaRequestFailed();
    }

    public boolean getPdnConnectionTimerExpired(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getPdnConnectionTimerExpired()");
            return true;
        }

        return serviceState.getPdnConnectionTimerExpired();
    }

    public boolean getSocketConnectionExpired(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getSocketConnectionExpired()");
            return true;
        }

        return serviceState.getSocketConnectionExpired();
    }

    public boolean getAllSrvAddrTried(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getAllSrvAddrTried()");
            return false;
        }

        return serviceState.getAllSrvAddrTried();
    }

    public boolean getPdnConnectionFailed(int slotId) {
        SscServiceState serviceState = getSscServiceState(slotId);
        if (serviceState == null) {
            ImsLog.i("getPdnConnectionFailed()");
            return true;
        }

        return serviceState.getPdnConnectionFailed();
    }
}