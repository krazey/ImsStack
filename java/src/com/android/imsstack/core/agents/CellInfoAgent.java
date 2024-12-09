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

import android.annotation.NonNull;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.AccessNetworkUtils;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.CellInfo;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoNr;
import android.telephony.CellInfoWcdma;
import android.telephony.ServiceState;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.ArrayMap;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcm.DcUtils;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * A class for tracking all the cell information of the mobile network and it manages
 * the last attached cell information for each radio access technology.
 * It also manages the time when the last valid access information was obatined
 * as the UTC date/time format.
 * The interested components SHOULD invoke {@link #startTrackingCellInfo} and
 * {@link #stopTrackingCellInfo} to start and stop tracking the cell information.
 */
public class CellInfoAgent implements CellInfoInterface {
    private static final int MAX_ACCESS_NETWORK_INFO = 8;
    @VisibleForTesting
    protected static final int ANI_INDEX_NETWORK_TYPE = 0;
    @VisibleForTesting
    protected static final int ANI_INDEX_UTC_TIME_FORMAT = 1;
    @VisibleForTesting
    protected static final int ANI_INDEX_CELL_INFO_AGE = 2;

    private static final int EVENT_UPDATE_ALL_CELL_INFO = 1001;
    private static final int EVENT_NETWORK_TYPE_CHANGED = 1002;
    private static final int EVENT_VOICE_NETWORK_TYPE_CHANGED = 1003;

    private static final String DELIMETER = ",";

    // Mapping from the radio access network type to the telephony's network type.
    private static final Map<Integer, List<Integer>>
            ACCESS_NETWORK_TYPE_TO_TELEPHONY_NETWORK_TYPE = Map.ofEntries(
            Map.entry(AccessNetworkType.EUTRAN, List.of(TelephonyManager.NETWORK_TYPE_LTE)),
            Map.entry(AccessNetworkType.NGRAN, List.of(TelephonyManager.NETWORK_TYPE_NR)),
            Map.entry(AccessNetworkType.UTRAN, List.of(
                    TelephonyManager.NETWORK_TYPE_UMTS,
                    TelephonyManager.NETWORK_TYPE_HSDPA,
                    TelephonyManager.NETWORK_TYPE_HSPAP,
                    TelephonyManager.NETWORK_TYPE_HSUPA,
                    TelephonyManager.NETWORK_TYPE_HSPA,
                    TelephonyManager.NETWORK_TYPE_TD_SCDMA)),
            Map.entry(AccessNetworkType.GERAN, List.of(
                    TelephonyManager.NETWORK_TYPE_GPRS,
                    TelephonyManager.NETWORK_TYPE_EDGE,
                    TelephonyManager.NETWORK_TYPE_GSM))
            );

    private static final class ImsCellInfo {
        // Most recent cell information for an access network type.
        private CellInfo mCellInfo;
        // Timestamp when this cell information is obtained as the current time in milli-seconds.
        private long mTimestamp;

        public void copyFrom(ImsCellInfo other) {
            mCellInfo = other.mCellInfo;
            mTimestamp = other.mTimestamp;
        }

        public CellInfo getCellInfo() {
            return mCellInfo;
        }

        public long getTimestamp() {
            return mTimestamp;
        }

        public void setCellInfo(CellInfo cellInfo) {
            mCellInfo = cellInfo;
            mTimestamp = System.currentTimeMillis();
        }

        @Override
        public String toString() {
            return "{ ImsCellInfo: cellInfo="
                    + (mCellInfo != null ? Log.pii(mCellInfo.toString()) : "null")
                    + ", timestamp=" + mTimestamp + " }";
        }
    }

    private final ConfigInterface.Listener mConfigListener = new ConfigInterface.Listener() {
        @Override
        public void onCarrierConfigChanged(int slotId, int subId) {
            ImsLog.d(this, mSlotId, "onCarrierConfigChanged");
            mTimeOffsetEnabledForUtcTimeFormat = isTimeOffsetEnabledForUtcTimeFormat();
        }
    };

    private final int mSlotId;
    // To listen events like the network type change.
    private final CellInfoHandler mCellInfoHandler;
    // To receive cell information changed notification.
    private CellInfoListener mCellInfoListener;
    private TelephonyManager.CellInfoCallback mCellInfoCallback;
    // Most recent network type.
    private int mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
    // Most recent cell information.
    private final ImsCellInfo mRecentCellInfo = new ImsCellInfo();
    // Mapping from the access network type to its cell information.
    private final ArrayMap<Integer, ImsCellInfo> mCellInfoPerAccessNetworkType =
            new ArrayMap<>(ACCESS_NETWORK_TYPE_TO_TELEPHONY_NETWORK_TYPE.size());
    private boolean mTimeOffsetEnabledForUtcTimeFormat;
    private IDcNetWatcher.Listener mNetWatcherListener;

    public CellInfoAgent(int slotId) {
        mSlotId = slotId;
        mCellInfoHandler = new CellInfoHandler(AppContext.getInstance().getMainLooper());

        mCellInfoPerAccessNetworkType.put(AccessNetworkType.EUTRAN, new ImsCellInfo());
        mCellInfoPerAccessNetworkType.put(AccessNetworkType.NGRAN, new ImsCellInfo());
        mCellInfoPerAccessNetworkType.put(AccessNetworkType.UTRAN, new ImsCellInfo());
        mCellInfoPerAccessNetworkType.put(AccessNetworkType.GERAN, new ImsCellInfo());
    }

    @Override
    public void init(Context context) {
        ImsLog.d(this, mSlotId, "init");
        ConfigInterface config = getConfigInterface();
        if (config != null) {
            config.addListener(mConfigListener);
        }
    }

    @Override
    public void cleanup() {
        mCellInfoHandler.removeCallbacksAndMessages(null);
        stopTrackingCellInfo();
        mTimeOffsetEnabledForUtcTimeFormat = false;

        ConfigInterface config = getConfigInterface();
        if (config != null) {
            config.removeListener(mConfigListener);
        }
    }

    /**
     * Returns the most recent cell information with the network type and timestamp (UTC format).
     *  [0] : network type
     *  [1] : timestamp (UTC format)
     *  [2] : cell age (seconds format)
     *  [3...7] : access network information based on network type
     */
    @Override
    public String[] getAccessNetworkInfo() {
        return formAccessNetworkInfo(mNetworkType, mRecentCellInfo);
    }

    @Override
    public String[] getAccessNetworkInfo(int networkType) {
        int accessNetworkType = getAccessNetworkTypeFromTelephonyNetworkType(networkType);
        ImsCellInfo recentCellInfo = mCellInfoPerAccessNetworkType.get(accessNetworkType);
        return formAccessNetworkInfo(networkType, recentCellInfo);
    }

    @Override
    public void startTrackingCellInfo() {
        if (mCellInfoListener == null) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            int subId = (sim != null) ? sim.getSubId() : MSimUtils.INVALID_SUB_ID;

            if (!MSimUtils.isValidSubId(subId)) {
                ImsLog.w(this, mSlotId, "CellInfo: Invalid sub id.");
                return;
            }

            mCellInfoListener = new CellInfoListener(subId);
            mCellInfoListener.register();

            if (mNetWatcherListener == null) {
                IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
                if (dnw != null) {
                    mNetWatcherListener = new NetWatcherListener();
                    dnw.addListener(mNetWatcherListener);
                }
            }

            ImsLog.i(this, mSlotId, "CellInfo: start");
            mCellInfoHandler.sendEmptyMessage(EVENT_UPDATE_ALL_CELL_INFO);
        }
    }

    @Override
    public void stopTrackingCellInfo() {
        if (mCellInfoListener != null) {
            ImsLog.i(this, mSlotId, "CellInfo: stop");
            mCellInfoListener.unregister();
            mCellInfoListener = null;

            if (mNetWatcherListener != null) {
                IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
                if (dnw != null) {
                    dnw.removeListener(mNetWatcherListener);
                }
                mNetWatcherListener = null;
            }
        }
    }

    /**
     * Sets the time offset flag to indicate whether the time offset format is used
     * for UTC time format for testing purpose.
     *
     * @param timeOffsetEnabled A flag specifying that the time offset is enabled.
     */
    @VisibleForTesting
    protected void setTimeOffsetEnabledForUtcTimeFormat(boolean timeOffsetEnabled) {
        mTimeOffsetEnabledForUtcTimeFormat = timeOffsetEnabled;
    }

    private String[] formAccessNetworkInfo(int networkType, ImsCellInfo imsCellInfo) {
        String[] cellIdentity = formCellIdentity(imsCellInfo.getCellInfo(), mSlotId);

        if (cellIdentity == null) {
            ImsLog.d(this, mSlotId, "CellInfo: fallback to cached LANI.");
            return getAccessNetworkInfoFromPersistentStorage();
        }

        String[] ani = new String[MAX_ACCESS_NETWORK_INFO];

        ani[ANI_INDEX_NETWORK_TYPE] = String.valueOf(networkType);
        ani[ANI_INDEX_UTC_TIME_FORMAT] = convertTimeToUtcFormat(imsCellInfo.getTimestamp());
        ani[ANI_INDEX_CELL_INFO_AGE] = getCellInfoAge(imsCellInfo.getTimestamp());
        System.arraycopy(cellIdentity, 0, ani, 3, cellIdentity.length);

        return ani;
    }

    private @NonNull List<CellInfo> getAllCellInfo() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        int subId = (sim != null) ? sim.getSubId() : MSimUtils.INVALID_SUB_ID;
        List<CellInfo> cellInfos = null;
        if (MSimUtils.isValidSubId(subId)) {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
            cellInfos = tmp.getAllCellInfo();

            if (cellInfos == null || cellInfos.isEmpty()) {
                if (mCellInfoCallback == null) {
                    mCellInfoCallback = new TelephonyManager.CellInfoCallback() {
                        @Override
                        public void onCellInfo(@NonNull List<CellInfo> cellInfo) {
                            ImsLog.d(this, mSlotId, "onCellInfo");
                            updateAllCellInfo(cellInfo);
                        }
                    };
                }
                tmp.requestCellInfoUpdate(mCellInfoHandler::post, mCellInfoCallback);
            }
        }
        return (cellInfos != null) ? cellInfos : Collections.emptyList();
    }

    private int updateCellInfo(@NonNull CellInfo cellInfo) {
        int accessNetworkType = checkAndGetAccessNetworkTypeFromCellInfo(cellInfo);
        ImsCellInfo imsCellInfo = mCellInfoPerAccessNetworkType.get(accessNetworkType);
        if (imsCellInfo != null) {
            imsCellInfo.setCellInfo(cellInfo);
        }
        return accessNetworkType;
    }

    private void updateAllCellInfo(@NonNull List<CellInfo> cellInfos) {
        if (cellInfos.isEmpty()) {
            return;
        }

        List<Integer> updatedAccessNetworkTypes = new ArrayList<>();

        cellInfos.stream()
                .filter(cellInfo -> cellInfo != null && cellInfo.isRegistered())
                .forEach(cellInfo -> {
                    int accessNetworkType = updateCellInfo(cellInfo);
                    if (accessNetworkType != AccessNetworkType.UNKNOWN) {
                        updatedAccessNetworkTypes.add(accessNetworkType);
                    }
                });

        // If NR is not supported, then remove the updated access network type
        // because NR cell information will not be used by the ImsStack.
        if (!CapabilityConfigs.isVoNrEnabled(mSlotId)) {
            updatedAccessNetworkTypes.remove(Integer.valueOf(AccessNetworkType.NGRAN));
        }

        if (updatedAccessNetworkTypes.isEmpty()) {
            // No updates.
            return;
        }

        // Priority: NGRAN > EUTRAN > UTRAN > GERAN
        final List<Integer> accessNetworkTypes = updatedAccessNetworkTypes.stream()
                .sorted(Comparator.reverseOrder())
                .collect(Collectors.toList());
        int recentAccessNetworkType = accessNetworkTypes.get(0);
        ImsCellInfo recentImsCellInfo = mCellInfoPerAccessNetworkType.get(
                recentAccessNetworkType);

        for (int i = 1; i < accessNetworkTypes.size(); ++i) {
            int accessNetworkType = accessNetworkTypes.get(i);
            ImsCellInfo imsCellInfo = mCellInfoPerAccessNetworkType.get(accessNetworkType);
            CellInfo cellInfo = imsCellInfo.getCellInfo();
            CellInfo recentCellInfo = recentImsCellInfo.getCellInfo();

            if (cellInfo.getTimestampMillis() > recentCellInfo.getTimestampMillis()) {
                recentAccessNetworkType = accessNetworkType;
                recentImsCellInfo = imsCellInfo;
            }
        }

        mNetworkType = getTelephonyNetworkTypeFromAccessNetworkType(recentAccessNetworkType);
        mRecentCellInfo.copyFrom(recentImsCellInfo);
        ImsLog.i(this, mSlotId, mRecentCellInfo.toString());
        setAccessNetworkInfoToPersistentStorage();
    }

    private String[] getAccessNetworkInfoFromPersistentStorage() {
        String lastAni = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO, "", mSlotId);
        String[] ani = lastAni.split(DELIMETER);

        if (ani == null || ani.length != MAX_ACCESS_NETWORK_INFO) {
            return null;
        }

        // Update cell-info-age based on the current time.
        ani[ANI_INDEX_CELL_INFO_AGE] = getCellInfoAge(
                Long.parseLong(ani[ANI_INDEX_CELL_INFO_AGE]));
        return ani;
    }

    private void setAccessNetworkInfoToPersistentStorage() {
        String[] cellIdentity = formCellIdentity(mRecentCellInfo.getCellInfo(), mSlotId);

        if (cellIdentity != null) {
            StringBuilder sb = new StringBuilder();
            sb.append(mNetworkType);
            sb.append(DELIMETER);
            sb.append(convertTimeToUtcFormat(mRecentCellInfo.getTimestamp()));
            sb.append(DELIMETER);
            sb.append(mRecentCellInfo.getTimestamp());
            sb.append(DELIMETER);
            sb.append(cellIdentity[0]);

            for (int i = 1; i < cellIdentity.length; ++i) {
                sb.append(DELIMETER);
                sb.append(cellIdentity[i]);
            }

            String lastAni = sb.toString();
            ImsPrivateProperties.Persistent.set(
                    ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO,
                    lastAni, mSlotId);
            ImsLog.i(this, mSlotId, "CellInfo: store LANI=" + ImsLog.hiddenString(lastAni));
        }
    }

    private String convertTimeToUtcFormat(long timeMillis) {
        DateTimeFormatter dtf;
        ZonedDateTime dateTime;

        if (mTimeOffsetEnabledForUtcTimeFormat) {
            dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ssxxx", Locale.US);
            dateTime = Instant.ofEpochMilli(timeMillis).atZone(ZoneId.systemDefault());
        } else {
            dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
            dateTime = Instant.ofEpochMilli(timeMillis).atZone(ZoneId.of("UTC"));
        }
        return dateTime.format(dtf);
    }

    private int checkAndGetAccessNetworkTypeFromCellInfo(CellInfo cellInfo) {
        if (cellInfo instanceof CellInfoLte) {
            CellIdentityLte ci = (CellIdentityLte) cellInfo.getCellIdentity();
            if (isCellIdentityLteValid(ci)) {
                return AccessNetworkType.EUTRAN;
            } else {
                ImsLog.i(this, mSlotId, cellIdentityLteToString(ci));
            }
        } else if (cellInfo instanceof CellInfoNr) {
            CellIdentityNr ci = (CellIdentityNr) cellInfo.getCellIdentity();
            if (isCellIdentityNrValid(ci)) {
                return AccessNetworkType.NGRAN;
            } else {
                ImsLog.i(this, mSlotId, cellIdentityNrToString(ci));
            }
        } else if (cellInfo instanceof CellInfoWcdma) {
            CellIdentityWcdma ci = (CellIdentityWcdma) cellInfo.getCellIdentity();
            if (isCellIdentityWcdmaValid(ci)) {
                return AccessNetworkType.UTRAN;
            } else {
                ImsLog.i(this, mSlotId, cellIdentityWcdmaToString(ci));
            }
        } else if (cellInfo instanceof CellInfoGsm) {
            CellIdentityGsm ci = (CellIdentityGsm) cellInfo.getCellIdentity();
            if (isCellIdentityGsmValid(ci)) {
                return AccessNetworkType.GERAN;
            } else {
                ImsLog.i(this, mSlotId, cellIdentityGsmToString(ci));
            }
        }
        return AccessNetworkType.UNKNOWN;
    }

    private static int getAccessNetworkTypeFromTelephonyNetworkType(int networkType) {
        return ACCESS_NETWORK_TYPE_TO_TELEPHONY_NETWORK_TYPE.entrySet().stream()
                .filter(entry -> entry.getValue().contains(networkType))
                .map(Map.Entry::getKey)
                .findFirst()
                .orElse(AccessNetworkType.UNKNOWN);
    }

    private static int getTelephonyNetworkTypeFromAccessNetworkType(int accessNetworkType) {
        return ACCESS_NETWORK_TYPE_TO_TELEPHONY_NETWORK_TYPE.entrySet().stream()
                .filter(entry -> entry.getKey() == accessNetworkType)
                .map(Map.Entry::getValue)
                .flatMap(Collection::stream)
                .findFirst()
                .orElse(TelephonyManager.NETWORK_TYPE_UNKNOWN);
    }

    private static String getCellInfoAge(long timestamp) {
        return String.valueOf((System.currentTimeMillis() - timestamp) / 1000);
    }

    private static String[] formCellIdentity(CellInfo cellInfo, int slotId) {
        if (cellInfo instanceof CellInfoLte) {
            CellIdentityLte ci = (CellIdentityLte) cellInfo.getCellIdentity();
            // TODO: need to use the public or system API for this.
            int band = AccessNetworkUtils.getOperatingBandForEarfcn(ci.getEarfcn());
            int duplexMode = AccessNetworkUtils.getDuplexModeForEutranBand(band);
            return formCellIdentity(
                    ci.getMccString(),
                    ci.getMncString(),
                    Integer.toHexString(ci.getCi()),
                    Integer.toHexString(ci.getTac()),
                    (duplexMode == ServiceState.DUPLEX_MODE_TDD) ? "TDD" : "FDD");
        } else if (cellInfo instanceof CellInfoNr) {
            CellIdentityNr ci = (CellIdentityNr) cellInfo.getCellIdentity();
            int duplexMode = DcUtils.getDuplexModeForNr(ci, slotId);
            return formCellIdentity(
                    ci.getMccString(),
                    ci.getMncString(),
                    Long.toHexString(ci.getNci()),
                    Integer.toHexString(ci.getTac()),
                    (duplexMode == ServiceState.DUPLEX_MODE_FDD) ? "FDD" : "TDD");
        } else if (cellInfo instanceof CellInfoWcdma) {
            CellIdentityWcdma ci = (CellIdentityWcdma) cellInfo.getCellIdentity();
            return formCellIdentity(
                    ci.getMccString(),
                    ci.getMncString(),
                    Integer.toHexString(ci.getCid()),
                    Integer.toHexString(ci.getLac()),
                    "");
        } else if (cellInfo instanceof CellInfoGsm) {
            CellIdentityGsm ci = (CellIdentityGsm) cellInfo.getCellIdentity();
            return formCellIdentity(
                    ci.getMccString(),
                    ci.getMncString(),
                    Integer.toHexString(ci.getCid()),
                    Integer.toHexString(ci.getLac()),
                    "");
        }
        return null;
    }

    private static boolean isCellIdentityLteValid(CellIdentityLte ci) {
        return !(TextUtils.isEmpty(ci.getMccString())
                || TextUtils.isEmpty(ci.getMncString())
                || ci.getCi() == CellInfo.UNAVAILABLE
                || ci.getTac() == CellInfo.UNAVAILABLE);
    }

    private static boolean isCellIdentityNrValid(CellIdentityNr ci) {
        return !(TextUtils.isEmpty(ci.getMccString())
                || TextUtils.isEmpty(ci.getMncString())
                || ci.getNci() == CellInfo.UNAVAILABLE_LONG
                || ci.getTac() == CellInfo.UNAVAILABLE);
    }

    private static boolean isCellIdentityWcdmaValid(CellIdentityWcdma ci) {
        return !(TextUtils.isEmpty(ci.getMccString())
                || TextUtils.isEmpty(ci.getMncString())
                || ci.getCid() == CellInfo.UNAVAILABLE
                || ci.getLac() == CellInfo.UNAVAILABLE);
    }

    private static boolean isCellIdentityGsmValid(CellIdentityGsm ci) {
        return !(TextUtils.isEmpty(ci.getMccString())
                || TextUtils.isEmpty(ci.getMncString())
                || ci.getCid() == CellInfo.UNAVAILABLE
                || ci.getLac() == CellInfo.UNAVAILABLE);
    }

    private static String cellIdentityLteToString(CellIdentityLte ci) {
        return ci.getClass().getSimpleName() + ": mcc=" + ci.getMccString()
                + ", mnc=" + ci.getMncString()
                + ", ci=" + ImsLog.hiddenString(String.valueOf(ci.getCi()))
                + ", tac=" + ImsLog.hiddenString(String.valueOf(ci.getTac()));
    }

    private static String cellIdentityNrToString(CellIdentityNr ci) {
        return ci.getClass().getSimpleName() + ": mcc=" + ci.getMccString()
                + ", mnc=" + ci.getMncString()
                + ", nci=" + ImsLog.hiddenString(String.valueOf(ci.getNci()))
                + ", tac=" + ImsLog.hiddenString(String.valueOf(ci.getTac()));
    }

    private static String cellIdentityWcdmaToString(CellIdentityWcdma ci) {
        return ci.getClass().getSimpleName() + ": mcc=" + ci.getMccString()
                + ", mnc=" + ci.getMncString()
                + ", cid=" + ImsLog.hiddenString(String.valueOf(ci.getCid()))
                + ", lac=" + ImsLog.hiddenString(String.valueOf(ci.getLac()));
    }

    private static String cellIdentityGsmToString(CellIdentityGsm ci) {
        return ci.getClass().getSimpleName() + ": mcc=" + ci.getMccString()
                + ", mnc=" + ci.getMncString()
                + ", cid=" + ImsLog.hiddenString(String.valueOf(ci.getCid()))
                + ", lac=" + ImsLog.hiddenString(String.valueOf(ci.getLac()));
    }

    private static String[] formCellIdentity(String mcc, String mnc,
            String cid, String tacOrLac, String duplextMode) {
        return new String[] { mcc, mnc, cid, tacOrLac, duplextMode };
    }

    private boolean isTimeOffsetEnabledForUtcTimeFormat() {
        ConfigInterface config = getConfigInterface();
        if (config != null) {
            CarrierConfig cc = config.getCarrierConfig();
            return cc.getBoolean(
                    CarrierConfig.Ims.KEY_CELLULAR_NETWORK_INFO_UTC_OFFSET_ENABLED_BOOL);
        }
        return false;
    }

    private ConfigInterface getConfigInterface() {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, mSlotId);
    }

    private final class CellInfoListener extends TelephonyCallback implements
            TelephonyCallback.CellInfoListener {
        private final int mSubId;

        public CellInfoListener(int subId) {
            ImsLog.i(this, mSlotId, "CellInfoListener: subId=" + subId);
            mSubId = subId;
        }

        public void register() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.registerTelephonyCallback(mCellInfoHandler::post, this);
        }

        public void unregister() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.unregisterTelephonyCallback(this);
        }

        @Override
        public void onCellInfoChanged(@NonNull List<CellInfo> cellInfo) {
            updateAllCellInfo(cellInfo);
        }
    }

    private final class NetWatcherListener implements IDcNetWatcher.Listener {
        @Override
        public void onDataNetworkTypeChanged() {
            mCellInfoHandler.sendEmptyMessage(EVENT_NETWORK_TYPE_CHANGED);
        }

        @Override
        public void onVoiceNetworkTypeChanged() {
            mCellInfoHandler.sendEmptyMessage(EVENT_VOICE_NETWORK_TYPE_CHANGED);
        }
    }

    private final class CellInfoHandler extends Handler {
        public CellInfoHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            ImsLog.i(this, mSlotId, "handleMessage: msg=" + msg.what);

            switch (msg.what) {
                case EVENT_UPDATE_ALL_CELL_INFO: // fall through
                case EVENT_NETWORK_TYPE_CHANGED: // fall through
                case EVENT_VOICE_NETWORK_TYPE_CHANGED:
                    IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
                    if (dnw != null
                            && (dnw.getNetworkType() != TelephonyManager.NETWORK_TYPE_UNKNOWN
                                    || dnw.getVoiceNetworkType()
                                            != TelephonyManager.NETWORK_TYPE_UNKNOWN)) {
                        updateAllCellInfo(getAllCellInfo());
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
