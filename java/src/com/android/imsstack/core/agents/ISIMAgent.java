package com.android.imsstack.core.agents;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.agentif.IISIM;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.MSimUtils;

public class ISIMAgent implements IISIM {
    private int mSlotId;

    public ISIMAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    @Override
    public String getImpi() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getIsimImpi() : null;
    }

    @Override
    public String getDomain() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getIsimDomain() : null;
    }

    @Override
    public String[] getImpu() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getIsimImpu() : null;
    }

    @Override
    public String getIst() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getIsimIst() : null;
    }

    @Override
    public String[] getPcscf() {
        return null;
    }

    @Override
    public String getChallengeResponse(String nonce) {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getIccAuthentication(
                TelephonyManager.APPTYPE_ISIM,
                TelephonyManager.AUTHTYPE_EAP_AKA,
                nonce) : null;
    }

    @Override
    public boolean isGbaSupported() {
        String ist = getIst();

        if (ist != null && ist.length() > 1
                && (0x02 & (byte)ist.charAt(1)) != 0) {
            return true;
        }

        return false;
    }

    private TelephonyManager getTelephonyManager() {
        int subId = MSimUtils.getSubId(mSlotId);
        return AppContext.getTelephonyManager(subId);
    }
}
