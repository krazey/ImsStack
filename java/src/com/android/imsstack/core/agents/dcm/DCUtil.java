package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellInfo;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoNr;
import android.telephony.CellSignalStrength;
import android.telephony.CellSignalStrengthLte;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.ISIMState;
import com.android.imsstack.core.agents.agentif.ITelephonyState;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.core.agents.dcmif.IDC;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDCUtil;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SettingsUtils;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.Executor;

public class DCUtil implements IDCUtil {
    // Constants--------------------------------------------------
    private static final int CELL_INFO_UPDATE_TIMEOUT_MILLS = 2000;

    private static final int LTE_PAN_INFO_ITEM_SIZE = 5;
    private static final int LTE_PAN_INFO_MCC_INDEX = 0;
    private static final int LTE_PAN_INFO_MNC_INDEX = 1;
    private static final int LTE_PAN_INFO_CELLID_INDEX = 2;
    private static final int LTE_PAN_INFO_TAC_INDEX = 3;
    private static final int LTE_PAN_INFO_MODE_INDEX = 4;

    private static final int EHRPD_PAN_INFO_ITEM_SIZE = 2;
    private static final int EHRPD_PAN_INFO_SECTOR_ID_INDEX = 0;
    private static final int EHRPD_PAN_INFO_SUBNET_LEN_INDEX = 1;

    // Variables--------------------------------------------------
    private Context mContext;
    private ArrayList<String> mRecentNrInfo = new ArrayList<String>();
    private ArrayList<String> mRecentLteInfo = new ArrayList<String>();
    private ImsCellInfoUpdater mCellInfoUpdater = null;
    private ImsCellInfoUpdater mEmergencyCellInfoUpdater = null;
    private int mSlotId = 0;

    // Public methods --------------------------------------------
    public DCUtil(int slotId) {
        mSlotId = slotId;

        mCellInfoUpdater = new ImsCellInfoUpdater();
    }

    @Override
    public void init(Context context) {
        mContext = context;
    }

    @Override
    public void cleanup() {
        mEmergencyCellInfoUpdater = null;
    }

     /**
     * getAccessNetworkInfo
     *
     * @param
     *         defaultNetworkType - default network type when it's unknown
     * @return
     *         PANI - P-Access Network Info String array.
     */
    @Override
    public IDCUtil.AccessNetworkInfo getAccessNetworkInfo(int defaultNetworkType) {
        /**
         * Get Radio Access Network Info
         * eHRPD - Sector ID/Subnet Length/Carrier ID(optional)
         * LTE - MCC/MNC/TAC/Cell Identity
         * NETWORK_TYPE_EHRPD = 13
         * NETWORK_TYPE_LTE = 14;
         */
        String[] panInfo = null;
        ITelephonyState ts = (ITelephonyState)AgentFactory.getAgent(
                AgentFactory.TELEPHONY_STATE, mSlotId);
        int rat = (ts != null) ? ts.getNetworkType() : TelephonyManager.NETWORK_TYPE_UNKNOWN;

        if (rat == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            // when no uicc is inserted, network type is not correctly reported from modem.
            // the only case we care is making 911 call with no uicc.
            // if 911 call is over ims, then UE must be in lte network.
            rat = (defaultNetworkType > 0) ?
                    defaultNetworkType : TelephonyManager.NETWORK_TYPE_LTE;
        }

        if (rat == TelephonyManager.NETWORK_TYPE_LTE) {
            panInfo = getLtePani(rat);
        } else if (rat == TelephonyManager.NETWORK_TYPE_NR) {
            panInfo = getNrPani(rat);
        } else if (rat == TelephonyManager.NETWORK_TYPE_EHRPD) {
            panInfo = getEhrpdPani(rat);
        } else if ((rat == TelephonyManager.NETWORK_TYPE_UMTS) ||
                (rat == TelephonyManager.NETWORK_TYPE_HSDPA) ||
                (rat == TelephonyManager.NETWORK_TYPE_HSUPA) ||
                (rat == TelephonyManager.NETWORK_TYPE_HSPA) ||
                (rat == TelephonyManager.NETWORK_TYPE_HSPAP) ||
                (rat == TelephonyManager.NETWORK_TYPE_GPRS) ||
                (rat == TelephonyManager.NETWORK_TYPE_EDGE)) {
            panInfo = getOthersPani(rat);
        } else {
            ImsLog.w(mSlotId, "RAT is not valid; rat=" + rat);
            return new IDCUtil.AccessNetworkInfo(rat, panInfo);
        }

        if (panInfo == null) {
            ImsLog.w(mSlotId, "PAN Info is null; rat=" + rat);
            return new IDCUtil.AccessNetworkInfo(rat, panInfo);
        }

        ImsLog.i(mSlotId, "PANInfo :: rat=" + rat + ", info=" + ImsLog.hiddenString(panInfo) +
                ", defaultNetworkType=" + defaultNetworkType);

        return new IDCUtil.AccessNetworkInfo(rat, panInfo);
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
    public boolean isMobileDataEnabled()
    {
        return (mContext != null) ?
                SettingsUtils.isMobileDataEnabled(mContext.getContentResolver()) : false;
    }

    @Override
    public int getLteRsrpStrength() {
        final int defaultRsrp = 0;

        ITelephonyState ts = (ITelephonyState)AgentFactory.getAgent(
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
                        CellSignalStrengthLte lteStrength = (CellSignalStrengthLte)css;
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

        IDCApn dcapn = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
        if (dcapn == null) {
            return true;
        }

        String iface = dcapn.getIfaceName(apnType);
        String pingIface = "-I " + iface + " ";

        ImsLog.d(mSlotId, "apnType = " + apnType + " interfaceName = " + iface +
                " pingIface = " + pingIface);

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
        ISIMState ss = (ISIMState)AgentFactory.getAgent(AgentFactory.SIM_STATE, mSlotId);
        IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);

        if ((ss != null && !ss.isIccLoaded()) ||
                (dcnw != null && dcnw.isLteEmergencyOnly())) {
            TelephonyManager tm = null;

            if (MSimUtils.isMultiSimEnabled()) {
                tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
            } else {
                tm = AppContext.getTelephonyManager();
            }

            if (tm == null) {
                return;
            }

            if (mEmergencyCellInfoUpdater == null) {
                mEmergencyCellInfoUpdater = new ImsCellInfoUpdater();
            }

            ImsLog.d(mSlotId, "requestCellInfoUpdate(Emergency) - S");

            tm.requestCellInfoUpdate(mEmergencyCellInfoUpdater, mEmergencyCellInfoUpdater);

            ImsLog.d(mSlotId, "requestCellInfoUpdate(Emergency) - E");
        }
    }

    // Private/Protected methods ---------------------------------
    private List<CellInfo> getAllCellInfo() {
        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        } else {
            tm = AppContext.getTelephonyManager();
        }

        return (tm != null) ? tm.getAllCellInfo() : null;
    }

    private CellInfo getValidCellInfo(List<CellInfo> ciList , Class<?> T,
        boolean bCheckRegistered) {
        if (ciList == null) {
            ImsLog.w(mSlotId, "ciList is null");
            return null;
        }

        if (bCheckRegistered) {
            for (CellInfo ci : ciList) {
                if (T.isInstance(ci)) {
                    if (ci.isRegistered()) {
                        return ci;
                    }
                }
            }
        }
        else {
            for (CellInfo ci : ciList) {
                if (T.isInstance(ci)) {
                    return ci;
                }
            }
        }

        ImsLog.w(mSlotId, "No valid CellInfo");
        return null;
    }

    private String[] getNrPani(int rat) {
        String[] panInfo = null;

        panInfo = getNrInfoFromCellInfo();

        if (panInfo == null) {
            panInfo = getNrInfoFromCache();
        }

        return panInfo;
    }

    private String[] getLtePani(int rat) {
        String[] panInfo = null;

        panInfo = getLteInfoFromCellInfo();

        if (panInfo == null) {
            panInfo = getLteInfoFromCache();
        }

        return panInfo;
    }

    private String[] getEhrpdPani(int rat) {
        // TODO : update Sector ID and Subnet length
        //String[] panInfo = mIppApi.updateNGetCellInfo(rat);
        String[] panInfo = null;

        if ((panInfo == null) || (panInfo.length < EHRPD_PAN_INFO_ITEM_SIZE)) {
            ImsLog.w(mSlotId, "PANI is invalid");
            return null;
        }

        if (isCellInfoInvalid(panInfo[EHRPD_PAN_INFO_SECTOR_ID_INDEX]) ||
                isCellInfoInvalid(panInfo[EHRPD_PAN_INFO_SUBNET_LEN_INDEX])) {
            return null;
        }

        return panInfo;
    }

    private String[] getOthersPani(int rat) {
        // MCC & MNC
        ITelephonySubscriber subscriber = (ITelephonySubscriber)AgentFactory.getAgent(
                AgentFactory.TELEPHONY_SUBSCRIBER, mSlotId);

        if (subscriber == null) {
            return null;
        }

        String operator = subscriber.getMccMnc(false);

        if (operator == null) {
            ImsLog.w(mSlotId, "Network operator is null");
            return null;
        }

        if (operator.length() < 5) {
            ImsLog.w(mSlotId, "Network operator (" + operator + ") is not valid");
            return null;
        }

        // Permission :: ACCESS_COARSE_LOCATION
        List<CellInfo> allCellInfo = getAllCellInfo();
        CellInfoGsm cellInfoGsm = getCellInfoGsm(allCellInfo);

        CellIdentityGsm ci = cellInfoGsm.getCellIdentity();

        if (ci == null) {
            ImsLog.i(mSlotId, "CellIdentityGsm is null");
            return null;
        }

        int nLac = ci.getLac();
        if (nLac == (-1) || nLac > 0xffff) {
            ImsLog.e(mSlotId, "LAC is not valid; LAC=" + nLac);
            return null;
        }

        int nCid = ci.getCid();

        if (nCid == (-1) || nCid > 0xfffffff) {
            SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                    SubsInfoInterface.class, mSlotId);
            if (subsInfo != null && subsInfo.isTestModeEnabledForGcf()) {
                /*
                    for some PTCRB test equipment, if Cid is null,
                    result will be failure. Therefore set default value to 0
                */
                nCid = 0;
            } else {
                ImsLog.w(mSlotId, "Cell identity is not valid; CID=" + nCid);
                return null;
            }
        }

        String[] panInfo = new String[4];

        // MCC / MNC / Cell identity / LAC
        panInfo[0] = operator.substring(0, 3);
        panInfo[1] = operator.substring(3);
        panInfo[2] = Integer.toHexString(nCid);
        panInfo[3] = Integer.toHexString(nLac);

        return panInfo;
    }

    private String[] getNrInfoFromCellInfo() {
        ImsLog.d(mSlotId, "");

        // Permission :: ACCESS_COARSE_LOCATION
        List<CellInfo> allCellInfo = getAllCellInfo();
        CellInfoNr cellInfoNr = getCellInfoNr(allCellInfo);

        if (cellInfoNr == null) {
            requestCellInfoUpdate();

            allCellInfo = mCellInfoUpdater.getAllCellInfo();
            cellInfoNr = getCellInfoNr(allCellInfo);

            if (cellInfoNr == null) {
                return null;
            }
        }

        CellIdentityNr ci = (CellIdentityNr)cellInfoNr.getCellIdentity();

        if (ci == null) {
            ImsLog.w(mSlotId, "CellIdentityNr is null");
            return null;
        }

        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        long nci = ci.getNci();
        int tac = ci.getTac();

        if (TextUtils.isEmpty(mcc)) {
            ImsLog.i("CellIdentityNr :: mcc is invalid");
            return null;
        }

        if (TextUtils.isEmpty(mnc)) {
            ImsLog.i("CellIdentityNr :: mnc is invalid");
            return null;
        }

        if (nci == CellInfo.UNAVAILABLE_LONG) {
            ImsLog.i("CellIdentityNr :: nci is invalid; nci=" + nci);
            return null;
        }

        if (tac == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityNr :: tac is invalid; tac=" + tac);
            return null;
        }

        String[] panInfo = new String[LTE_PAN_INFO_ITEM_SIZE];
        panInfo[LTE_PAN_INFO_MCC_INDEX] = mcc;
        panInfo[LTE_PAN_INFO_MNC_INDEX] = mnc;
        panInfo[LTE_PAN_INFO_CELLID_INDEX] = Long.toHexString(nci);
        panInfo[LTE_PAN_INFO_TAC_INDEX] = Integer.toHexString(tac);

        setNrInfoToCache(panInfo);

        return panInfo;
    }

    private String[] getLteInfoFromCellInfo() {
        ImsLog.d(mSlotId, "");

        // Permission :: ACCESS_COARSE_LOCATION
        List<CellInfo> allCellInfo = getAllCellInfo();
        CellInfoLte cellInfoLte = getCellInfoLte(allCellInfo);

        if (cellInfoLte == null) {
            requestCellInfoUpdate();

            allCellInfo = mCellInfoUpdater.getAllCellInfo();
            cellInfoLte = getCellInfoLte(allCellInfo);

            if (cellInfoLte == null) {
                return null;
            }
        }

        CellIdentityLte ci = cellInfoLte.getCellIdentity();
        if (ci == null) {
            ImsLog.w(mSlotId, "CellIdentityLte is null");
            return null;
        }

        String mcc = ci.getMccString();
        if (TextUtils.isEmpty(mcc)) {
            ImsLog.w(mSlotId, "mcc is invalid; MCC = " + mcc);
            return null;
        }

        String mnc = ci.getMncString();
        if (TextUtils.isEmpty(mnc)) {
            ImsLog.w(mSlotId, "mnc is invalid; MNC = " + mnc);
            return null;
        }

        // getCi() returns invalid Cell-ID (-1) while SIB info is being decoded.
        int cellId = ci.getCi();
        if ((cellId == Integer.MAX_VALUE) || (cellId == -1)) {
            ImsLog.w(mSlotId, "cellId is invalid; cellId = " + cellId);
            return null;
        }

        // getTac() returns invalid TAC (0xffff, Character.MAX_VALUE) while SIB info is being decoded.
        // (ci.getTac() == Character.MAX_VALUE) is removed, the value is used for TAC in AT&T Lab.
        int tac = ci.getTac();
        if (tac == Integer.MAX_VALUE) {
            ImsLog.w(mSlotId, "tac is invalid; tac = " + tac);
            return null;
        }

        // MCC / MNC / Cell identity / TAC
        String[] panInfo = new String[LTE_PAN_INFO_ITEM_SIZE];
        panInfo[LTE_PAN_INFO_MCC_INDEX] = mcc;
        if (is3digitMncRequired()) {
            panInfo[LTE_PAN_INFO_MNC_INDEX] = (mnc.length() == 2) ? '0' + mnc : mnc;
        } else {
            panInfo[LTE_PAN_INFO_MNC_INDEX] = mnc;
        }
        panInfo[LTE_PAN_INFO_CELLID_INDEX] = Integer.toHexString(cellId);
        panInfo[LTE_PAN_INFO_TAC_INDEX] = Integer.toHexString(tac);

        IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (dcnw != null && (dcnw.getLteDuplexMode() != ServiceState.DUPLEX_MODE_UNKNOWN)) {
            panInfo[LTE_PAN_INFO_MODE_INDEX] =
                    (dcnw.getLteDuplexMode() == ServiceState.DUPLEX_MODE_FDD) ? "FDD" : "TDD";
        }

        setLteInfoToCache(panInfo);

        return panInfo;
    }

    private String[] getNrInfoFromCache() {
        ImsLog.d(mSlotId, "");

        if (mRecentNrInfo.isEmpty()) {
            ImsLog.w(mSlotId, "mRecentNrInfo is null");
            return null;
        }

        String[] panInfo = new String[mRecentNrInfo.size()];

        for (int i = 0; i < mRecentNrInfo.size(); i++) {
            panInfo[i] = mRecentNrInfo.get(i);
        }

        return panInfo;
    }

    private String[] getLteInfoFromCache() {
        ImsLog.d(mSlotId, "");

        if (mRecentLteInfo.isEmpty()) {
            ImsLog.w(mSlotId, "mRecentLteInfo is null");
            return null;
        }

        String[] panInfo = new String[mRecentLteInfo.size()];

        for (int i = 0; i < mRecentLteInfo.size(); i++) {
            panInfo[i] = mRecentLteInfo.get(i);
        }

        return panInfo;
    }

    private void setNrInfoToCache(String[] panInfo) {
        if (panInfo == null) {
            ImsLog.w(mSlotId, "panInfo is null");
            return;
        }

        mRecentNrInfo.clear();

        for (String item : panInfo) {
            mRecentNrInfo.add(item);
        }
    }

    private void setLteInfoToCache(String[] panInfo) {
        if (panInfo == null) {
            ImsLog.w(mSlotId, "panInfo is null");
            return;
        }

        mRecentLteInfo.clear();

        for (String item : panInfo) {
            mRecentLteInfo.add(item);
        }
    }

    private boolean isCellInfoInvalid(String value) {
        return (value == null) || (value.length() == 0);
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

    private Boolean is3digitMncRequired() {

        IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (dcnw != null) {
            String nOperatorNumeric = dcnw.getOperatorNumeric();
            if ((nOperatorNumeric != null) && (nOperatorNumeric.length() == 6)) {
                return true;
            }
        }

        return false;
    }

    private CellInfoNr getCellInfoNr(List<CellInfo> allCellInfo) {
        CellInfoNr cellInfoNr = (CellInfoNr)getValidCellInfo(allCellInfo, CellInfoNr.class, true);

        if (cellInfoNr == null) {
            ImsLog.w(mSlotId, "CellInfoLte is null");
            return null;
        }

        return cellInfoNr;
    }

    private CellInfoLte getCellInfoLte(List<CellInfo> allCellInfo) {
        CellInfoLte cellInfoLte = null;
        IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);

        if ((dcnw != null) && dcnw.isLteEmergencyOnly()) {
            cellInfoLte = (CellInfoLte)getValidCellInfo(allCellInfo, CellInfoLte.class, false);
        } else {
            cellInfoLte = (CellInfoLte)getValidCellInfo(allCellInfo, CellInfoLte.class, true);
        }

        if (cellInfoLte == null) {
            ImsLog.w(mSlotId, "CellInfoLte is null");
            return null;
        }

        return cellInfoLte;
    }

    private CellInfoGsm getCellInfoGsm(List<CellInfo> allCellInfo) {
        CellInfoGsm cellInfoGsm =
                (CellInfoGsm)getValidCellInfo(allCellInfo, CellInfoGsm.class, true);

        if (cellInfoGsm == null) {
            ImsLog.w(mSlotId, "CellInfoGsm is null");
            return null;
        }

        return cellInfoGsm;
    }

    private void requestCellInfoUpdate() {
        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        } else {
            tm = AppContext.getTelephonyManager();
        }

        if (tm == null) {
            return;
        }

        ImsLog.d(mSlotId, "requestCellInfoUpdate - S");

        tm.requestCellInfoUpdate(mCellInfoUpdater, mCellInfoUpdater);

        mCellInfoUpdater.waitForResponse(CELL_INFO_UPDATE_TIMEOUT_MILLS);

        ImsLog.d(mSlotId, "requestCellInfoUpdate - E");
    }

    private class ImsCellInfoUpdater extends TelephonyManager.CellInfoCallback
            implements Executor {
        private final Object mLock = new Object();
        private List<CellInfo> mLastCellInfoList = null;

        @Override
        public void onCellInfo(/*@NonNull*/ List<CellInfo> cellInfo) {
            if (!cellInfo.isEmpty()) {
                ImsLog.d(mSlotId, "onCellInfo");

                synchronized (mLock) {
                    mLastCellInfoList = cellInfo;
                }
            }

            synchronized (mLock) {
                mLock.notifyAll();
            }
        }

        @Override
        public void execute(Runnable r) {
            r.run();
        }

        public List<CellInfo> getAllCellInfo() {
            synchronized (mLock) {
                if (mLastCellInfoList == null) {
                    return null;
                }

                return new ArrayList<CellInfo>(mLastCellInfoList);
            }
        }

        public void waitForResponse(int millis) {
            synchronized (mLock) {
                try {
                    mLock.wait(millis);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
        }
    }
}
