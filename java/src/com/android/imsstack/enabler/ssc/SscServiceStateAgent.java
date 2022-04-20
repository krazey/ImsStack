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

    public SscServiceState get(int slotId) {
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

    public void init(int slotId) {
        if (OperatorInfo.isSlotIdValid(slotId) != true) {
            ImsLog.w("Invalid SlotId(" + slotId + ")");
            return;
        }

        setSscServiceState(slotId, new SscServiceState());
    }

    public void deInit(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState != null) {
            serviceState.deInit();
        }
    }

    @VisibleForTesting
    public void setSscServiceState(int slotId, SscServiceState sscServiceState) {
        deInit(slotId);
        sscServiceState.init(slotId);
        mSscServiceState.put(slotId, sscServiceState);
    }

    public boolean isUtAvailable(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("isUtAvailable()");
            return false;
        }

        return serviceState.isUtAvailable();
    }

    // This method called by Transaction
    public void setErrorResponseCode(int slotId, int responseCode) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setErrorResponseCode()");
            return;
        }

        serviceState.setErrorResponseCode(responseCode);
    }

    // This method called by HTTPConnection
    public void setDNSQueryFailed(int slotId, boolean input) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setDNSQueryFailed()");
            return;
        }

        serviceState.setDNSQueryFailed(input);
    }

    public void setGBARequestFailed(int slotId, boolean input) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setGBARequestFailed()");
            return;
        }

        serviceState.setGBARequestFailed(input);
    }

    public void setPDNConnectionTimerExpired(int slotId, boolean input) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setPDNConnectionTimerExpired()");
            return;
        }

        serviceState.setPDNConnectionTimerExpired(input);
    }

    public void setSocketConnectionExpired(int slotId, boolean input) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setSocketConnectionExpired()");
            return;
        }

        serviceState.setSocketConnectionExpired(input);
    }

    public void setIsInitialQueryDone(int slotId, boolean input) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setIsInitialQueryDone()");
            return;
        }

        serviceState.setIsInitialQueryDone(input);
    }

    public void setAllSRVAddrTried(int slotId, boolean input) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("setAllSRVAddrTried()");
            return;
        }

        serviceState.setAllSRVAddrTried(input);
    }

    public boolean getPdnConnectionFailed(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getPdnConnectionFailed()");
            return true;
        }

        return serviceState.getPdnConnectionFailed();
    }

    public boolean getDNSQueryFailed(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getDNSQueryFailed()");
            return true;
        }

        return serviceState.getDNSQueryFailed();
    }

    public boolean getGBARequestFailed(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getGBARequestFailed()");
            return true;
        }

        return serviceState.getGBARequestFailed();
    }

    public boolean getPDNConnectionTimerExpired(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getPDNConnectionTimerExpired()");
            return true;
        }

        return serviceState.getPDNConnectionTimerExpired();
    }

    public boolean getAllSRVAddrTried(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getAllSRVAddrTried()");
            return false;
        }

        return serviceState.getAllSRVAddrTried();
    }

    public boolean getSocketConnectionExpired(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getSocketConnectionExpired()");
            return true;
        }

        return serviceState.getSocketConnectionExpired();
    }

    public boolean getIsInitialQueryDone(int slotId) {
        SscServiceState serviceState = get(slotId);
        if (serviceState == null) {
            ImsLog.i("getIsInitialQueryDone()");
            return false;
        }

        return serviceState.getIsInitialQueryDone();
    }
};
