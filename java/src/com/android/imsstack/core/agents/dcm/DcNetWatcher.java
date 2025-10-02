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

import static android.provider.Settings.Global.AIRPLANE_MODE_ON;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.Annotation.NetworkType;
import android.telephony.DataSpecificRegistrationInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.VopsSupportInfo;

import androidx.annotation.NonNull;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.stream.IntStream;

/** This class is for providing the network information */
public class DcNetWatcher implements IDcNetWatcher {
    @VisibleForTesting
    protected static final int EVENT_AIRPLANE_MODE_CHANGED = 2001;

    // Default IMS supported network types
    private static final int[] DEFAULT_IMS_SUPPORTED_NETWORKS = new int[]{
            AccessNetworkConstants.AccessNetworkType.EUTRAN,
            AccessNetworkConstants.AccessNetworkType.NGRAN,
            AccessNetworkConstants.AccessNetworkType.IWLAN
    };

    private Context mContext;

    private IDcSettings mDcSettings;

    private DcNetWatcherReceiver mDcNetWatcherReceiver = null;
    @VisibleForTesting
    protected Handler mDcNetWatcherHandler;
    @VisibleForTesting
    protected DcNetWatcherPhoneStateListener mPhoneStateListener = null;
    protected DcNetWatcherConfigListener mConfigListener = null;
    @VisibleForTesting
    protected List<Integer> mImsSupportedAccessNetworks = Collections.emptyList();

    private DcNetWatcherNativeStateListener mNativeStateListener;

    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    private int mVoiceRoamingType = 0;
    private int mDataRoamingType = 0;

    private int mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    private int mVoiceNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    private int mVoiceServiceState = ServiceState.STATE_OUT_OF_SERVICE;
    private int mDataServiceState = ServiceState.STATE_OUT_OF_SERVICE;
    private int mCellularDataServiceState = ServiceState.STATE_OUT_OF_SERVICE;
    private int mNetworkRegistrationState =
            NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    private int mNetworkRegistrationRejectCause = REGISTRATION_REJECT_CAUSE_NONE;
    private String mNetworkOperator = "";
    // mDataRoaming and mVoiceRoaming refer to the roamingType.
    // So they could be overridden by the carrier config
    private boolean mDataRoaming = false;
    private boolean mVoiceRoaming = false;
    private boolean mRoamingChanged = false;
    // data network roaming state that was not overridden by any carrier config
    private boolean mDataNetworkRoaming = false;
    private boolean mAirplaneMode = false;

    // Network types in TelephonyManager for sync with ServiceState
    @VisibleForTesting
    protected int mTelephonyNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    @VisibleForTesting
    protected int mTelephonyVoiceNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;

    // IMS voice over PS Session Supported
    private int mImsVopsState = ImsEventDef.IMS_VOICE_OVER_PS_INVALID;
    private String mImsVopsPlmn = "";
    // Emergency bearer support capability
    private boolean mEmcbs = false;

    private int mLteAttachResultType = ImsEventDef.IMS_LTE_INFO_UNKNOWN;
    private int mLteAttachExtraInfo = ImsEventDef.IMS_LTE_INFO_EXTRA_NONE;

    private ISystem mSystem;
    private final int mSlotId;

    public DcNetWatcher(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "");

        if (context == null) {
            return;
        }

        mContext = context;

        mDcSettings = DcFactory.getDcAgent(IDcSettings.class, mSlotId);
        if (mDcSettings == null) {
            ImsLog.w(mSlotId, "IDcSettings is null");
        }

        loadImsSupportedAccessNetworks();
        mAirplaneMode = (getAirplaneMode() == 1);

        mDcNetWatcherHandler = new DcNetWatcherHandler(Looper.myLooper());

        mSystem = SystemInterface.getInstance().getSystem(mSlotId);
        if (mSystem == null) {
            return;
        }

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        if (nsi != null) {
            mNativeStateListener = new DcNetWatcherNativeStateListener();
            nsi.addListener(mNativeStateListener);
        }

        mDcNetWatcherReceiver = new DcNetWatcherReceiver();
        mDcNetWatcherReceiver.register();

        mPhoneStateListener = new DcNetWatcherPhoneStateListener();
        mPhoneStateListener.setListener();

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);
        if (config != null) {
            mConfigListener = new DcNetWatcherConfigListener();
            config.addListener(mConfigListener);
        }
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "");
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);
        if (config != null && mConfigListener != null) {
            config.removeListener(mConfigListener);
            mConfigListener = null;
        }

        if (mDcNetWatcherHandler != null) {
            mDcNetWatcherHandler.removeCallbacksAndMessages(null);
        }

        if (mPhoneStateListener != null) {
            mPhoneStateListener.dispose();
            mPhoneStateListener = null;
        }

        if (mDcNetWatcherReceiver != null) {
            mDcNetWatcherReceiver.unregister();
            mDcNetWatcherReceiver = null;
        }

        if (mNativeStateListener != null) {
            NativeStateInterface nsi =
                    AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }

        mSystem = null;
        mDcSettings = null;

        mImsSupportedAccessNetworks = Collections.emptyList();
        mVoiceRoamingType = 0;
        mDataRoamingType = 0;
        mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mVoiceNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mVoiceServiceState = ServiceState.STATE_OUT_OF_SERVICE;
        mDataServiceState = ServiceState.STATE_OUT_OF_SERVICE;
        mNetworkRegistrationState =
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
        mNetworkRegistrationRejectCause = REGISTRATION_REJECT_CAUSE_NONE;
        mNetworkOperator = "";
        mDataRoaming = false;
        mVoiceRoaming = false;
        mDataNetworkRoaming = false;
        mAirplaneMode = false;
        mTelephonyNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mTelephonyVoiceNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mImsVopsState = ImsEventDef.IMS_VOICE_OVER_PS_INVALID;
        mEmcbs = false;
        mLteAttachResultType = ImsEventDef.IMS_LTE_INFO_UNKNOWN;
        mLteAttachExtraInfo = ImsEventDef.IMS_LTE_INFO_EXTRA_NONE;
    }

    @Override
    public int getCellularDataServiceState() {
        return mCellularDataServiceState;
    }

    // Data service state in ServiceState
    @Override
    public int getDataServiceState() {
        return mDataServiceState;
    }

    @Override
    public @NetworkType int getNetworkType() {
        return mNetworkType;
    }

    @Override
    public @NetworkType int getVoiceNetworkType() {
        return mVoiceNetworkType;
    }

    @Override
    // Voice service state in ServiceState
    public int getVoiceServiceState() {
        return mVoiceServiceState;
    }

    @Override
    public int getMocnPlmnInfo() {
        // TODO : update MOCN PLMN info
        return 0;
    }

    @Override
    public String getNetworkOperator() {
        return mNetworkOperator;
    }

    @Override
    public int getNetworkRegistrationRejectCause() {
        return mNetworkRegistrationRejectCause;
    }

    @Override
    public void clearNetworkRegistrationRejectCause() {
        mNetworkRegistrationRejectCause = REGISTRATION_REJECT_CAUSE_NONE;
    }

    @Override
    public boolean isAirplaneMode() {
        return mAirplaneMode;
    }

    @Override
    public boolean isEmergencyOnly() {
        return mNetworkRegistrationState == NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY;
    }

    @Override
    public boolean isEmergencyServiceSupported() {
        return mEmcbs;
    }

    @Override
    public boolean isRoaming() {
        return mDataRoaming || mVoiceRoaming;
    }

    @Override
    public boolean isVoiceRoaming() {
        return mVoiceRoaming;
    }

    @Override
    public boolean isDataNetworkRoaming() {
        return mDataNetworkRoaming;
    }

    @Override
    public int getVoiceRoamingType() {
        return mVoiceRoamingType;
    }

    @Override
    public int getDataRoamingType() {
        return mDataRoamingType;
    }

    @Override
    public boolean is3G() {
        return getAccessNetwork(mNetworkType) == AccessNetworkConstants.AccessNetworkType.UTRAN;
    }

    @Override
    public boolean is4G() {
        return getAccessNetwork(mNetworkType) == AccessNetworkConstants.AccessNetworkType.EUTRAN;
    }

    @Override
    public boolean is5G() {
        return getAccessNetwork(mNetworkType) == AccessNetworkConstants.AccessNetworkType.NGRAN;
    }

    @Override
    public @AccessNetworkConstants.RadioAccessNetworkType int getAccessNetworkType() {
        return getAccessNetwork(mNetworkType);
    }

    @Override
    public boolean isImsSupportedNetworkType(@NetworkType int networkType) {
        return mImsSupportedAccessNetworks.contains(getAccessNetwork(networkType));
    }

    @Override
    public boolean isVopsSupported() {
        IDcSettings dcSettings = DcFactory.getDcAgent(IDcSettings.class, mSlotId);
        if (dcSettings == null || dcSettings.isVopsIgnored()) {
            return true;
        } else {
            return mImsVopsState != ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED;
        }
    }

    @Override
    public void updateTelephonyNetworkType(@NetworkType int networkType) {
        mTelephonyNetworkType = networkType;
    }

    @Override
    public void updateTelephonyVoiceNetworkType(@NetworkType int networkType) {
        mTelephonyVoiceNetworkType = networkType;
    }

    @Override
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    @Override
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    @Override
    public void notifyDataConnectionState(EApnType apnType, EDataState dataState) {
        for (Listener l : mListeners) {
            l.onDataConnectionStateChanged(apnType, dataState);
        }
    }

    @Override
    public void notifyPdnConnectionFailed(EApnType apnType, int smCause) {
        for (Listener l : mListeners) {
            l.onPdnConnectionFailed(apnType, smCause);
        }
    }

    @SuppressWarnings("deprecation")
    private static int getInternalAccessNetworkType(@NetworkType int networkType) {
        final int accessNetwork = getAccessNetwork(networkType);
        return switch (accessNetwork) {
            case AccessNetworkConstants.AccessNetworkType.GERAN -> AN_GEREAN;
            case AccessNetworkConstants.AccessNetworkType.UTRAN -> AN_UTRAN;
            case AccessNetworkConstants.AccessNetworkType.EUTRAN -> AN_EUTRAN;
            case AccessNetworkConstants.AccessNetworkType.NGRAN -> AN_NGRAN;
            case AccessNetworkConstants.AccessNetworkType.CDMA2000 -> switch (networkType) {
                case TelephonyManager.NETWORK_TYPE_EHRPD -> AN_EHRPD;
                case TelephonyManager.NETWORK_TYPE_CDMA,
                     TelephonyManager.NETWORK_TYPE_1xRTT -> AN_1XRTT;
                case TelephonyManager.NETWORK_TYPE_EVDO_0,
                     TelephonyManager.NETWORK_TYPE_EVDO_A,
                     TelephonyManager.NETWORK_TYPE_EVDO_B -> AN_EVDO;
                default -> AN_UNKNOWN;
            };
            default -> // UNKNOWN, IWLAN
                    AN_UNKNOWN;
        };
    }

    private int getAvailableInternalAccessNetworkType(@NetworkType int networkType) {
        return isImsSupportedNetworkType(networkType)
                ? getInternalAccessNetworkType(networkType) : AN_UNKNOWN;
    }

    @SuppressWarnings("deprecation")
    private static @AccessNetworkConstants.RadioAccessNetworkType int getAccessNetwork(
            @NetworkType int networkType) {
        return switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_GPRS,
                 TelephonyManager.NETWORK_TYPE_EDGE,
                 TelephonyManager.NETWORK_TYPE_GSM
                    -> AccessNetworkConstants.AccessNetworkType.GERAN;
            case TelephonyManager.NETWORK_TYPE_UMTS,
                 TelephonyManager.NETWORK_TYPE_HSDPA,
                 TelephonyManager.NETWORK_TYPE_HSUPA,
                 TelephonyManager.NETWORK_TYPE_HSPAP,
                 TelephonyManager.NETWORK_TYPE_HSPA,
                 TelephonyManager.NETWORK_TYPE_TD_SCDMA
                    -> AccessNetworkConstants.AccessNetworkType.UTRAN;
            case TelephonyManager.NETWORK_TYPE_LTE,
                 TelephonyManager.NETWORK_TYPE_LTE_CA
                    -> AccessNetworkConstants.AccessNetworkType.EUTRAN;
            case TelephonyManager.NETWORK_TYPE_NR
                    -> AccessNetworkConstants.AccessNetworkType.NGRAN;
            case TelephonyManager.NETWORK_TYPE_IWLAN
                    -> AccessNetworkConstants.AccessNetworkType.IWLAN;
            case TelephonyManager.NETWORK_TYPE_EHRPD,
                 TelephonyManager.NETWORK_TYPE_CDMA,
                 TelephonyManager.NETWORK_TYPE_1xRTT,
                 TelephonyManager.NETWORK_TYPE_EVDO_0,
                 TelephonyManager.NETWORK_TYPE_EVDO_A,
                 TelephonyManager.NETWORK_TYPE_EVDO_B
                    -> AccessNetworkConstants.AccessNetworkType.CDMA2000;
            default -> AccessNetworkConstants.AccessNetworkType.UNKNOWN;
        };
    }

    private void loadImsSupportedAccessNetworks() {
        int[] supportedNetworks = (mDcSettings != null)
                ? mDcSettings.getImsSupportedAccessNetworks()
                : DEFAULT_IMS_SUPPORTED_NETWORKS;

        if (supportedNetworks.length == 0) {
            supportedNetworks = DEFAULT_IMS_SUPPORTED_NETWORKS;
        }
        mImsSupportedAccessNetworks = IntStream.of(supportedNetworks).boxed().toList();
    }

    private static boolean getDataRoaming(ServiceState ss) {
        return getDataRoamingType(ss) != ServiceState.ROAMING_TYPE_NOT_ROAMING;
    }

    private static int getDataRoamingType(ServiceState ss) {
        NetworkRegistrationInfo regState =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regState != null) {
            return regState.getRoamingType();
        }

        return ServiceState.ROAMING_TYPE_NOT_ROAMING;
    }

    private static int getVoiceRoamingType(ServiceState ss) {
        NetworkRegistrationInfo regState =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_CS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regState != null) {
            return regState.getRoamingType();
        }

        return ServiceState.ROAMING_TYPE_NOT_ROAMING;
    }

    private static boolean getVoiceRoaming(ServiceState ss) {
        NetworkRegistrationInfo regState =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_CS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regState != null) {
            return regState.getRoamingType() != ServiceState.ROAMING_TYPE_NOT_ROAMING;
        }

        return false;
    }

    private static boolean getDataNetworkRoaming(ServiceState ss) {
        NetworkRegistrationInfo regState =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regState != null) {
            return regState.isNetworkRoaming();
        }

        return false;
    }

    private static int getAccessNetworkTechnology(ServiceState ss) {
        NetworkRegistrationInfo nri =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_CS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (nri != null) {
            return nri.getAccessNetworkTechnology();
        }

        return TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private static int getCellularDataServiceState(ServiceState ss) {
        final NetworkRegistrationInfo wwanRegInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        int nriState = (wwanRegInfo != null)
                ? wwanRegInfo.getNetworkRegistrationState()
                : NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
        return nriStateToServiceState(nriState, ss, wwanRegInfo);
    }

    private static int getDataServiceState(ServiceState ss) {
        final NetworkRegistrationInfo iwlanRegInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WLAN);
        final NetworkRegistrationInfo wwanRegInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        int nriState = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;

        if (iwlanRegInfo == null || !iwlanRegInfo.isNetworkRegistered()) {
            nriState =
                    (wwanRegInfo != null)
                            ? wwanRegInfo.getNetworkRegistrationState()
                            : NetworkRegistrationInfo
                                    .REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
        } else if (wwanRegInfo != null && !wwanRegInfo.isNetworkRegistered()) {
            nriState = iwlanRegInfo.getNetworkRegistrationState();
        } else if (wwanRegInfo != null) {
            nriState = wwanRegInfo.getNetworkRegistrationState();
        }

        return nriStateToServiceState(nriState, ss, wwanRegInfo);
    }

    private static int nriStateToServiceState(
            int nriState, ServiceState ss, NetworkRegistrationInfo wwanRegInfo) {
        int retState = ServiceState.STATE_OUT_OF_SERVICE;

        if (ss.getState() == ServiceState.STATE_POWER_OFF) {
            retState = ServiceState.STATE_POWER_OFF;
        } else if (nriState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME
                || nriState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING) {
            retState = ServiceState.STATE_IN_SERVICE;
        } else if (wwanRegInfo != null && wwanRegInfo.isEmergencyEnabled()) {
            retState = ServiceState.STATE_EMERGENCY_ONLY;
        }

        return retState;
    }

    private static int getLteAttachExtraInfo(int extraInfo) {
        int imsExtraInfo = ImsEventDef.IMS_LTE_INFO_EXTRA_NONE;

        if ((extraInfo
                & DataSpecificRegistrationInfo.LTE_ATTACH_EXTRA_INFO_CSFB_NOT_PREFERRED) > 0) {
            imsExtraInfo |= ImsEventDef.IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED;
        }

        if ((extraInfo & DataSpecificRegistrationInfo.LTE_ATTACH_EXTRA_INFO_SMS_ONLY) > 0) {
            imsExtraInfo |= ImsEventDef.IMS_LTE_INFO_EXTRA_SMS_ONLY;
        }

        return imsExtraInfo;
    }

    private static int getLteAttachResultType(int attachResult) {
        if (attachResult == DataSpecificRegistrationInfo.LTE_ATTACH_TYPE_COMBINED) {
            return ImsEventDef.IMS_LTE_INFO_COMBINED_ATTACHED;
        }

        return ImsEventDef.IMS_LTE_INFO_EPS_ONLY_ATTACHED;
    }

    private static int getWwanNetworkRegistrationState(ServiceState ss) {
        final NetworkRegistrationInfo wwanRegInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        return (wwanRegInfo != null) ? wwanRegInfo.getNetworkRegistrationState()
                : NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    }

    private static int getWwanNetworkRegistrationRejectCause(ServiceState ss) {
        final NetworkRegistrationInfo wwanRegInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        return (wwanRegInfo != null)
                ? wwanRegInfo.getRejectCause() : REGISTRATION_REJECT_CAUSE_NONE;
    }

    private int getCurrentTelephonyNetworkType() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);

        return (telephony != null)
                ? telephony.getNetworkType()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private boolean updateVoiceServiceState(ServiceState serviceState) {
        int voiceServiceState = serviceState.getState();

        if (mVoiceServiceState != voiceServiceState) {
            mVoiceServiceState = voiceServiceState;
            return true;
        }

        return false;
    }

    private boolean updateNetworkOperator(ServiceState serviceState) {
        String operatorNumeric = serviceState.getOperatorNumeric();

        if (operatorNumeric == null) {
            mNetworkOperator = "";
        } else if (!mNetworkOperator.equals(operatorNumeric)) {
            if (getDataServiceState(serviceState) == ServiceState.STATE_IN_SERVICE) {
                mNetworkOperator = operatorNumeric;
                return true;
            }
        }

        return false;
    }

    private boolean updateDataServiceState(ServiceState serviceState) {
        int dataServiceState = getDataServiceState(serviceState);

        mCellularDataServiceState = getCellularDataServiceState(serviceState);
        if (mDataServiceState != dataServiceState) {
            mDataServiceState = dataServiceState;
            return true;
        }

        return false;
    }

    private boolean updateNetworkType() {
        int networkType = getCurrentTelephonyNetworkType();

        if (mNetworkType != networkType || getNetworkType() != mTelephonyNetworkType) {
            mNetworkType = networkType;
            return true;
        }

        return false;
    }

    private boolean updateVoiceNetworkType(ServiceState serviceState) {
        int networkType = getAccessNetworkTechnology(serviceState);

        if (mVoiceNetworkType != networkType
                || getVoiceNetworkType() != mTelephonyVoiceNetworkType) {
            mVoiceNetworkType = networkType;
            return true;
        }

        return false;
    }

    private boolean updateRoamingState(ServiceState serviceState) {
        boolean roaming = isRoaming();
        boolean isVoiceRoamingChanged = updateVoiceRoamingState(serviceState);
        boolean isDataRoamingChanged = updateDataRoamingState(serviceState);
        mRoamingChanged = roaming != isRoaming();

        return isVoiceRoamingChanged || isDataRoamingChanged;
    }

    private boolean updateVoiceRoamingState(ServiceState serviceState) {
        boolean isUpdated = false;
        boolean isVoiceRoaming = getVoiceRoaming(serviceState);
        int voiceRoamingType = getVoiceRoamingType(serviceState);

        if (mVoiceRoaming != isVoiceRoaming) {
            mVoiceRoaming = isVoiceRoaming;
            isUpdated = true;
        }

        if (mVoiceRoamingType != voiceRoamingType) {
            mVoiceRoamingType = voiceRoamingType;
        }

        return isUpdated;
    }

    private boolean updateDataRoamingState(ServiceState serviceState) {
        boolean isUpdated = false;
        boolean isDataRoaming = getDataRoaming(serviceState);
        int dataRoamingType = getDataRoamingType(serviceState);

        if (mDataRoaming != isDataRoaming) {
            mDataRoaming = isDataRoaming;
            isUpdated = true;
        }

        if (mDataRoamingType != dataRoamingType) {
            mDataRoamingType = dataRoamingType;
        }

        mDataNetworkRoaming = getDataNetworkRoaming(serviceState);

        return isUpdated;
    }

    private boolean updateVopsState(ServiceState serviceState) {
        if (!is4G() && !is5G()) {
            mImsVopsState = ImsEventDef.IMS_VOICE_OVER_PS_INVALID;
            return false;
        }

        NetworkRegistrationInfo regInfo =
                serviceState.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regInfo == null) {
            return false;
        }

        DataSpecificRegistrationInfo dsrInfo = regInfo.getDataSpecificInfo();
        if (dsrInfo == null) {
            return false;
        }

        VopsSupportInfo vsi = dsrInfo.getVopsSupportInfo();
        if (vsi == null) {
            return false;
        }

        // Emergency service
        boolean emcbs = vsi.isEmergencyServiceSupported();
        if (mEmcbs != emcbs) {
            ImsLog.w(mSlotId, "update emergency service supported info : " + emcbs);
            mEmcbs = emcbs;
        }

        // VoPS
        if (!regInfo.isNetworkRegistered()) {
            mImsVopsState = ImsEventDef.IMS_VOICE_OVER_PS_INVALID;
            return false;
        }

        int newVopsState =
                (vsi.isVopsSupported() || mDcSettings == null || mDcSettings.isVopsIgnored())
                ? ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED
                : ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED;
        if (mImsVopsState != newVopsState || !mImsVopsPlmn.equals(mNetworkOperator)) {
            ImsLog.d(mSlotId, "VoPS support indication is updated to ["
                    + (newVopsState == ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED
                            ? "SUPPORTED" : "NOT_SUPPORTED") + "] on PLMN(" + mNetworkOperator
                    + ")");

            mImsVopsState = newVopsState;
            mImsVopsPlmn = mNetworkOperator;

            return true;
        }

        return false;
    }

    private boolean updateLteInfo(ServiceState serviceState) {
        // LTE attach result and extra info
        NetworkRegistrationInfo regInfo =
                serviceState.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regInfo == null) {
            return false;
        }

        DataSpecificRegistrationInfo dsrInfo = regInfo.getDataSpecificInfo();
        if (dsrInfo == null) {
            return false;
        }

        int lteAttachResultType = getLteAttachResultType(dsrInfo.getLteAttachResultType());
        int lteAttachExtraInfo = getLteAttachExtraInfo(dsrInfo.getLteAttachExtraInfo());

        if (mLteAttachResultType != lteAttachResultType
                || mLteAttachExtraInfo != lteAttachExtraInfo) {
            ImsLog.d(mSlotId, "lte attach result = " + lteAttachResultType
                    + ", extra = " + lteAttachExtraInfo);
            mLteAttachResultType = lteAttachResultType;
            mLteAttachExtraInfo = lteAttachExtraInfo;

            return true;
        }

        return false;
    }

    private boolean updateNetworkRegistrationState(ServiceState serviceState) {
        boolean isUpdated = false;

        // WWAN network registration state
        int networkRegistrationState = getWwanNetworkRegistrationState(serviceState);
        if (networkRegistrationState != mNetworkRegistrationState) {
            mNetworkRegistrationState = networkRegistrationState;
            isUpdated = true;
        }

        // WWAN network registration reject cause
        if (mNetworkRegistrationState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME
                || mNetworkRegistrationState
                        == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING) {
            mNetworkRegistrationRejectCause = REGISTRATION_REJECT_CAUSE_NONE;
        } else {
            int rejectCause = getWwanNetworkRegistrationRejectCause(serviceState);
            if (rejectCause != REGISTRATION_REJECT_CAUSE_NONE) {
                mNetworkRegistrationRejectCause = rejectCause;
            }
        }

        return isUpdated;
    }

    private void notifyNetworkOperatorChanged() {
        for (Listener l : mListeners) {
            l.onNetworkOperatorChanged(mNetworkOperator);
        }
    }

    private void notifyDataServiceStateChanged() {
        for (Listener l : mListeners) {
            l.onDataServiceStateChanged(mDataServiceState);
        }
        mSystem.notifyServiceStateChanged(mDataServiceState);
    }

    private void notifyVoiceServiceStateChanged() {
        mSystem.notifyEvent(ImsEventDef.IMS_EVENT_VOICE_SERVICE_STATE, mVoiceServiceState, 0);
    }

    private void notifyNetworkTypeChanged() {
        for (Listener l : mListeners) {
            l.onDataNetworkTypeChanged();
        }
        mSystem.notifyNetworkTypeChanged(getAvailableInternalAccessNetworkType(mNetworkType));
    }

    private void notifyVoiceNetworkTypeChanged() {
        for (Listener l : mListeners) {
            l.onVoiceNetworkTypeChanged();
        }
        mSystem.notifyVoiceNetworkTypeChanged(getInternalAccessNetworkType(mVoiceNetworkType));
    }

    private void notifyRoamingState(boolean dataRoaming, boolean voiceRoaming) {
        if (mRoamingChanged) {
            for (Listener l : mListeners) {
                l.onRoamingStateChanged(isRoaming());
            }
            mRoamingChanged = false;
        }

        int psRoamingState =
                (dataRoaming)
                        ? ImsEventDef.IMS_ROAMING_STATE_ON
                        : ImsEventDef.IMS_ROAMING_STATE_OFF;
        int csRoamingState =
                (voiceRoaming)
                        ? ImsEventDef.IMS_ROAMING_STATE_ON
                        : ImsEventDef.IMS_ROAMING_STATE_OFF;
        mSystem.notifyEvent(ImsEventDef.IMS_EVENT_ROAMING_STATE, psRoamingState, csRoamingState);
    }

    private void notifyImsNetworkVopsChanged() {
        if (mImsVopsState == ImsEventDef.IMS_VOICE_OVER_PS_INVALID) {
            return;
        }

        mSystem.notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE, mImsVopsState, 0);

        for (Listener l : mListeners) {
            l.onVopsStateChanged(mImsVopsState, mImsVopsPlmn);
        }
    }

    private void notifyLteInfoChanged() {
        mSystem.notifyEvent(ImsEventDef.IMS_EVENT_LTE_INFO, mLteAttachResultType,
                mLteAttachExtraInfo);
    }

    private void notifyNetworkRegistrationStateChanged() {
        for (Listener l : mListeners) {
            l.onNetworkRegistrationStateChanged(mNetworkRegistrationState);
        }
    }

    private static int getAirplaneMode() {
        return AppContext.getInstance().getContentProviderProxy().getGlobalSettings()
                .getInt(AIRPLANE_MODE_ON, -1);
    }

    /** This class is for receiving the airplane mode intent */
    public class DcNetWatcherReceiver extends BroadcastReceiver {
        /** Registers the broadcast receiver. */
        public void register() {
            IntentFilter filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
            AppContext.getInstance().getBroadcastReceiverProxy().registerReceiver(this, filter);
        }

        /** Unregisters the broadcast receiver. */
        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(mSlotId, ImsLog.lastSubString(action, "."));

            if (mDcNetWatcherHandler == null) {
                return;
            }

            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                Message.obtain(mDcNetWatcherHandler, EVENT_AIRPLANE_MODE_CHANGED, intent)
                        .sendToTarget();
            }
        }
    }

    private class DcNetWatcherConfigListener implements ConfigInterface.Listener {
        @Override
        public void onCarrierConfigChanged(int slotId, int subId) {
            ImsLog.i(mSlotId, "carrier config is changed");
            loadImsSupportedAccessNetworks();
        }
    }

    private final class DcNetWatcherNativeStateListener implements NativeStateInterface.Listener {
        @Override
        public void onNativeServiceReady() {
            ImsLog.i(mSlotId, "NativeState: service ready.");

            for (Listener l : mListeners) {
                l.onAirplaneModeChanged(mAirplaneMode);
            }

            if (mSystem != null) {
                mSystem.notifyAirplaneModeChanged(mAirplaneMode ? 1 : 0);
            }

            // roaming state
            notifyRoamingState(mDataRoaming, mVoiceRoaming);

            notifyVoiceServiceStateChanged();
            mSystem.notifyEvent(ImsEventDef.IMS_EVENT_LTE_INFO, mLteAttachResultType,
                    mLteAttachExtraInfo);
        }
    }

    private class DcNetWatcherPhoneStateListener implements ImsPhoneStateListener {
        private enum NetworkServiceState {
            VOICE_SERVICE_STATE,
            NETWORK_OPERATOR,
            DATA_SERVICE_STATE,
            DATA_NETWORK_TYPE,
            VOICE_NETWORK_TYPE,
            ROAMING_STATE,
            VOPS_STATE,
            LTE_INFO,
            NETWORK_REGISTRATION_STATE
        }

        private IPhoneStateNotifier mNotifier;
        private PhoneStateInterface mPhoneState;

        DcNetWatcherPhoneStateListener() {}

        public void dispose() {
            if (mNotifier != null) {
                if (mPhoneState != null) {
                    mPhoneState.removeNotifier(mNotifier);
                }

                mNotifier.setListener(null);
                mNotifier = null;
                mPhoneState = null;
            }
        }

        public void setListener() {
            mPhoneState = AgentFactory.getInstance().getAgent(PhoneStateInterface.class, mSlotId);

            if (mPhoneState != null) {
                mNotifier = mPhoneState.createNotifier(this, mDcNetWatcherHandler.getLooper());
                mNotifier.setEvents(
                        LISTEN_CALL_STATE | LISTEN_SERVICE_STATE | LISTEN_PRECISE_CALL_STATE);

                mPhoneState.addNotifier(mNotifier);
            }
        }

        /** Invokes when service state is changed. */
        @Override
        public void onServiceStateChanged(@NonNull ServiceState serviceState) {
            Set<NetworkServiceState> changedStates = EnumSet.noneOf(NetworkServiceState.class);

            if (updateVoiceServiceState(serviceState)) {
                changedStates.add(NetworkServiceState.VOICE_SERVICE_STATE);
            }

            if (updateNetworkOperator(serviceState)) {
                changedStates.add(NetworkServiceState.NETWORK_OPERATOR);
            }

            if (updateDataServiceState(serviceState)) {
                changedStates.add(NetworkServiceState.DATA_SERVICE_STATE);
            }

            if (updateNetworkType()) {
                changedStates.add(NetworkServiceState.DATA_NETWORK_TYPE);
            }

            if (updateVoiceNetworkType(serviceState)) {
                changedStates.add(NetworkServiceState.VOICE_NETWORK_TYPE);
            }

            if (updateRoamingState(serviceState)) {
                changedStates.add(NetworkServiceState.ROAMING_STATE);
            }

            if (updateVopsState(serviceState)) {
                changedStates.add(NetworkServiceState.VOPS_STATE);
            }

            if (is4G()) {
                if (updateLteInfo(serviceState)) {
                    changedStates.add(NetworkServiceState.LTE_INFO);
                }
            }

            if (updateNetworkRegistrationState(serviceState)) {
                changedStates.add(NetworkServiceState.NETWORK_REGISTRATION_STATE);
            }

            if (!changedStates.isEmpty()) {
                notifyServiceStateChanged(changedStates);
            }
        }

        private void notifyServiceStateChanged(Set<NetworkServiceState> changedStates) {
            if (changedStates.contains(NetworkServiceState.VOICE_SERVICE_STATE)) {
                notifyVoiceServiceStateChanged();
            }

            if (changedStates.contains(NetworkServiceState.NETWORK_OPERATOR)) {
                notifyNetworkOperatorChanged();
            }

            if (changedStates.contains(NetworkServiceState.DATA_SERVICE_STATE)) {
                notifyDataServiceStateChanged();
            }

            if (changedStates.contains(NetworkServiceState.DATA_NETWORK_TYPE)) {
                notifyNetworkTypeChanged();
            }

            if (changedStates.contains(NetworkServiceState.VOICE_NETWORK_TYPE)) {
                notifyVoiceNetworkTypeChanged();
            }

            if (changedStates.contains(NetworkServiceState.ROAMING_STATE)) {
                notifyRoamingState(mDataRoaming, mVoiceRoaming);
            }

            if (changedStates.contains(NetworkServiceState.VOPS_STATE)) {
                notifyImsNetworkVopsChanged();
            }

            if (changedStates.contains(NetworkServiceState.LTE_INFO)) {
                notifyLteInfoChanged();
            }

            if (changedStates.contains(NetworkServiceState.NETWORK_REGISTRATION_STATE)) {
                notifyNetworkRegistrationStateChanged();
            }
        }
    }

    // -----------------------------------------------------------
    private class DcNetWatcherHandler extends Handler {
        DcNetWatcherHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(@NonNull Message msg) {
            Objects.requireNonNull(msg, "msg must not be null");
            ImsLog.i(mSlotId, "Message(" + msg.what + ")");

            if (msg.what == EVENT_AIRPLANE_MODE_CHANGED) {
                handleAirplaneModeChanged(msg);
            }
        }

        private void handleAirplaneModeChanged(Message msg) {
            Intent intent = (Intent) msg.obj;

            if (intent == null) {
                return;
            }

            boolean state = intent.getBooleanExtra("state", false);
            if (mAirplaneMode != state) {
                if (mContext == null) {
                    return;
                }

                int stateFromIntent = state ? 1 : 0;
                int stateFromSettings = getAirplaneMode();

                ImsLog.i(
                        mSlotId,
                        "AirplaneMode :: settings-db="
                                + stateFromSettings
                                + ", intent="
                                + stateFromIntent
                                + "-"
                                + ((stateFromIntent == 0) ? "OFF" : "ON"));

                if ((stateFromSettings != 0) && (stateFromSettings != 1)) {
                    ImsLog.w(
                            mSlotId,
                            "AirplaneMode :: settings("
                                    + stateFromSettings
                                    + ") is not valid; "
                                    + "fallback to the default value(0)");
                    stateFromSettings = 0;
                }

                if (stateFromIntent != stateFromSettings) {
                    ImsLog.w(
                            mSlotId,
                            "AirplaneMode :: state (intent="
                                    + state
                                    + ", settings="
                                    + stateFromSettings
                                    + ") is not matched; ignored...");
                    return;
                }

                mAirplaneMode = state;

                if (mSystem != null) {
                    mSystem.notifyAirplaneModeChanged(mAirplaneMode ? 1 : 0);
                }

                for (Listener l : mListeners) {
                    l.onAirplaneModeChanged(mAirplaneMode);
                }
            }
        }
    }
}

