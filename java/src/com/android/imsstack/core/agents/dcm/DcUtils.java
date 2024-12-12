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
package com.android.imsstack.core.agents.dcm;

import android.annotation.NonNull;
import android.content.Context;
import android.telephony.AccessNetworkConstants;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.List;

/**
 * This implements the utility interfaces that are related to the data network.
 */
public class DcUtils implements IDcUtils {
    public static final int ANI_ITEM_SIZE = 5;
    public static final int ANI_INDEX_MCC = 0;
    public static final int ANI_INDEX_MNC = 1;
    public static final int ANI_INDEX_CELL_ID = 2;
    public static final int ANI_INDEX_TAC_OR_LAC = 3;
    public static final int ANI_INDEX_MODE = 4;

    public static final String MODE_FDD = "FDD";
    public static final String MODE_TDD = "TDD";

    // TODO: do we need to support eHRPD?
    /*
    public static final int EHRPD_ANI_ITEM_SIZE = 2;
    public static final int EHRPD_ANI_INDEX_SECTOR_ID = 0;
    public static final int EHRPD_ANI_INDEX_SUBNET_LEN = 1;
    */
    private final SparseArray<List<String>> mRecentAccessNetworkInfos = new SparseArray<>(2);
    private int mSlotId = 0;

    public DcUtils(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    /**
     * Returns the access network information that the device is currently attached.
     *
     * @param defaultNetworkType A default network type that requests the access network information
     * @return A string array that contains the access network information.
     */
    @Override
    public IDcUtils.AccessNetworkInfo getAccessNetworkInfo(int defaultNetworkType) {
        // GET_ACCESS_NETWORK_INFO
        //     eHRPD: Sector ID/Subnet Length/Carrier ID(optional)
        //     GERAN/UTRAN: MCC/MNC/Cell Identity/LAC
        //     EUTRAN/NGRAN: MCC/MNC/Cell Identity/TAC/Duplex Mode
        String[] ani = null;
        ServiceState ss = getServiceState();
        NetworkRegistrationInfo nri = getNetworkRegistrationInfo(ss);
        int networkType = (nri != null)
                ? nri.getAccessNetworkTechnology()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;

        if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            // When no UICC is inserted, network type is not correctly reported from modem.
            // the only case we care is making 911 call with no UICC.
            // if 911 call is over IMS, then UE must be in lte network.
            networkType = (defaultNetworkType > 0)
                    ? defaultNetworkType : TelephonyManager.NETWORK_TYPE_LTE;
        }

        if (nri == null) {
            ImsLog.w(mSlotId, "NetworkRegistrationInfo is null");
            ani = getAccessNetworkInfoFromCache(networkType);
        } else {
            switch (networkType) {
                case TelephonyManager.NETWORK_TYPE_LTE:
                    ani = getAccessNetworkInfoForLte(nri, ss.getDuplexMode());
                    break;
                case TelephonyManager.NETWORK_TYPE_NR:
                    ani = getAccessNetworkInfoForNr(nri);
                    break;
                case TelephonyManager.NETWORK_TYPE_UMTS: // FALL-THROUGH
                case TelephonyManager.NETWORK_TYPE_HSDPA: // FALL-THROUGH
                case TelephonyManager.NETWORK_TYPE_HSUPA: // FALL-THROUGH
                case TelephonyManager.NETWORK_TYPE_HSPA: // FALL-THROUGH
                case TelephonyManager.NETWORK_TYPE_HSPAP:
                    ani = getAccessNetworkInfoForWcdma(nri);
                    break;
                case TelephonyManager.NETWORK_TYPE_GPRS: // FALL-THROUGH
                case TelephonyManager.NETWORK_TYPE_EDGE:
                    ani = getAccessNetworkInfoForGsm(nri);
                    break;
                case TelephonyManager.NETWORK_TYPE_EHRPD:
                    // Not implemented at this time.
                    break;
                default:
                    break;
            }
        }

        ImsLog.i(mSlotId, "ANI: networkType=" + networkType
                + ", info=" + ImsLog.hiddenString(ani)
                + ", defaultNetworkType=" + defaultNetworkType);

        return new IDcUtils.AccessNetworkInfo(networkType, ani);
    }

    /**
     * Checks whether the mobile data setting is enabled or not.
     *
     * @return {@code true} if the mobile data setting is enabled, {@code false} otherwise.
     */
    @Override
    public boolean isMobileDataEnabled() {
        TelephonyManagerProxy tmp =
                AppContext.getTelephonyManagerProxy(MSimUtils.getSubId(mSlotId));
        return tmp.isDataEnabled();
    }

    @Override
    public void updateAllCellInfoForcinglyOnLimitedServiceState() {
        ImsLog.w(mSlotId, "Not implemented");
    }

    @VisibleForTesting
    protected ServiceState getServiceState() {
        final TelephonyManagerProxy tmp =
                AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
        return tmp.getServiceState(mSlotId);
    }

    @VisibleForTesting
    protected String[] getAccessNetworkInfoFromCache(int networkType) {
        ImsLog.d(mSlotId, "ANI: from cache");
        List<String> ani = mRecentAccessNetworkInfos.get(networkType);
        return (ani != null) ? ani.toArray(new String[0]) : null;
    }

    @VisibleForTesting
    protected void storeAccessNetworkInfoToCache(int networkType, @NonNull String[] ani) {
        mRecentAccessNetworkInfos.put(networkType, Arrays.asList(ani));
    }

    @VisibleForTesting
    protected String[] getAccessNetworkInfoForLte(@NonNull NetworkRegistrationInfo nri,
            int duplexMode) {
        String[] ani = null;
        CellIdentityLte ci = getCellIdentity(nri, CellIdentityLte.class);

        if (ci == null) {
            ani = getAccessNetworkInfoFromCache(TelephonyManager.NETWORK_TYPE_LTE);
        } else {
            ani = getAccessNetworkInfoFromCellIdentityLte(ci, duplexMode);

            if (ani == null) {
                ani = getAccessNetworkInfoFromCache(TelephonyManager.NETWORK_TYPE_LTE);
            } else {
                // Store most recent cell information
                storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_LTE, ani);
            }
        }

        return ani;
    }

    @VisibleForTesting
    protected String[] getAccessNetworkInfoForNr(@NonNull NetworkRegistrationInfo nri) {
        String[] ani = null;
        CellIdentityNr ci = getCellIdentity(nri, CellIdentityNr.class);

        if (ci == null) {
            ani = getAccessNetworkInfoFromCache(TelephonyManager.NETWORK_TYPE_NR);
        } else {
            ani = getAccessNetworkInfoFromCellIdentityNr(ci, getDuplexModeForNr(ci, mSlotId));

            if (ani == null) {
                ani = getAccessNetworkInfoFromCache(TelephonyManager.NETWORK_TYPE_NR);
            } else {
                // Store most recent cell information
                storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_NR, ani);
            }
        }

        return ani;
    }

    @VisibleForTesting
    protected String[] getAccessNetworkInfoForWcdma(@NonNull NetworkRegistrationInfo nri) {
        CellIdentityWcdma ci = getCellIdentity(nri, CellIdentityWcdma.class);

        if (ci != null) {
            return getAccessNetworkInfoFromCellIdentityWcdma(ci);
        }

        return null;
    }

    @VisibleForTesting
    protected String[] getAccessNetworkInfoForGsm(@NonNull NetworkRegistrationInfo nri) {
        CellIdentityGsm ci = getCellIdentity(nri, CellIdentityGsm.class);

        if (ci != null) {
            return getAccessNetworkInfoFromCellIdentityGsm(ci);
        }

        return null;
    }

    private static NetworkRegistrationInfo getNetworkRegistrationInfo(ServiceState ss) {
        if (ss != null) {
            NetworkRegistrationInfo nri = ss.getNetworkRegistrationInfo(
                    NetworkRegistrationInfo.DOMAIN_PS,
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            return nri;
        }

        return null;
    }

    /**
     * Returns the duplex mode of NGRAN.
     *
     * @param ci The cell-identity of NGRAN.
     * @param slotId The slot-id.
     * @return The duplex mode of NGRAN.
     *         {@link ServiceState#DUPLEX_MODE_FDD} or
     *         {@link ServiceState#DUPLEX_MODE_TDD}.
     */
    public static int getDuplexModeForNr(@NonNull CellIdentityNr ci, int slotId) {
        String nrMode = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_CONFIG_NR_DUPLEX_MODE, slotId);

        if (!TextUtils.isEmpty(nrMode)) {
            ImsLog.d(slotId, "predefined nr mode: " + nrMode);

            if (nrMode.equals(MODE_TDD)) {
                return ServiceState.DUPLEX_MODE_TDD;
            } else if (nrMode.equals(MODE_FDD)) {
                return ServiceState.DUPLEX_MODE_FDD;
            }
        }

        // TODO: Is the arfcn for downlink? Need to check if it could be uplink as well
        int arfcn = ci.getNrarfcn();
        if (arfcn == CellInfo.UNAVAILABLE) {
            return ServiceState.DUPLEX_MODE_TDD;
        }

        int[] bands = ci.getBands();
        for (int band : bands) {
            for (NgranBandArfcnFrequency ngranBand : NgranBandArfcnFrequency.values()) {
                if (band == ngranBand.getBand()) {
                    if (isInNrarfcnRange(ngranBand, arfcn)) {
                        return ngranBand.getDuplexMode();
                    }
                }
            }
        }

        return ServiceState.DUPLEX_MODE_TDD;
    }

    private static boolean isInNrarfcnRange(@NonNull NgranBandArfcnFrequency ngranBand, int arfcn) {
        return arfcn >= ngranBand.getArfcnRangeFirst() && arfcn <= ngranBand.getArfcnRangeLast();
    }

    @SuppressWarnings("unchecked")
    private static <T> T getCellIdentity(@NonNull NetworkRegistrationInfo nri, Class<T> clazz) {
        try {
            return (clazz != null) ? clazz.cast(nri.getCellIdentity()) : (T) nri.getCellIdentity();
        } catch (ClassCastException e) {
            ImsLog.d("getCellIdentity: " + e + " class: " + clazz.getName());
            return null;
        }
    }

    private static String[] getAccessNetworkInfoFromCellIdentityLte(@NonNull CellIdentityLte ci,
            int duplexMode) {
        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        int cellId = ci.getCi();
        int tac = ci.getTac();

        if (mcc == null || mnc == null
                || cellId == CellInfo.UNAVAILABLE || tac == CellInfo.UNAVAILABLE) {
            ImsLog.w("CellIdentityLte: contains the invalid values - "
                    + "mcc=" + mcc + ", mnc=" + mnc + ", ci=" + pii(cellId) + ", tac=" + pii(tac));
            return null;
        }

        // MCC / MNC / Cell identity / TAC / Duplex Mode
        String[] ani = new String[ANI_ITEM_SIZE];

        ani[ANI_INDEX_MCC] = mcc;
        ani[ANI_INDEX_MNC] = mnc;
        ani[ANI_INDEX_CELL_ID] = Integer.toHexString(cellId);
        ani[ANI_INDEX_TAC_OR_LAC] = Integer.toHexString(tac);
        ani[ANI_INDEX_MODE] = (duplexMode == ServiceState.DUPLEX_MODE_TDD) ? MODE_TDD : MODE_FDD;

        return ani;
    }

    private static String[] getAccessNetworkInfoFromCellIdentityNr(@NonNull CellIdentityNr ci,
            int duplexMode) {
        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        long cellId = ci.getNci();
        int tac = ci.getTac();

        if (mcc == null || mnc == null
                || cellId == CellInfo.UNAVAILABLE_LONG || tac == CellInfo.UNAVAILABLE) {
            ImsLog.w("CellIdentityNr: contains the invalid values - "
                    + "mcc=" + mcc + ", mnc=" + mnc + ", ci=" + pii(cellId) + ", tac=" + pii(tac));
            return null;
        }

        // MCC / MNC / Cell identity / TAC / Duplex Mode
        String[] ani = new String[ANI_ITEM_SIZE];

        ani[ANI_INDEX_MCC] = mcc;
        ani[ANI_INDEX_MNC] = mnc;
        ani[ANI_INDEX_CELL_ID] = Long.toHexString(cellId);
        ani[ANI_INDEX_TAC_OR_LAC] = Integer.toHexString(tac);
        ani[ANI_INDEX_MODE] = (duplexMode == ServiceState.DUPLEX_MODE_FDD) ? MODE_FDD : MODE_TDD;

        return ani;
    }

    private static String[] getAccessNetworkInfoFromCellIdentityWcdma(
            @NonNull CellIdentityWcdma ci) {
        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        int cellId = ci.getCid();
        int lac = ci.getLac();

        if (mcc == null || mnc == null
                || cellId == CellInfo.UNAVAILABLE || lac == CellInfo.UNAVAILABLE) {
            ImsLog.w("CellIdentityWcdma: contains the invalid values - "
                    + "mcc=" + mcc + ", mnc=" + mnc + ", ci=" + pii(cellId) + ", lac=" + pii(lac));
            return null;
        }

        // MCC / MNC / Cell identity / LAC
        String[] ani = new String[ANI_ITEM_SIZE];

        ani[ANI_INDEX_MCC] = mcc;
        ani[ANI_INDEX_MNC] = mnc;
        ani[ANI_INDEX_CELL_ID] = Integer.toHexString(cellId);
        ani[ANI_INDEX_TAC_OR_LAC] = Integer.toHexString(lac);
        ani[ANI_INDEX_MODE] = "";

        return ani;
    }

    private static String[] getAccessNetworkInfoFromCellIdentityGsm(@NonNull CellIdentityGsm ci) {
        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        int cellId = ci.getCid();
        int lac = ci.getLac();

        if (mcc == null || mnc == null
                || cellId == CellInfo.UNAVAILABLE || lac == CellInfo.UNAVAILABLE) {
            ImsLog.w("CellIdentityGsm: contains the invalid values - "
                    + "mcc=" + mcc + ", mnc=" + mnc + ", ci=" + pii(cellId) + ", lac=" + pii(lac));
            return null;
        }

        // MCC / MNC / Cell identity / LAC
        String[] ani = new String[ANI_ITEM_SIZE];

        ani[ANI_INDEX_MCC] = mcc;
        ani[ANI_INDEX_MNC] = mnc;
        ani[ANI_INDEX_CELL_ID] = Integer.toHexString(cellId);
        ani[ANI_INDEX_TAC_OR_LAC] = Integer.toHexString(lac);
        ani[ANI_INDEX_MODE] = "";

        return ani;
    }

    private static String pii(int value) {
        return (value == 0 || value == CellInfo.UNAVAILABLE)
                ? String.valueOf(value) : Log.pii(String.valueOf(value));
    }

    private static String pii(long value) {
        return (value == 0 || value == CellInfo.UNAVAILABLE_LONG)
                ? String.valueOf(value) : Log.pii(String.valueOf(value));
    }
}
