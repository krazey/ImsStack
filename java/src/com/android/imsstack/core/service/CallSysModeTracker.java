package com.android.imsstack.core.service;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.util.ImsLog;

public class CallSysModeTracker {
    public static final int RIL_SYS_MODE_UNKNOWN = -1;
    public static final int RIL_SYS_MODE_WIFI = 0x06;
    public static final int RIL_SYS_MODE_LTE = 0x08;
    public static final int RIL_SYS_MODE_NR5G = 0x0C;
    public static final int RIL_SYS_MODE_3G = 0x05;

    public static final int IPCAN_INVALID = -1;

    public static final int APN_TYPE_IMS = EApnType.IMS.getType();
    public static final int APN_TYPE_EMERGENCY = EApnType.EMERGENCY.getType();

    private final int mSlotId;
    private ImsApnStateListener mApnStateListener = null;
    private IListener mListener = null;

    public CallSysModeTracker(int slotId) {
        mSlotId = slotId;
        setApnStateListener();
    }

    public void setListener(IListener listener) {
        mListener = listener;
    }

    public void clear() {
        clearApnStateListener();
    }

    public int getSysMode(int apnType) {

        int ipCanCategory = getIpcanCategory(apnType);
        ImsLog.d(mSlotId, "getSysMode :: apnType=" + apnType + ", ipcanCategory=" + ipCanCategory);

        if (ipCanCategory == IPCAN_INVALID) {
            return RIL_SYS_MODE_UNKNOWN;
        } else if (ipCanCategory == IApn.IPCAN_CATEGORY_WLAN) {
            return RIL_SYS_MODE_WIFI;
        }

        IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dcnw == null) {
            return RIL_SYS_MODE_LTE;
        }

        if (dcnw.is5G()) {
            return RIL_SYS_MODE_NR5G;
        } else if (dcnw.is3G()) {
            return RIL_SYS_MODE_3G;
        } else {
            return RIL_SYS_MODE_LTE;
        }
    }

    private int getIpcanCategory(int apnType) {

        IDcApn dcApn = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);

        if (dcApn != null && dcApn.isConnected(apnType)) {
            return dcApn.getIpcanCategory(apnType);
        }

        return IPCAN_INVALID;
    }

    private void clearApnStateListener() {
        if (mApnStateListener == null) {
            return;
        }

        IDcApn dcApn = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);

        IApn apnIms = (dcApn != null) ? dcApn.getApnControl(APN_TYPE_IMS) : null;
        if (apnIms != null) {
            apnIms.removeListener(mApnStateListener);
        }

        IApn apnEmergency = (dcApn != null) ? dcApn.getApnControl(APN_TYPE_EMERGENCY) : null;
        if (apnEmergency != null) {
            apnEmergency.removeListener(mApnStateListener);
        }

        mApnStateListener = null;
    }

    private void setApnStateListener() {
        IDcApn dcApn = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);

        IApn apnIms = (dcApn != null) ? dcApn.getApnControl(APN_TYPE_IMS) : null;
        if (apnIms != null) {
            mApnStateListener = new ImsApnStateListener();
            apnIms.addListener(mApnStateListener);
        }

        IApn apnEmergency
                = (dcApn != null) ? dcApn.getApnControl(APN_TYPE_EMERGENCY) : null;
        if (apnEmergency != null) {
            if (mApnStateListener == null) {
                mApnStateListener = new ImsApnStateListener();
            }
            apnEmergency.addListener(mApnStateListener);
        }
    }

    private class ImsApnStateListener extends ApnStateListener {
        @Override
        public void onIpcanCategoryChanged(int apnType, int ipcanCategory) {
            ImsLog.d(mSlotId, "onIpcanCategoryChanged :: apnType="
                    + apnType + ", ipcanCategory=" + ipcanCategory);

            if (mListener != null) {
                mListener.onIpCanChanged(apnType);
            }
        }
    }

    public interface IListener {

        public void onIpCanChanged(int apnType);
    }
}
