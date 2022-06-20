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
import android.telephony.CellSignalStrength;
import android.telephony.CellSignalStrengthLte;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.util.SparseArray;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.ITelephonyState;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SettingsUtils;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
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
        /**
         * GET_ACCESS_NETWORK_INFO
         * eHRPD: Sector ID/Subnet Length/Carrier ID(optional)
         * GERAN/UTRAN: MCC/MNC/Cell Identity/LAC
         * EUTRAN/NGRAN: MCC/MNC/Cell Identity/TAC/Duplex Mode
         */
        String[] ani = null;
        ServiceState ss = getServiceState();
        NetworkRegistrationInfo nri = getNetworkRegistrationInfo(ss);
        int networkType = (nri != null)
                ? nri.getAccessNetworkTechnology()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;

        if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            // when no uicc is inserted, network type is not correctly reported from modem.
            // the only case we care is making 911 call with no uicc.
            // if 911 call is over ims, then UE must be in lte network.
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
                    ani = getAccessNetworkInfoForNr(nri, ss.getDuplexMode());
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
    * isMobileDataEnabled
    *
    * @param
    * @return
    *
    *         false: Disable
    *         true: Enable
    */
    @Override
    public boolean isMobileDataEnabled() {
        return SettingsUtils.isMobileDataEnabled(AppContext.get().getContentResolver());
    }

    @Override
    public int getLteRsrpStrength() {
        final int defaultRsrp = 0;

        ITelephonyState ts = (ITelephonyState) AgentFactory.getAgent(
                AgentFactory.TELEPHONY_STATE, mSlotId);

        if (ts == null) {
            return defaultRsrp;
        }

        int rat = ts.getNetworkType();

        if (rat == TelephonyManager.NETWORK_TYPE_LTE) {
            TelephonyManager tm = null;

            if (MSimUtils.isMultiSimEnabled()) {
                tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
            } else {
                tm = AppContext.getTelephonyManager();
            }

            SignalStrength ss = tm.getSignalStrength();
            if (ss != null) {
                List<CellSignalStrength> cssList = ss.getCellSignalStrengths();
                for (CellSignalStrength css : cssList) {
                    if (css instanceof CellSignalStrengthLte) {
                        CellSignalStrengthLte lteStrength = (CellSignalStrengthLte) css;
                        return lteStrength.getRsrp();
                    }
                }
            }
        }

        return defaultRsrp;
    }

    @Override
    public boolean sendPingToHostAddress(int apnType, String hostAddress) {
        ImsLog.d(mSlotId, "");

        String pingCmd;
        String tempPing4Cmd = "ping -c 5 -i 0.2 -w 1 ";
        String tempping6Cmd = "ping6 -c 5 -i 0.2 -w 1 ";

        try {
            InetAddress address = InetAddress.getByName(hostAddress);

            if (address instanceof Inet6Address) {
                ImsLog.d(mSlotId, "use ping6 command");
                pingCmd = tempping6Cmd;
            } else if (address instanceof Inet4Address) {
                ImsLog.d(mSlotId, "use ping command");
                pingCmd = tempPing4Cmd;
            } else {
                ImsLog.e(mSlotId, "host address is invalid.");
                return true;
            }
        } catch (UnknownHostException e) {
            ImsLog.e(mSlotId, "UnknownHostException :: " + e);
            return true;
        }

        IDCApn dcapn = (IDCApn) DCFactory.getDC(DCFactory.APN, mSlotId);
        if (dcapn == null) {
            return true;
        }

        String iface = dcapn.getIfaceName(apnType);
        String pingIface = "-I " + iface + " ";

        ImsLog.d(mSlotId, "apnType = " + apnType + " interfaceName = " + iface
                + " pingIface = " + pingIface);

        try {
            ArrayList<String> cmdResult = getPingCmdResult(pingCmd + pingIface + hostAddress);
            if (cmdResult == null) {
                return true;
            }

            for (String line : cmdResult) {
                ImsLog.d(mSlotId, "line : " + line);

                if (line != null && line.contains("bytes from " + hostAddress)) {
                    ImsLog.i(mSlotId, "ping response is successfully received from the server.");
                    return true;
                }

                if (line != null && line.contains("100% packet loss")) {
                    ImsLog.i(mSlotId, "ping to the server fail");
                    return false;
                }
            }
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception :: " + e);
        }

        return true;
    }

    @Override
    public void updateAllCellInfoForcinglyOnLimitedServiceState() {
        ImsLog.w(mSlotId, "Not implemented");
    }

    private ArrayList<String> getPingCmdResult(String cmd) {
        ArrayList<String> list = new ArrayList<String>();
        Runtime runtime = Runtime.getRuntime();
        Process process;

        InputStream inStream = null;
        InputStreamReader inStreamReader = null;
        BufferedReader buffReader = null;

        ImsLog.i(mSlotId, "getshelllog: " + cmd);

        String[] listCmd = null;
        listCmd = cmd.split(" ");

        try {
            process = runtime.exec(listCmd);

            if (process == null) {
                ImsLog.e(mSlotId, "Process is null.");
                return null;
            }

            inStream = process.getInputStream();

            if (inStream == null) {
                ImsLog.e(mSlotId, "InputStream is null.");
                return null;
            }

            inStreamReader = new InputStreamReader(inStream);

            if (inStreamReader == null) {
                ImsLog.e(mSlotId, "InputStreamReader is null.");
                return null;
            }

            buffReader = new BufferedReader(inStreamReader);

            if (buffReader == null) {
                ImsLog.e(mSlotId, "BufferedReader is null.");
                return null;
            }

            String line;

            while ((line = buffReader.readLine()) != null) {
                list.add(line);
            }
        } catch (Exception e) {
            ImsLog.i(mSlotId, "Error getting i/o stream. " + e);
        } finally {
            if (buffReader != null) {
                try {
                    buffReader.close();
                } catch (Exception e) {
                    ImsLog.e(mSlotId, "exception occrurred");
                }
            }

            if (inStreamReader != null) {
                try {
                    inStreamReader.close();
                } catch (Exception e) {
                    ImsLog.e(mSlotId, "exception occrurred");
                }
            }

            if (inStream != null) {
                try {
                    inStream.close();
                } catch (Exception e) {
                    ImsLog.e(mSlotId, "exception occrurred");
                }
            }
        }

        return list;
    }

    private ServiceState getServiceState() {
        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        } else {
            tm = AppContext.getTelephonyManager();
        }

        return (tm != null) ? tm.getServiceState() : null;
    }

    private String[] getAccessNetworkInfoFromCache(int networkType) {
        ImsLog.d(mSlotId, "ANI: from cache");
        List<String> ani = mRecentAccessNetworkInfos.get(networkType);
        return (ani != null) ? ani.toArray(new String[0]) : null;
    }

    private void storeAccessNetworkInfoToCache(int networkType, String[] ani) {
        if (ani == null) {
            mRecentAccessNetworkInfos.put(networkType, null);
        } else {
            mRecentAccessNetworkInfos.put(networkType, Arrays.asList(ani));
        }
    }

    private String[] getAccessNetworkInfoForLte(@NonNull NetworkRegistrationInfo nri,
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

    private String[] getAccessNetworkInfoForNr(@NonNull NetworkRegistrationInfo nri,
            int duplexMode) {
        String[] ani = null;
        CellIdentityNr ci = getCellIdentity(nri, CellIdentityNr.class);

        if (ci == null) {
            ani = getAccessNetworkInfoFromCache(TelephonyManager.NETWORK_TYPE_NR);
        } else {
            ani = getAccessNetworkInfoFromCellIdentityNr(ci, duplexMode);

            if (ani == null) {
                ani = getAccessNetworkInfoFromCache(TelephonyManager.NETWORK_TYPE_NR);
            } else {
                // Store most recent cell information
                storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_NR, ani);
            }
        }

        return ani;
    }

    private String[] getAccessNetworkInfoForWcdma(@NonNull NetworkRegistrationInfo nri) {
        CellIdentityWcdma ci = getCellIdentity(nri, CellIdentityWcdma.class);

        if (ci != null) {
            return getAccessNetworkInfoFromCellIdentityWcdma(ci);
        }

        return null;
    }

    private String[] getAccessNetworkInfoForGsm(@NonNull NetworkRegistrationInfo nri) {
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

    private static <T> T getCellIdentity(@NonNull NetworkRegistrationInfo nri, Class<T> clazz) {
        try {
            return (T) nri.getCellIdentity();
        } catch (ClassCastException e) {
            ImsLog.d("getCellIdentity: " + e);
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
