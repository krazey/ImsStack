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
package com.android.imsstack.core.agents;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.CellInfo;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoNr;
import android.telephony.CellInfoWcdma;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.agents.agentif.ICellInfo;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.MSimUtils;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;
import java.util.concurrent.Executor;
import java.util.concurrent.RejectedExecutionException;

/**
 * This class monitors all the cell information of the mobile network and it manages
 * the last attached cell information for each radio access technology.
 * It also manages the time when the last valid access information is detected
 * as the UTC date/time format.
 * To start/stop using this component, the application SHALL invoke
 * CellInfoTrackter#startTrackingCellInfo / CellInfoAgent#stopTrackingCellInfo.
 */
public class CellInfoAgent implements ICellInfo {
    private static final int FEATURE_NONE = 0x00000000;
    private static final int FEATURE_STORE_LANI = 0x00000001;

    private static final int CELL_INFO_NONE = 0x00;
    private static final int CELL_INFO_LTE = 0x01;
    private static final int CELL_INFO_WCDMA = 0x02;
    private static final int CELL_INFO_GSM = 0x04;
    private static final int CELL_INFO_NR = 0x08;
    private static final int MAX_AN_INFO = 8;
    private static final String UTC_FORMAT = "yyyy-MM-dd'T'HH:mm:ss";
    private static final String UTC_FORMAT_OFFSET = "yyyy-MM-dd'T'HH:mm:ssXXX";

    private static final int EVENT_READ_ON_BOOTUP = 1001;
    private static final int EVENT_RAT_CHANGED = 1002;
    private static final int EVENT_VOICE_RAT_CHANGED = 1003;

    private static final String DELIMETER = ",";

    /**
     * To receive cell info. changed notification
     */
    private CellInfoListener mCellInfoListener = null;

    /**
     * Most recent network type
     */
    private int mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;

    /**
     * Cell info. will be tracked for all the network type (except for CDMA)
     *  mCellInfo : Most recent cell info. regardless of RAT
     *  mCellInfoLte : Most recent cell info. for LTE
     *  mCellInfoWcdma : Most recent cell info. for WCDMA
     *  mCellInfoGsm : Most recent cell info. for GSM
     *  CellInfoNr : Most recent cell info. for NR
     */
    private CellInfo mCellInfo = null;
    private CellInfoLte mCellInfoLte = null;
    private CellInfoWcdma mCellInfoWcdma = null;
    private CellInfoGsm mCellInfoGsm = null;
    private CellInfoNr mCellInfoNr = null;

    /**
     * Stores the timestamp when the cell info. is obtained as the current time
     */
    private long mTimeStamp = 0;
    private long mTimeStampLte = 0;
    private long mTimeStampWcdma = 0;
    private long mTimeStampGsm = 0;
    private long mTimeStampNr = 0;

    private int mFeatures = FEATURE_NONE;

    /**
     * To listen messages like the network status change.
     */
    private CellInfoHandler mCellInfoHandler;

    private enum ENetworkCategory {
        LTE,
        WCDMA,
        GSM,
        NR,
        NONE
    }

    private int mSlotId = 0;

    public CellInfoAgent(int slotId) {
        ImsLog.d(slotId, "");

        mSlotId = slotId;
        mCellInfoHandler = new CellInfoHandler(AppContext.getInstance().getMainLooper());
    }

    @Override
    public void init(Context context) {

    }

    @Override
    public void cleanup() {
        if (mCellInfoHandler != null) {
            mCellInfoHandler.removeCallbacksAndMessages(null);
        }
    }

    /**
     * Returns the most recent cell information with the network type and timestamp (UTC format).
     *  [0] : network type
     *  [1] : timestamp (UTC format)
     *  [2] : cell age (seconds format)
     *  [3...7] : access network information based on network type
     */
    public String[] getAccessNetworkInfo() {
        CellInfo cellInfo = null;
        int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        long timeStamp = 0;

        synchronized (this) {
            cellInfo = getCellInfo();
            networkType = getNetworkType();
            timeStamp = getTimeStamp();
        }

        return formAccessNetworkInfoToStringArray(cellInfo, networkType, timeStamp);
    }

    public String[] getAccessNetworkInfo(int networkType) {
        CellInfo cellInfo = null;
        long timeStamp = 0;

        synchronized (this) {
            cellInfo = getCellInfo(networkType);
            timeStamp = getTimeStamp(networkType);
        }

        return formAccessNetworkInfoToStringArray(cellInfo, networkType, timeStamp);
    }

    public void startTrackingCellInfo(Context context) {
        if (context == null) {
            return;
        }

        if (mCellInfoListener == null) {
            int subId = MSimUtils.getSubId(mSlotId);
            if (!MSimUtils.isValidSubId(subId)) {
                ImsLog.w(mSlotId, "invalid sub id =" + subId);
                return;
            }

            mCellInfoListener = new CellInfoListener(subId);
            setListener(mCellInfoListener);

            IDcNetWatcher dcnw =
                    (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
            if (dcnw != null) {
                dcnw.registerForRatChanged(mCellInfoHandler, EVENT_RAT_CHANGED, null);
                dcnw.registerForVoiceRatChanged(mCellInfoHandler, EVENT_VOICE_RAT_CHANGED, null);

                mCellInfoHandler.sendEmptyMessage(EVENT_READ_ON_BOOTUP);
            }
        }
    }

    public void stopTrackingCellInfo(Context context) {
        if (context == null) {
            return;
        }

        if (mCellInfoListener != null) {
            removeListener(mCellInfoListener);
            mCellInfoListener = null;

            IDcNetWatcher dcnw =
                    (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
            if (dcnw != null) {
                dcnw.unregisterForRatChanged(mCellInfoHandler);
                dcnw.unregisterForVoiceRatChanged(mCellInfoHandler);
            }
        }
    }

    public void setLastCellInfoStorage(boolean bStore) {
        ImsLog.d(mSlotId, "");

        if (bStore) {
            enableFeature(FEATURE_STORE_LANI);
        } else {
            disableFeature(FEATURE_STORE_LANI);
        }
    }

    private CellInfo getCellInfo() {
        return mCellInfo;
    }

    private int getNetworkType() {
        return mNetworkType;
    }

    private long getTimeStamp() {
        return mTimeStamp;
    }

    private CellInfo getCellInfo(int networkType) {
        ENetworkCategory netCategory = getNetworkCategory(networkType);
        if (netCategory == ENetworkCategory.LTE) {
            return mCellInfoLte;
        } else if (netCategory == ENetworkCategory.WCDMA) {
            return mCellInfoWcdma;
        } else if (netCategory == ENetworkCategory.GSM) {
            return mCellInfoGsm;
        } else if (netCategory == ENetworkCategory.NR) {
            return mCellInfoNr;
        }
        return null;
    }

    private CellInfo getCellInfoForType(int type) {
        if (type == CELL_INFO_LTE) {
            return mCellInfoLte;
        } else if (type == CELL_INFO_WCDMA) {
            return mCellInfoWcdma;
        } else if (type == CELL_INFO_GSM) {
            return mCellInfoGsm;
        } else if (type == CELL_INFO_NR) {
            return mCellInfoNr;
        }
        return null;
    }

    private long getTimeStamp(int networkType) {
        ENetworkCategory netCategory = getNetworkCategory(networkType);
        if (netCategory == ENetworkCategory.LTE) {
            return mTimeStampLte;
        } else if (netCategory == ENetworkCategory.WCDMA) {
            return mTimeStampWcdma;
        } else if (netCategory == ENetworkCategory.GSM) {
            return mTimeStampGsm;
        } else if (netCategory == ENetworkCategory.NR) {
            return mTimeStampNr;
        }
        return 0;
    }

    private ENetworkCategory getNetworkCategory(int networkType) {
        ImsLog.d(mSlotId, "networkType : " + networkType);

        if (networkType == TelephonyManager.NETWORK_TYPE_LTE) {
            return ENetworkCategory.LTE;
        } else if ((networkType == TelephonyManager.NETWORK_TYPE_UMTS)
                || (networkType == TelephonyManager.NETWORK_TYPE_HSDPA)
                || (networkType == TelephonyManager.NETWORK_TYPE_HSUPA)
                || (networkType == TelephonyManager.NETWORK_TYPE_HSPA)
                || (networkType == TelephonyManager.NETWORK_TYPE_HSPAP)) {
            return ENetworkCategory.WCDMA;
        } else if ((networkType == TelephonyManager.NETWORK_TYPE_GPRS)
                || (networkType == TelephonyManager.NETWORK_TYPE_EDGE)) {
            return ENetworkCategory.GSM;
        } else if (networkType == TelephonyManager.NETWORK_TYPE_NR) {
            return ENetworkCategory.NR;
        }
        return ENetworkCategory.NONE;
    }

    private static String getCellInfoAge(long timeStamp) {
        return String.valueOf((System.currentTimeMillis() - timeStamp) / 1000);
    }

    private String[] formAccessNetworkInfoToStringArray(CellInfo cellInfo,
            int networkType, long timeStamp) {
        String[] cellIdentity = createCellIdentity(cellInfo);

        if (cellIdentity == null) {
            if (isFeatureEnabled(FEATURE_STORE_LANI)) {
                return getStoredAccessNetworkInfo();
            } else {
                return null;
            }
        }

        String[] anInfo = new String[MAX_AN_INFO];

        anInfo[0] = String.valueOf(networkType);
        anInfo[1] = convertTimeToUTCFormat(timeStamp, isNumOffset());
        anInfo[2] = getCellInfoAge(timeStamp);
        anInfo[MAX_AN_INFO - 1] = "";

        System.arraycopy(cellIdentity, 0, anInfo, 3, cellIdentity.length);

        return anInfo;
    }

    private void removeListener(CellInfoListener listener) {
        ImsLog.d(mSlotId, "");

        TelephonyManager tm = AppContext.getTelephonyManager(listener.getSubId());

        if (tm != null) {
            tm.unregisterTelephonyCallback(listener);
        }
    }

    private void setListener(CellInfoListener listener) {
        TelephonyManager tm = AppContext.getTelephonyManager(listener.getSubId());

        if (tm != null) {
            tm.registerTelephonyCallback(mCellInfoHandler, listener);
        }
    }

    private int updateCellInfo(CellInfo ci) {
        if (ci == null) {
            return CELL_INFO_NONE;
        }

        if (validateCellIdentity(ci) == false) {
            ImsLog.i(mSlotId, "invalid cell identity");
            return CELL_INFO_NONE;
        }

        if (ci instanceof CellInfoLte) {
            mCellInfoLte = (CellInfoLte) ci;
            mTimeStampLte = System.currentTimeMillis();

            return CELL_INFO_LTE;
        } else if (ci instanceof CellInfoWcdma) {
            mCellInfoWcdma = (CellInfoWcdma) ci;
            mTimeStampWcdma = System.currentTimeMillis();

            return CELL_INFO_WCDMA;
        } else if (ci instanceof CellInfoGsm) {
            mCellInfoGsm = (CellInfoGsm) ci;
            mTimeStampGsm = System.currentTimeMillis();

            return CELL_INFO_GSM;
        } else if (ci instanceof CellInfoNr) {
            mCellInfoNr = (CellInfoNr)ci;
            mTimeStampNr = System.currentTimeMillis();

            return CELL_INFO_NR;
        } else {
            ImsLog.d(mSlotId, "Unknown cell information; ignore it");
        }

        return CELL_INFO_NONE;
    }

    private List<CellInfo> getAllCellInfo() {
        // TODO: need to be improved
        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        } else {
            tm = AppContext.getTelephonyManager();
        }

        return (tm != null) ? tm.getAllCellInfo() : null;
    }

    private CellInfo getMostRecentCellInfoForVoNR(int updatedCellInfo) {
        int[] cellType = new int[] {CELL_INFO_GSM, CELL_INFO_WCDMA, CELL_INFO_LTE, CELL_INFO_NR};
        CellInfo finalizedCellInfo = null;

        for (int i = 0; i < cellType.length; i++) {
            if ((updatedCellInfo & cellType[i]) != 0) {
                CellInfo cellInfo = getCellInfoForType(cellType[i]);
                if (cellInfo != null) {
                    if (finalizedCellInfo != null) {
                        if (finalizedCellInfo.getTimestampMillis() <=
                                cellInfo.getTimestampMillis()) {
                            finalizedCellInfo = cellInfo;
                        }
                    } else {
                        finalizedCellInfo = cellInfo;
                    }
                }
            }
        }

        return finalizedCellInfo;
    }

    private void updateAllCellInfo(List<CellInfo> cellInfo) {
        if (cellInfo == null) {
            cellInfo = getAllCellInfo();

            if (cellInfo == null) {
                ImsLog.i(mSlotId, "List<CellInfo> is null");
                return;
            }
        }

        int updatedCellInfo = CELL_INFO_NONE;
        boolean updated = false;

        synchronized (this) {
            for (int i = 0; i < cellInfo.size(); ++i) {
                CellInfo ci = cellInfo.get(i);

                if ((ci != null) && ci.isRegistered()) {
                    updatedCellInfo |= updateCellInfo(ci);
                }
            }

            if (updatedCellInfo != CELL_INFO_NONE) {
                updated = updateMostRecentCellInfo(updatedCellInfo);
            }
        }

        if (updated && isFeatureEnabled(FEATURE_STORE_LANI)) {
            storeAccessNetworkInfo();
        }
    }

    private boolean updateMostRecentCellInfo(int updatedCellInfo) {
        if (updatedCellInfo == CELL_INFO_LTE) {
            mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
            mCellInfo = mCellInfoLte;
            mTimeStamp = mTimeStampLte;
        } else if (updatedCellInfo == CELL_INFO_WCDMA) {
            mNetworkType = TelephonyManager.NETWORK_TYPE_UMTS;
            mCellInfo = mCellInfoWcdma;
            mTimeStamp = mTimeStampWcdma;
        } else if (updatedCellInfo == CELL_INFO_GSM) {
            mNetworkType = TelephonyManager.NETWORK_TYPE_GPRS;
            mCellInfo = mCellInfoGsm;
            mTimeStamp = mTimeStampGsm;
        } else if (updatedCellInfo == CELL_INFO_NR) {
            if (CapabilityConfigs.isVoNrEnabled(mSlotId)) {
                mNetworkType = TelephonyManager.NETWORK_TYPE_NR;
                mCellInfo = mCellInfoNr;
                mTimeStamp = mTimeStampNr;
            } else {
                return false;
            }
        } else {
            CellInfo recentCellInfo = null;
            if (CapabilityConfigs.isVoNrEnabled(mSlotId)) {
                recentCellInfo = getMostRecentCellInfoForVoNR(updatedCellInfo);
            } else {
                if ((updatedCellInfo & CELL_INFO_LTE) != 0) {
                    recentCellInfo = mCellInfoLte;
                }

                if ((updatedCellInfo & CELL_INFO_WCDMA) != 0) {
                    if (recentCellInfo == null) {
                        recentCellInfo = mCellInfoWcdma;
                    } else if ((mCellInfoWcdma != null)
                            && (mCellInfoWcdma.getTimestampMillis() >
                                    recentCellInfo.getTimestampMillis())) {
                        recentCellInfo = mCellInfoWcdma;
                    }
                }

                if ((updatedCellInfo & CELL_INFO_GSM) != 0) {
                    if (recentCellInfo == null) {
                        recentCellInfo = mCellInfoGsm;
                    } else if ((mCellInfoGsm != null)
                            && (mCellInfoGsm.getTimestampMillis() >
                                    recentCellInfo.getTimestampMillis())) {
                        recentCellInfo = mCellInfoGsm;
                    }
                }
            }

            if (recentCellInfo == null) {
                return false;
            }

            if (recentCellInfo instanceof CellInfoLte) {
                mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
                mCellInfo = (CellInfoLte)recentCellInfo;
                mTimeStamp = mTimeStampLte;
            } else if (recentCellInfo instanceof CellInfoWcdma) {
                mNetworkType = TelephonyManager.NETWORK_TYPE_UMTS;
                mCellInfo = (CellInfoWcdma)recentCellInfo;
                mTimeStamp = mTimeStampWcdma;
            } else if (recentCellInfo instanceof CellInfoGsm) {
                mNetworkType = TelephonyManager.NETWORK_TYPE_GPRS;
                mCellInfo = (CellInfoGsm)recentCellInfo;
                mTimeStamp = mTimeStampGsm;
            } else if (recentCellInfo instanceof CellInfoNr) {
                mNetworkType = TelephonyManager.NETWORK_TYPE_NR;
                mCellInfo = recentCellInfo;
                mTimeStamp = mTimeStampNr;
            } else {
                return false;
            }
        }

        ImsLog.i(mSlotId, "CellInfo :: " + ImsLog.hiddenString(mCellInfo.toString()) +
                ", TimeStamp=" + mTimeStamp);

        return true;
    }

    private static String convertTimeToUTCFormat(long currentTimeMillis, boolean isNumOffset) {
        SimpleDateFormat sdf = null;
        String utcFormat = null;

        if (isNumOffset) {
            sdf = new SimpleDateFormat(UTC_FORMAT_OFFSET, Locale.US);

            utcFormat = sdf.format(new Date(currentTimeMillis));
        } else {
            sdf = new SimpleDateFormat(UTC_FORMAT, Locale.US);
            sdf.setTimeZone(TimeZone.getTimeZone("UTC"));

            utcFormat = sdf.format(new Date(currentTimeMillis)) + "Z";
        }

        return utcFormat;
    }

    private static String[] createCellIdentity(CellInfo cellInfo) {
        if (cellInfo == null) {
            return null;
        }

        if (cellInfo instanceof CellInfoLte) {
            return createCellIdentityLte((CellInfoLte)cellInfo);
        } else if (cellInfo instanceof CellInfoWcdma) {
            return createCellIdentityWcdma((CellInfoWcdma)cellInfo);
        } else if (cellInfo instanceof CellInfoGsm) {
            return createCellIdentityGsm((CellInfoGsm)cellInfo);
        } else if (cellInfo instanceof CellInfoNr) {
            return createCellIdentityNr((CellInfoNr)cellInfo);
        }

        return null;
    }

    private static String[] createCellIdentityLte(CellInfoLte cellInfo) {
        if (cellInfo == null) {
            return null;
        }

        CellIdentityLte ci = cellInfo.getCellIdentity();

        if (ci == null) {
            ImsLog.i("CellIdentityLte is null");
            return null;
        }

        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        int cellId = ci.getCi();
        int tac = ci.getTac();

        if (TextUtils.isEmpty(mcc)) {
            ImsLog.i("CellIdentityLte :: mcc is invalid; mcc=" + mcc);
            return null;
        }

        if (TextUtils.isEmpty(mnc)) {
            ImsLog.i("CellIdentityLte :: mnc is invalid; mnc=" + mnc);
            return null;
        }

        if (cellId == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityLte :: ci is invalid; ci=" + cellId);
            return null;
        }

        if (tac == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityLte :: tac is invalid; tac=" + tac);
            return null;
        }

        String[] cellIdentity = new String[5];

        cellIdentity[0] = mcc;
        cellIdentity[1] = mnc;
        cellIdentity[2] = Integer.toHexString(cellId);
        cellIdentity[3] = Integer.toHexString(tac);
        // FIXME: need to identify LTE mode (FDD or TDD)
        cellIdentity[4] = "FDD";

        return cellIdentity;
    }

    private static String[] createCellIdentityWcdma(CellInfoWcdma cellInfo) {
        if (cellInfo == null) {
            return null;
        }

        CellIdentityWcdma ci = cellInfo.getCellIdentity();

        if (ci == null) {
            ImsLog.i("CellIdentityWcdma is null");
            return null;
        }

        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        int cellId = ci.getCid();
        int lac = ci.getLac();

        if (TextUtils.isEmpty(mcc)) {
            ImsLog.i("CellIdentityWcdma :: mcc is invalid; mcc=" + mcc);
            return null;
        }

        if (TextUtils.isEmpty(mnc)) {
            ImsLog.i("CellIdentityWcdma :: mnc is invalid; mnc=" + mnc);
            return null;
        }

        if (cellId == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityWcdma :: cid is invalid; cid=" + cellId);
            return null;
        }

        if (lac == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityWcdma :: lac is invalid; lac=" + lac);
            return null;
        }

        String[] cellIdentity = new String[4];

        cellIdentity[0] = mcc;
        cellIdentity[1] = mnc;
        cellIdentity[2] = Integer.toHexString(cellId);
        cellIdentity[3] = Integer.toHexString(lac);

        return cellIdentity;
    }

    private static String[] createCellIdentityGsm(CellInfoGsm cellInfo) {
        if (cellInfo == null) {
            return null;
        }

        CellIdentityGsm ci = cellInfo.getCellIdentity();

        if (ci == null) {
            ImsLog.i("CellIdentityGsm is null");
            return null;
        }

        String mcc = ci.getMccString();
        String mnc = ci.getMncString();
        int cellId = ci.getCid();
        int lac = ci.getLac();

        if (TextUtils.isEmpty(mcc)) {
            ImsLog.i("CellIdentityGsm :: mcc is invalid; mcc=" + mcc);
            return null;
        }

        if (TextUtils.isEmpty(mnc)) {
            ImsLog.i("CellIdentityGsm :: mnc is invalid; mnc=" + mnc);
            return null;
        }

        if (cellId == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityGsm :: cid is invalid; cid=" + cellId);
            return null;
        }

        if (lac == CellInfo.UNAVAILABLE) {
            ImsLog.i("CellIdentityGsm :: lac is invalid; lac=" + lac);
            return null;
        }

        String[] cellIdentity = new String[4];

        cellIdentity[0] = mcc;
        cellIdentity[1] = mnc;
        cellIdentity[2] = Integer.toHexString(cellId);
        cellIdentity[3] = Integer.toHexString(lac);

        return cellIdentity;
    }

    private static String[] createCellIdentityNr(CellInfoNr cellInfo) {
        if (cellInfo == null) {
            return null;
        }

        CellIdentityNr ci = (CellIdentityNr)cellInfo.getCellIdentity();

        if (ci == null) {
            ImsLog.i("CellIdentityNr is null");
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

        String[] cellIdentity = new String[5];

        cellIdentity[0] = mcc;
        cellIdentity[1] = mnc;
        cellIdentity[2] = Long.toHexString(nci);
        cellIdentity[3] = Integer.toHexString(tac);
        // FIXME: need to identify LTE mode (FDD or TDD)
        cellIdentity[4] = "TDD";

        return cellIdentity;
    }

    private boolean validateCellIdentity(CellInfo ci) {
        String[] cellIdentity = null;

        if (ci instanceof CellInfoLte) {
            CellInfoLte ciLte = (CellInfoLte)ci;
            cellIdentity = createCellIdentityLte(ciLte);

            if (cellIdentity == null) {
                return false;
            }
        } else if (ci instanceof CellInfoWcdma) {
            CellInfoWcdma ciWcdma = (CellInfoWcdma)ci;
            cellIdentity = createCellIdentityWcdma(ciWcdma);

            if (cellIdentity == null) {
                return false;
            }
        } else if (ci instanceof CellInfoGsm) {
            CellInfoGsm ciGsm = (CellInfoGsm)ci;
            cellIdentity = createCellIdentityGsm(ciGsm);

            if (cellIdentity == null) {
                return false;
            }
        } else if (ci instanceof CellInfoNr) {
            CellInfoNr ciNr = (CellInfoNr)ci;
            cellIdentity = createCellIdentityNr(ciNr);

            if (cellIdentity == null) {
                return false;
            }
        }

        return true;
    }

    private void storeAccessNetworkInfo() {
        String[] cellIdentity = createCellIdentity(getCellInfo());

        if (cellIdentity == null) {
            ImsLog.i(mSlotId, "cellIdentity is null");
            return;
        }

        String anInfoString = String.valueOf(getNetworkType());
        anInfoString = anInfoString.concat(DELIMETER);

        anInfoString = anInfoString.concat(convertTimeToUTCFormat(getTimeStamp(), isNumOffset()));
        anInfoString = anInfoString.concat(DELIMETER);

        anInfoString = anInfoString.concat(String.valueOf(getTimeStamp()));
        anInfoString = anInfoString.concat(DELIMETER);

        for (int i = 0; i < cellIdentity.length; i++) {
            if (i != 0) {
                anInfoString = anInfoString.concat(DELIMETER);
            }

            anInfoString = anInfoString.concat(cellIdentity[i]);
        }

        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO,
                anInfoString, mSlotId);

        ImsLog.i(mSlotId, "stored access info = " + anInfoString);
    }

    private String[] getStoredAccessNetworkInfo() {
        String anInfoString = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO, "", mSlotId);

        if (anInfoString.length() == 0) {
            return null;
        }

        String[] tokens = anInfoString.split(DELIMETER);

        if (tokens == null) {
            return null;
        }

        String[] anInfo = new String[MAX_AN_INFO];
        anInfo[0] = tokens[0];
        anInfo[1] = tokens[1];
        anInfo[2] = getCellInfoAge(Long.parseLong(tokens[2]));

        for (int i = 3; i < tokens.length; i++) {
            anInfo[i] = tokens[i];
        }

        return anInfo;
    }

    private void disableFeature(int feature) {
        mFeatures &= (~feature);
    }

    private void enableFeature(int feature) {
        mFeatures |= feature;
    }

    private boolean isFeatureEnabled(int feature) {
        return (mFeatures & feature) == feature;
    }

    private boolean isNumOffset() {
        if ("MTS".equals(OperatorInfo.getOperator(mSlotId))) {
            return true;
        }

        return false;
    }

    private final class CellInfoListener extends TelephonyCallback implements
        TelephonyCallback.CellInfoListener {

        private final int mSubId_;

        public CellInfoListener(int subId) {
            mSubId_ = subId;
            ImsLog.i(mSlotId, "CellInfoListener :: subId=" + subId);
        }

        public int getSubId() {
            return mSubId_;
        }

        /**
         * Callback invoked when a observed cell info has changed,
         * or new cells have been added or removed.
         * @param cellInfo the list of currently visible cells
         */
        @Override
        public void onCellInfoChanged(List<CellInfo> cellInfo) {
            if (cellInfo == null) {
                return;
            }

            updateAllCellInfo(cellInfo);
        }
    }

    private class CellInfoHandler extends Handler implements Executor {
        public CellInfoHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void execute(Runnable command) {
            if (!post(command)) {
                throw new RejectedExecutionException(this + " is shutting down");
            }
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i(mSlotId, "CellInfoHandler :: msg=" + msg.what);

            switch (msg.what) {
                case EVENT_READ_ON_BOOTUP: //FALL-THROUGH
                case EVENT_RAT_CHANGED:    //FALL-THROUGH
                case EVENT_VOICE_RAT_CHANGED:
                    IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(
                            DcFactory.NETWORK_WATCHER, mSlotId);

                    if (dcnw == null) {
                        break;
                    }

                    if ((dcnw.getNetworkType() == TelephonyManager.NETWORK_TYPE_UNKNOWN)
                        && (dcnw.getVoiceNetworkType() == TelephonyManager.NETWORK_TYPE_UNKNOWN)) {
                        break;
                    }

                    updateAllCellInfo(null);
                    break;
                default:
                    break;
            }
        }
    }
}
