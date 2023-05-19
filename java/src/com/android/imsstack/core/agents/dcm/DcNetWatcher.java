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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.provider.Settings;
import android.telephony.AccessNetworkConstants;
import android.telephony.Annotation.CallState;
import android.telephony.DataSpecificRegistrationInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.PreciseCallState;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.VopsSupportInfo;

import com.android.imsstack.core.CapabilityConfigs;
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
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/** This class is for providing the network information */
public class DcNetWatcher implements IDcNetWatcher {
    // Constants--------------------------------------------------
    // RAT for IMS adaptation layer
    public static final int RAT_EHRPD = 1;
    public static final int RAT_2G = 2;
    public static final int RAT_3G = 3;
    public static final int RAT_4G = 4;
    public static final int RAT_5G = 5;
    public static final int RAT_1XRTT = 6; // will be removed
    public static final int RAT_EVDO = 7; // will be removed

    // Policy for RAT
    public static final int POLICY_RAT_1XRTT = 0x00000001;
    public static final int POLICY_RAT_EHRPD = 0x00000002;
    public static final int POLICY_RAT_3G = 0x00000004;
    public static final int POLICY_RAT_4G = 0x00000008;
    public static final int POLICY_RAT_2G = 0x00000010;
    public static final int POLICY_RAT_EVDO = 0x00000020;
    public static final int POLICY_RAT_5G = 0x00000040;
    public static final int POLICY_RAT_WLAN = 0x00000080;

    public static final int EVENT_AIRPLANE_MODE_CHANGED = 2001;
    public static final int EVENT_VOLTE_LTE_STATE_INFO = 2002;
    public static final int EVENT_NR_REGISTRATION_INFO = 2006;

    // Variables--------------------------------------------------
    private Context mContext;

    private IDcSettings mDcSettings;

    private DcNetWatcherReceiver mDcNetWatcherReceiver = null;
    @VisibleForTesting
    protected Handler mDcNetWatcherHandler;
    @VisibleForTesting
    protected DcNetWatcherPhoneStateListener mPhoneStateListener = null;
    protected DcNetWatcherConfigListener mConfigListener = null;
    private DcNetWatcherNativeStateListener mNativeStateListener;

    private RegistrantList mStateDataConnectionState = new RegistrantList();
    private RegistrantList mDataServiceStateChangedRegistrants = new RegistrantList();
    private RegistrantList mRatChangedRegistrants = new RegistrantList();
    private RegistrantList mVoiceRatChangedRegistrants = new RegistrantList();
    private RegistrantList mRoamingStateChangedRegistrants = new RegistrantList();
    private RegistrantList mVoiceRoamingStateChangedRegistrants = new RegistrantList();
    private RegistrantList mAirplaneModeChangedRegistrants = new RegistrantList();
    private RegistrantList mNetworkOperatorChangedRegistrants = new RegistrantList();
    private RegistrantList mCsCallStatusChangedRegistrants = new RegistrantList();
    private RegistrantList mPreciseCsCallStatusChangedRegistrants = new RegistrantList();
    private RegistrantList mImsVopsChangedRegistrants = new RegistrantList();
    private RegistrantList mPowerOffChangedRegistrants = new RegistrantList();
    private RegistrantList mPdnConnectionFailedRegistrants = new RegistrantList();
    private RegistrantList mVoiceRoamingTypeChangedRegistrants = new RegistrantList();
    private RegistrantList mDataRoamingTypeChangedRegistrants = new RegistrantList();

    private int mRatPolicy = 0;
    private int mCallState = TelephonyManager.CALL_STATE_IDLE;
    private int mPreciseCallState = PreciseCallState.PRECISE_CALL_STATE_IDLE;
    private int mVoiceRoamingType = 0;
    private int mDataRoamingType = 0;

    private int mRat = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    private int mVoiceRat = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    private int mVoiceServiceState = ServiceState.STATE_OUT_OF_SERVICE;
    private int mDataServiceState = ServiceState.STATE_OUT_OF_SERVICE;
    private int mLteDuplexMode = ServiceState.DUPLEX_MODE_UNKNOWN;
    private String mNetworkOperator = "";
    // mDataRoaming and mVoiceRoaming refer to the roamingType.
    // So they could be overridden by the carrier config
    private boolean mDataRoaming = false;
    private boolean mVoiceRoaming = false;
    // data network roaming state that was not overridden by any carrier config
    private boolean mDataNetworkRoaming = false;
    private boolean mAirplaneMode = false;

    // RAT in TelephonyManager for sync with ServiceState
    @VisibleForTesting
    protected int mRatFromTm = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    @VisibleForTesting
    protected int mVoiceRatFromTm = TelephonyManager.NETWORK_TYPE_UNKNOWN;

    // IMS voice over PS Session Supported
    private boolean mImsVops = false;
    // Emergency bearer support capability
    private boolean mEmcbs = false;

    private boolean mDoingOffRadio = false;

    private int mLteAttachResultType = ImsEventDef.IMS_LTE_INFO_UNKNOWN;
    private int mLteAttachExtraInfo = ImsEventDef.IMS_LTE_INFO_EXTRA_NONE;
    private int mNrRegistrationInfo = ImsEventDef.IMS_NR_INFO_UNKNOWN;

    private ISystem mSystem;
    private IAosInfo mAosInfo;
    private int mSlotId = 0;

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
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

        mDcSettings = (IDcSettings) DcFactory.getDc(DcFactory.SETTING, mSlotId);
        if (mDcSettings == null) {
            ImsLog.w(mSlotId, "IDcSettings is null");
        }

        setRatPolicy();
        setAirplaneMode();

        mDcNetWatcherHandler = new DcNetWatcherHandler(Looper.myLooper());

        mSystem = SystemInterface.getInstance().getSystem(mSlotId);
        if (mSystem == null) {
            return;
        }

        mAosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (mAosInfo == null) {
            return;
        }

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        if (nsi != null) {
            mNativeStateListener = new DcNetWatcherNativeStateListener();
            nsi.addListener(mNativeStateListener);
        }

        mDcNetWatcherReceiver = new DcNetWatcherReceiver();
        mContext.registerReceiver(
                mDcNetWatcherReceiver,
                mDcNetWatcherReceiver.getFilter(),
                Context.RECEIVER_EXPORTED);

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

        if (mDcNetWatcherReceiver != null && mContext != null) {
            mContext.unregisterReceiver(mDcNetWatcherReceiver);
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

        mAosInfo = null;
        mSystem = null;
        mDcSettings = null;

        mRatPolicy = 0;
        mCallState = TelephonyManager.CALL_STATE_IDLE;
        mPreciseCallState = PreciseCallState.PRECISE_CALL_STATE_IDLE;
        mVoiceRoamingType = 0;
        mDataRoamingType = 0;
        mRat = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mVoiceRat = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mVoiceServiceState = ServiceState.STATE_OUT_OF_SERVICE;
        mDataServiceState = ServiceState.STATE_OUT_OF_SERVICE;
        mLteDuplexMode = ServiceState.DUPLEX_MODE_UNKNOWN;
        mNetworkOperator = "";
        mDataRoaming = false;
        mVoiceRoaming = false;
        mDataNetworkRoaming = false;
        mAirplaneMode = false;
        mRatFromTm = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mVoiceRatFromTm = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mImsVops = false;
        mEmcbs = false;
        mDoingOffRadio = false;
        mLteAttachResultType = ImsEventDef.IMS_LTE_INFO_UNKNOWN;
        mLteAttachExtraInfo = ImsEventDef.IMS_LTE_INFO_EXTRA_NONE;
        mNrRegistrationInfo = ImsEventDef.IMS_NR_INFO_UNKNOWN;
    }

    @Override
    public boolean isRatPolicyAvailable() {
        if (((mRatPolicy & POLICY_RAT_5G) != 0) && is5G()) {
            ImsLog.i(
                    mSlotId,
                    "mRatPolicy : "
                            + Integer.toHexString(mRatPolicy).toUpperCase()
                            + ", mRat : "
                            + mRat
                            + ", ServiceState : 5G");
            return true;
        }

        if (((mRatPolicy & POLICY_RAT_4G) != 0) && is4G()) {
            ImsLog.i(
                    mSlotId,
                    "mRatPolicy : "
                            + Integer.toHexString(mRatPolicy).toUpperCase()
                            + ", mRat : "
                            + mRat
                            + ", ServiceState : 4G");
            return true;
        }

        if (((mRatPolicy & POLICY_RAT_3G) != 0) && is3G()) {
            ImsLog.i(
                    mSlotId,
                    "mRatPolicy : "
                            + Integer.toHexString(mRatPolicy).toUpperCase()
                            + ", mRat : "
                            + mRat
                            + ", ServiceState : 3G");
            return true;
        }

        if (((mRatPolicy & POLICY_RAT_EHRPD) != 0) && isEhrpd()) {
            ImsLog.i(
                    mSlotId,
                    "mRatPolicy : "
                            + Integer.toHexString(mRatPolicy).toUpperCase()
                            + ", mRat : "
                            + mRat
                            + ", ServiceState : eHRPD");
            return true;
        }

        if (((mRatPolicy & POLICY_RAT_2G) != 0) && is2G()) {
            ImsLog.i(
                    mSlotId,
                    "mRatPolicy : "
                            + Integer.toHexString(mRatPolicy).toUpperCase()
                            + ", mRat : "
                            + mRat
                            + ", ServiceState : 2G");
            return true;
        }

        if (((mRatPolicy & POLICY_RAT_EVDO) != 0) && isEvdo()) {
            ImsLog.i(
                    mSlotId,
                    "mRatPolicy : "
                            + Integer.toHexString(mRatPolicy).toUpperCase()
                            + ", mRat : "
                            + mRat
                            + ", ServiceState : EVDO");
            return true;
        }

        ImsLog.i(
                mSlotId,
                "mRatPolicy : "
                        + Integer.toHexString(mRatPolicy).toUpperCase()
                        + ", mRat : "
                        + mRat
                        + ", ServiceState : Unavailable");
        return false;
    }

    // CS Call state in TelephonyManager
    @Override
    public int getCallState() {
        return mCallState;
    }

    // precise CS Call state in TelephonyManager
    @Override
    public int getPreciseCallState() {
        return mPreciseCallState;
    }

    // Data service state in ServiceState
    @Override
    public int getDataServiceState() {
        return mDataServiceState;
    }

    // RAT info. will be returned to the type in TelephonyManager
    @Override
    public int getNetworkType() {
        return mRat;
    }

    // Voice RAT info. will be returned to the type in TelephonyManager
    @Override
    public int getVoiceNetworkType() {
        return mVoiceRat;
    }

    @Override
    // Voice service state in ServiceState
    public int getVoiceServiceState() {
        return mVoiceServiceState;
    }

    @Override
    public int getNrRegistrationInfo() {
        return mNrRegistrationInfo;
    }

    @Override
    public int getMocnPlmnInfo() {
        if (isLteEmergencyOnly()) {
            return 0;
        }

        // TODO : update MOCN PLMN info
        return 0;
    }

    @Override
    public String getOperatorNumeric() {
        return mNetworkOperator;
    }

    @Override
    public boolean isAirplaneMode() {
        return mAirplaneMode;
    }

    @Override
    public boolean isLteEmergencyOnly() {
        if (CapabilityConfigs.isVoNrEnabled(mSlotId)) {
            return isEmergencyOnlyForVonr();
        }

        // TODO: check EMERGENCY_ATTACHED
        return false;
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
    public boolean is1xRtt() {
        return is1xRtt(mRat);
    }

    @Override
    public boolean is2G() {
        return is2G(mRat);
    }

    @Override
    public boolean is3G() {
        return is3G(mRat);
    }

    @Override
    public boolean is4G() {
        return is4G(mRat);
    }

    @Override
    public boolean is5G() {
        return is5G(mRat);
    }

    @Override
    public boolean is5GRequired() {
        return ((mRatPolicy & POLICY_RAT_5G) != 0);
    }

    @Override
    public boolean isEhrpd() {
        return isEhrpd(mRat);
    }

    @Override
    public boolean isEvdo() {
        return isEvdo(mRat);
    }

    @Override
    public boolean isVoiceRat4G() {
        return is4G(mVoiceRat);
    }

    @Override
    public boolean isVoiceRat5G() {
        return is5G(mVoiceRat);
    }

    @Override
    public boolean isVops() {
        IDcSettings dcst = (IDcSettings) DcFactory.getDc(DcFactory.SETTING, mSlotId);
        if ((dcst != null) && dcst.isVopsRequired()) {
            return mImsVops;
        } else {
            return true;
        }
    }

    @Override
    public int getLteDuplexMode() {
        return mLteDuplexMode;
    }

    @Override
    public void setDoingOffRadio(boolean input) {
        mDoingOffRadio = input;
        mPowerOffChangedRegistrants.notifyResult(mDoingOffRadio);
    }

    @Override
    public boolean isDoingOffRadio() {
        return mDoingOffRadio;
    }

    @Override
    public void setRatFromTelephonyManager(int nRat) {
        mRatFromTm = nRat;
    }

    @Override
    public void setVoiceRatFromTelephonyManager(int nVoiceRat) {
        mVoiceRatFromTm = nVoiceRat;
    }

    @Override
    public void setNrRegistrationInfo(int state, int reason) {
        if (mDcNetWatcherHandler != null) {
            Message.obtain(mDcNetWatcherHandler, EVENT_NR_REGISTRATION_INFO, state, reason)
                    .sendToTarget();
        }
    }

    // Public methods --------------------------------------------
    /** Only subject class can invoke this API. DO NOT allow to accessed by observer class */
    public void notifyResult(EApnType eApnType, EDataState nDataState) {
        mStateDataConnectionState.notifyResult(new IDcNetWatcher.NotiObj(eApnType, nDataState, -1));
    }

    /** notify pdn connection failure */
    public void notifyPdnConnectionFailed(EApnType eApnType, int smCause) {
        mPdnConnectionFailedRegistrants.notifyResult(
                new IDcNetWatcher.NotiObj(eApnType, EDataState.DATA_STATE_CONNECT_FAILED, smCause));
    }

    /** Register data state */
    @Override
    public void registerForDataStateChanged(Handler h, int what, Object obj) {
        mStateDataConnectionState.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForDataStateChanged(Handler h) {
        mStateDataConnectionState.remove(h);
    }

    @Override
    public void registerForDataServiceStateChanged(Handler h, int what, Object obj) {
        mDataServiceStateChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForDataServiceStateChanged(Handler h) {
        mDataServiceStateChangedRegistrants.remove(h);
    }

    @Override
    public void registerForRatChanged(Handler h, int what, Object obj) {
        mRatChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForRatChanged(Handler h) {
        mRatChangedRegistrants.remove(h);
    }

    @Override
    public void registerForVoiceRatChanged(Handler h, int what, Object obj) {
        mVoiceRatChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForVoiceRatChanged(Handler h) {
        mVoiceRatChangedRegistrants.remove(h);
    }

    @Override
    public void registerForRoamingStateChanged(Handler h, int what, Object obj) {
        mRoamingStateChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForRoamingStateChanged(Handler h) {
        mRoamingStateChangedRegistrants.remove(h);
    }

    @Override
    public void registerForVoiceRoamingStateChanged(Handler h, int what, Object obj) {
        mVoiceRoamingStateChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForVoiceRoamingStateChanged(Handler h) {
        mVoiceRoamingStateChangedRegistrants.remove(h);
    }

    @Override
    public void registerForAirplaneModeChanged(Handler h, int what, Object obj) {
        mAirplaneModeChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForAirplaneModeChanged(Handler h) {
        mAirplaneModeChangedRegistrants.remove(h);
    }

    // Message (obj) - AsyncResult, AsyncResult (result) - NetworkOperator
    @Override
    public void registerForNetworkOperatorChanged(Handler h, int what, Object obj) {
        mNetworkOperatorChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForNetworkOperatorChanged(Handler h) {
        mNetworkOperatorChangedRegistrants.remove(h);
    }

    @Override
    public void registerForCsCallStatusChanged(Handler h, int what, Object obj) {
        mCsCallStatusChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForCsCallStatusChanged(Handler h) {
        mCsCallStatusChangedRegistrants.remove(h);
    }

    @Override
    public void registerForPreciseCsCallStatusChanged(Handler h, int what, Object obj) {
        mPreciseCsCallStatusChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForPreciseCsCallStatusChanged(Handler h) {
        mPreciseCsCallStatusChangedRegistrants.remove(h);
    }

    @Override
    public void registerForImsVopsChanged(Handler h, int what, Object obj) {
        mImsVopsChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForImsVopsChanged(Handler h) {
        mImsVopsChangedRegistrants.remove(h);
    }

    @Override
    public void registerForPowerOffChanged(Handler h, int what, Object obj) {
        mPowerOffChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForPowerOffChanged(Handler h) {
        mPowerOffChangedRegistrants.remove(h);
    }

    /** Register pdn connection failure */
    public void registerForPdnConnectionFailed(Handler h, int what, Object obj) {
        mPdnConnectionFailedRegistrants.add(new Registrant(h, what, obj));
    }

    /** Unregister pdn connection failure */
    public void unregisterForPdnConnectionFailed(Handler h) {
        mPdnConnectionFailedRegistrants.remove(h);
    }

    @Override
    public void registerForVoiceRoamingTypeChanged(Handler h, int what, Object obj) {
        mVoiceRoamingTypeChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForVoiceRoamingTypeChanged(Handler h) {
        mVoiceRoamingTypeChangedRegistrants.remove(h);
    }

    @Override
    public void registerForDataRoamingTypeChanged(Handler h, int what, Object obj) {
        mDataRoamingTypeChangedRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForDataRoamingTypeChanged(Handler h) {
        mDataRoamingTypeChangedRegistrants.remove(h);
    }

    // Private/Protected methods ---------------------------------
    private int checkAndConvertNrRat(int rat) {
        if (is5G(rat)) {
            if (!is5GRequired()) {
                return TelephonyManager.NETWORK_TYPE_LTE;
            }
        }

        return rat;
    }

    private int getPreferredRadioTechCategory(int radioTech) {
        if (is4GRequired() && is4G(radioTech)) {
            return RAT_4G;
        } else if (is3GRequired() && is3G(radioTech)) {
            return RAT_3G;
        } else if (is5GRequired() && is5G(radioTech)) {
            return RAT_5G;
        } else if (isEhrpdRequired() && isEhrpd(radioTech)) {
            return RAT_EHRPD;
        } else if (is1xRttRequired() && is1xRtt(radioTech)) {
            return RAT_1XRTT;
        } else if (is2GRequired() && is2G(radioTech)) {
            return RAT_2G;
        } else if (isEvdoRequired() && isEvdo(radioTech)) {
            return RAT_EVDO;
        }

        return 0;
    }

    private int getRadioTechCategory(int radioTech) {
        if (is4G(radioTech)) {
            return RAT_4G;
        } else if (is3G(radioTech)) {
            return RAT_3G;
        } else if (is5G(radioTech)) {
            return RAT_5G;
        } else if (isEhrpd(radioTech)) {
            return RAT_EHRPD;
        } else if (is1xRtt(radioTech)) {
            return RAT_1XRTT;
        } else if (is2G(radioTech)) {
            return RAT_2G;
        } else if (isEvdo(radioTech)) {
            return RAT_EVDO;
        }

        return 0;
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

    private static int getDataRegState(ServiceState ss) {
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

    private boolean is1xRttRequired() {
        return ((mRatPolicy & POLICY_RAT_1XRTT) != 0);
    }

    private boolean is2GRequired() {
        return ((mRatPolicy & POLICY_RAT_2G) != 0);
    }

    private boolean is3GRequired() {
        return ((mRatPolicy & POLICY_RAT_3G) != 0);
    }

    private boolean is4GRequired() {
        return ((mRatPolicy & POLICY_RAT_4G) != 0);
    }

    private boolean isEhrpdRequired() {
        return ((mRatPolicy & POLICY_RAT_EHRPD) != 0);
    }

    private boolean isEvdoRequired() {
        return ((mRatPolicy & POLICY_RAT_EVDO) != 0);
    }

    private boolean isEmergencyOnlyForVonr() {
        // TODO: check EMERGENCY_ATTACHED
        return (mNrRegistrationInfo == ImsEventDef.IMS_NR_INFO_EMERGENCY_REGISTRATION);
    }

    private static boolean is1xRtt(int rat) {
        switch (rat) {
            case TelephonyManager.NETWORK_TYPE_CDMA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                return true;
            default:
                return false;
        }
    }

    private static boolean is2G(int rat) {
        switch (rat) {
            case TelephonyManager.NETWORK_TYPE_GPRS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_GSM: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return true;
            default:
                return false;
        }
    }

    private static boolean is3G(int rat) {
        switch (rat) {
            case TelephonyManager.NETWORK_TYPE_UMTS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSDPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSUPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPAP: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                return true;
            default:
                return false;
        }
    }

    private static boolean is4G(int rat) {
        return (rat == TelephonyManager.NETWORK_TYPE_LTE);
    }

    private static boolean is5G(int rat) {
        return (rat == TelephonyManager.NETWORK_TYPE_NR);
    }

    private static boolean isEhrpd(int rat) {
        return (rat == TelephonyManager.NETWORK_TYPE_EHRPD);
    }

    private static boolean isEvdo(int rat) {
        switch (rat) {
            case TelephonyManager.NETWORK_TYPE_EVDO_0: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_EVDO_A: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
                return true;
            default:
                return false;
        }
    }

    private void setRatPolicy() {
        // NR RAT
        if (CapabilityConfigs.isVoNrEnabled(mSlotId)) {
            mRatPolicy |= POLICY_RAT_5G;
        }

        if (mDcSettings != null) {
            int[] supportedRats = mDcSettings.getImsSupportedRats();
            for (int i = 0; i < supportedRats.length; i++) {
                if (supportedRats[i] == AccessNetworkConstants.AccessNetworkType.EUTRAN) {
                    mRatPolicy |= POLICY_RAT_4G;
                } else if (supportedRats[i] == AccessNetworkConstants.AccessNetworkType.UTRAN) {
                    mRatPolicy |= POLICY_RAT_3G;
                } else if (supportedRats[i] == AccessNetworkConstants.AccessNetworkType.GERAN) {
                    mRatPolicy |= POLICY_RAT_2G;
                } else if (supportedRats[i] == AccessNetworkConstants.AccessNetworkType.IWLAN) {
                    mRatPolicy |= POLICY_RAT_WLAN;
                }
            }

            // Temp Setting
            if (supportedRats.length == 0) {
                mRatPolicy |= POLICY_RAT_4G;
                mRatPolicy |= POLICY_RAT_WLAN;
            }
        }
        ImsLog.i(mSlotId, "mRatPolicy=" + Integer.toHexString(mRatPolicy).toUpperCase());
    }

    private void setAirplaneMode() {
        int stateFromSettings =
                Settings.System.getInt(
                        mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1);

        if (stateFromSettings == 1) {
            mAirplaneMode = true;
        } else {
            mAirplaneMode = false;
        }
    }

    private void handleNetworkOperatorChanged() {
        mNetworkOperatorChangedRegistrants.notifyResult(mNetworkOperator);

        mAosInfo.notifyPlmnChanged();
    }

    private void handleDataServiceStateChanged() {
        mDataServiceStateChangedRegistrants.notifyResult(Integer.valueOf(mDataServiceState));

        mSystem.notifyServiceStateChanged(mDataServiceState);
    }

    private void handleVoiceServiceStateChanged() {
        mSystem.notifyEvent(ImsEventDef.IMS_EVENT_VOICE_SERVICE_STATE, mVoiceServiceState, 0);
    }

    // Data Radio Tech
    private void handleRadioTechChanged() {
        int radioTech = getPreferredRadioTechCategory(mRat);

        mRatChangedRegistrants.notifyResult(Integer.valueOf(radioTech));

        mSystem.notifyNetworkTypeChanged(radioTech);
    }

    // Voice Radio Tech
    private void handleVoiceRadioTechChanged() {
        int radioTech = getRadioTechCategory(mVoiceRat);

        mVoiceRatChangedRegistrants.notifyResult(Integer.valueOf(radioTech));
        mSystem.notifyVoiceNetworkTypeChanged(radioTech);
    }

    private void handleRoamingStateChanged(ServiceState ss) {
        // Notify combination of data and voice roaming state
        boolean roaming = ss.getRoaming();
        if (isRoaming() != roaming) {
            mRoamingStateChangedRegistrants.notifyResult(Boolean.valueOf(roaming));
        }

        // Notify data and voice roaming state respectively
        boolean isDataRoaming = getDataRoaming(ss);
        boolean isVoiceRoaming = getVoiceRoaming(ss);
        if (mVoiceRoaming != isVoiceRoaming) {
            if (mDataRoaming != isDataRoaming) {
                mDataRoaming = isDataRoaming;
            }
            mVoiceRoaming = isVoiceRoaming;
            mVoiceRoamingStateChangedRegistrants.notifyResult(Boolean.valueOf(mVoiceRoaming));
            notifyRoamingState(mDataRoaming, mVoiceRoaming);
        } else if (mDataRoaming != isDataRoaming) {
            mDataRoaming = isDataRoaming;
            notifyRoamingState(mDataRoaming, mVoiceRoaming);
        }

        mDataNetworkRoaming = getDataNetworkRoaming(ss);
    }

    private void notifyRoamingState(boolean dataRoaming, boolean voiceRoaming) {
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

    private void handleVoiceRoamingTypeChanged() {
        mVoiceRoamingTypeChangedRegistrants.notifyResult(mVoiceRoamingType);
    }

    private void handleDataRoamingTypeChanged() {
        mDataRoamingTypeChangedRegistrants.notifyResult(mDataRoamingType);
    }

    private void handleImsNetworkFeature(ServiceState ss) {
        // LTE duplex mode
        if (is4G() && (ss.getDuplexMode() != ServiceState.DUPLEX_MODE_UNKNOWN)) {
            mLteDuplexMode = ss.getDuplexMode();
            ImsLog.d(mSlotId, "lte duplex mode = " + mLteDuplexMode);
        }

        // VoPS and Emergency service
        NetworkRegistrationInfo regInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        if (regInfo != null) {
            DataSpecificRegistrationInfo dsrInfo = regInfo.getDataSpecificInfo();
            if (dsrInfo != null) {
                VopsSupportInfo vsi = dsrInfo.getVopsSupportInfo();
                if (vsi != null) {
                    boolean vops = vsi.isVopsSupported();
                    boolean emcbs = vsi.isEmergencyServiceSupported();

                    handleImsNetworkVoPSChanged(vops);
                    if (mEmcbs != emcbs) {
                        ImsLog.w(mSlotId, "update emergency service supported info : " + emcbs);
                        mEmcbs = emcbs;
                    }
                }

                // LTE attach result and extra info
                if (is4G()) {
                    int lteAttachResultType = getLteAttachResultType(
                            dsrInfo.getLteAttachResultType());
                    int lteAttachExtraInfo = getLteAttachExtraInfo(dsrInfo.getLteAttachExtraInfo());

                    if (mLteAttachResultType != lteAttachResultType
                            || mLteAttachExtraInfo != lteAttachExtraInfo) {
                        ImsLog.d(mSlotId, "lte attach result = " + lteAttachResultType
                                + ", extra = " + lteAttachExtraInfo);
                        mLteAttachResultType = lteAttachResultType;
                        mLteAttachExtraInfo = lteAttachExtraInfo;
                        mSystem.notifyEvent(ImsEventDef.IMS_EVENT_LTE_INFO, mLteAttachResultType,
                                mLteAttachExtraInfo);
                    }
                }
            }
        }
    }

    private void handleImsNetworkVoPSChanged(boolean currVops) {
        boolean vopsSupported = true;

        if (!currVops && mDcSettings != null && mDcSettings.isVopsRequired()) {
            vopsSupported = currVops;
        }

        if (mImsVops != vopsSupported) {
            mImsVops = vopsSupported;
            ImsLog.d(mSlotId, "updated vops = " + mImsVops);

            mImsVopsChangedRegistrants.notifyResult(Boolean.valueOf(mImsVops));

            int state =
                    (mImsVops)
                            ? ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED
                            : ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED;
            mSystem.notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE, state, 0);
        }
    }

    /** This class is for receiving the airplain mode intent */
    public class DcNetWatcherReceiver extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public DcNetWatcherReceiver() {
            mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
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
            setRatPolicy();
        }
    }

    private final class DcNetWatcherNativeStateListener implements NativeStateInterface.Listener {
        @Override
        public void onNativeServiceReady() {
            ImsLog.i(mSlotId, "NativeState: service ready.");

            if (mAosInfo != null) {
                mAosInfo.notifyAirplaneSetting(mAirplaneMode);
            }

            if (mSystem != null) {
                mSystem.notifyAirplaneModeChanged(mAirplaneMode ? 1 : 0);
            }

            // roaming state
            notifyRoamingState(mDataRoaming, mVoiceRoaming);

            handleVoiceServiceStateChanged();
            mSystem.notifyEvent(ImsEventDef.IMS_EVENT_LTE_INFO, mLteAttachResultType,
                    mLteAttachExtraInfo);
        }
    }

    private class DcNetWatcherPhoneStateListener implements ImsPhoneStateListener {
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
        public void onServiceStateChanged(ServiceState serviceState) {
            int voiceRegState = serviceState.getState();
            int voiceRAT = getAccessNetworkTechnology(serviceState);
            int dataRegState = getDataRegState(serviceState);
            int dataRAT = getCellularDataRAT();
            String operatorNumeric = serviceState.getOperatorNumeric();
            int voiceRoamingType = getVoiceRoamingType(serviceState);
            int dataRoamingType = getDataRoamingType(serviceState);

            // Phone service state
            if (mVoiceServiceState != voiceRegState) {
                mVoiceServiceState = voiceRegState;
                handleVoiceServiceStateChanged();
            }

            // Network Operator Info.
            if (operatorNumeric == null) {
                mNetworkOperator = "";
            } else if (!mNetworkOperator.equals(operatorNumeric)) {
                if (dataRegState == ServiceState.STATE_IN_SERVICE) {
                    mNetworkOperator = operatorNumeric;
                    handleNetworkOperatorChanged();
                }
            }

            // Data service state
            if (mDataServiceState != dataRegState) {
                mDataServiceState = dataRegState;
                handleDataServiceStateChanged();
            }

            // Data RAT info.
            dataRAT = checkAndConvertNrRat(dataRAT);
            if (mRat != dataRAT || getNetworkType() != mRatFromTm) {
                mRat = dataRAT;
                handleRadioTechChanged();
            }

            // Voice RAT info.
            voiceRAT = checkAndConvertNrRat(voiceRAT);
            if (mVoiceRat != voiceRAT || getVoiceNetworkType() != mVoiceRatFromTm) {
                mVoiceRat = voiceRAT;
                handleVoiceRadioTechChanged();
            }

            // Roaming state
            handleRoamingStateChanged(serviceState);

            // network feature for IMS
            if (is4G() || is5G()) {
                handleImsNetworkFeature(serviceState);
            }

            // Voice Roaming Type
            if (mVoiceRoamingType != voiceRoamingType) {
                mVoiceRoamingType = voiceRoamingType;
                handleVoiceRoamingTypeChanged();
            }

            // Data Roaming Type
            if (mDataRoamingType != dataRoamingType) {
                mDataRoamingType = dataRoamingType;
                handleDataRoamingTypeChanged();
            }
        }

        /** Invokes when call state is changed. */
        @Override
        public void onCallStateChanged(@CallState int state) {
            TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                    TelephonyInterface.class, mSlotId);

            if (telephony == null) {
                return;
            }

            int csStateFromPhone = telephony.getCsCallState();

            if (csStateFromPhone != state) {
                ImsLog.i(mSlotId, "call state is not CS");
                if ((csStateFromPhone == TelephonyManager.CALL_STATE_IDLE)
                        && (mCallState != TelephonyManager.CALL_STATE_IDLE)) {
                    ImsLog.i(mSlotId, "CS call state is not IDLE. Therefore update it explicitly");
                    state = TelephonyManager.CALL_STATE_IDLE;
                } else {
                    return;
                }
            }

            if (mCallState != state) {
                mCallState = state;
                mCsCallStatusChangedRegistrants.notifyResult(Integer.valueOf(state));

                mSystem.notifyVoiceCallStateChanged(mCallState);
                mSystem.notifyEvent(ImsEventDef.IMS_EVENT_CSCALL_STATE, mCallState, 0);
            }
        }

        /** Invokes when precise call state is changed. */
        @Override
        public void onPreciseCallStateChanged(PreciseCallState callState) {
            if (mCallState == TelephonyManager.CALL_STATE_IDLE) {
                if (mPreciseCallState != PreciseCallState.PRECISE_CALL_STATE_IDLE) {
                    mPreciseCallState = PreciseCallState.PRECISE_CALL_STATE_IDLE;
                    mPreciseCsCallStatusChangedRegistrants.notifyResult(
                            Integer.valueOf(mPreciseCallState));
                    mAosInfo.notifyPreciseCallState(mPreciseCallState);
                }
                return;
            }

            // For CS Call, both Foreground and Background Call state check is required
            int nForegroundCallState = callState.getForegroundCallState();
            int nBackgroundCallState = callState.getBackgroundCallState();

            ImsLog.i(
                    mSlotId,
                    "nForegroundCallState = "
                            + nForegroundCallState
                            + ", nBackgroundCallState = "
                            + nBackgroundCallState);

            if ((nBackgroundCallState == PreciseCallState.PRECISE_CALL_STATE_HOLDING)
                    || (nBackgroundCallState == PreciseCallState.PRECISE_CALL_STATE_ACTIVE)) {
                mPreciseCallState = nBackgroundCallState;
                mPreciseCsCallStatusChangedRegistrants.notifyResult(
                        Integer.valueOf(mPreciseCallState));
                mAosInfo.notifyPreciseCallState(mPreciseCallState);
            } else {
                // If Background Call is not present, post ForegroundCallState to AoSCallTracker
                if (mPreciseCallState != nForegroundCallState) {
                    mPreciseCallState = nForegroundCallState;
                    mPreciseCsCallStatusChangedRegistrants.notifyResult(
                            Integer.valueOf(mPreciseCallState));
                    mAosInfo.notifyPreciseCallState(mPreciseCallState);
                }
            }
        }

        private int getCellularDataRAT() {
            TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                    TelephonyInterface.class, mSlotId);

            return (telephony != null)
                    ? telephony.getNetworkType()
                    : TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }
    }

    // -----------------------------------------------------------
    private class DcNetWatcherHandler extends Handler {
        DcNetWatcherHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            if (msg != null) {
                ImsLog.i(mSlotId, "Message(" + msg.what + ")");

                switch (msg.what) {
                    case EVENT_AIRPLANE_MODE_CHANGED:
                        handleAirplaneModeChanged(msg);
                        break;
                    case EVENT_VOLTE_LTE_STATE_INFO:
                        // TODO: will be implemented
                        break;
                    case EVENT_NR_REGISTRATION_INFO:
                        handleNrRegistrationInfo(msg.arg1, msg.arg2);
                        break;
                    default:
                        break;
                }
            }
        }

        private void handleAirplaneModeChanged(Message msg) {
            Intent intent = (Intent) msg.obj;

            if (intent == null) {
                return;
            }

            boolean state = intent.getBooleanExtra("state", false);
            int stateFromIntent = 0;
            if (mAirplaneMode != state) {

                if (state) {
                    stateFromIntent = 1;
                } else {
                    stateFromIntent = 0;
                }

                if (mContext == null) {
                    return;
                }

                int stateFromSettings =
                        Settings.System.getInt(
                                mContext.getContentResolver(),
                                Settings.Global.AIRPLANE_MODE_ON,
                                -1);

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

                if (mAosInfo != null) {
                    mAosInfo.notifyAirplaneSetting(mAirplaneMode);
                }

                if (mSystem != null) {
                    mSystem.notifyAirplaneModeChanged(mAirplaneMode ? 1 : 0);
                }

                // Notify the state changed to the applications
                mAirplaneModeChangedRegistrants.notifyResult(Boolean.valueOf(mAirplaneMode));
            }
        }

        private void handleNrRegistrationInfo(int state, int reason) {
            if (!isNrRegistrationInfoValid(state)) {
                return;
            }

            if (mNrRegistrationInfo != state) {
                mNrRegistrationInfo = state;
                mDataServiceStateChangedRegistrants.notifyResult(
                        Integer.valueOf(mDataServiceState));
                mSystem.notifyEvent(ImsEventDef.IMS_EVENT_NR_INFO, state, reason);
            }
        }

        private boolean isNrRegistrationInfoValid(int state) {
            switch (state) {
                case ImsEventDef.IMS_NR_INFO_REGISTRATION: // FALL-THROUGH
                case ImsEventDef.IMS_NR_INFO_DEREGISTRATION: // FALL-THROUGH
                case ImsEventDef.IMS_NR_INFO_EMERGENCY_REGISTRATION:
                    return true;

                default:
                    return false;
            }
        }
    }
}

