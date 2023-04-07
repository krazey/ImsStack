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

package com.android.imsstack.core.service;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsManager;
import android.telephony.ims.ImsMmTelManager;
import android.text.TextUtils;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.service.serviceif.ICallSettingService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.MSimUtils;

/** this class is the interface for the call setting */
public class CallSettingService implements ICallSettingService {
    private static final int VALUE_NOT_INITIALIZED = -1;

    private ContentObserver mVoLTESettingObserver = null;
    private ContentObserver mVoWIFISettingObserver = null;
    private ContentObserver mMobileDataSettingObserver = null;
    private ContentObserver mVideoCallSettingObserver = null;
    private ContentObserver mVideoCallRoamingSettingObserver = null;
    private ContentObserver mVoLTERoamingObserver = null;
    private ContentObserver mVoWIFIPreferenceObserver = null;
    private ContentObserver mVoWIFIRoamingSetObserver = null;
    private ContentObserver mDataRoamingSettingObserver = null;
    private ContentObserver mRttModeSettingObserver = null;
    private RegistrantList mVoLTESettingRegistrants = new RegistrantList();
    private RegistrantList mVoWIFISettingRegistrants = new RegistrantList();
    private RegistrantList mVideoCallSettingRegistrants = new RegistrantList();
    private RegistrantList mVideoCallRoamingSettingRegistrants = new RegistrantList();
    private RegistrantList mVoLTERoamingRegistrants = new RegistrantList();
    private RegistrantList mVoWIFIPreferenceRegistrants = new RegistrantList();
    private RegistrantList mVoWIFIRoamingSetRegistrants = new RegistrantList();
    private RegistrantList mMobileDataSettingRegistrants = new RegistrantList();
    private RegistrantList mDataRoamingSettingRegistrants = new RegistrantList();
    private RegistrantList mRttModeSettingRegistrants = new RegistrantList();
    private IVoLteService mVoLteService = null;
    private boolean mVoLTESet = false;
    private boolean mVoWIFISet = false;
    private boolean mVideoCallSet = false;
    private boolean mVideoCallRoamingSet = false;
    private boolean mVoLTERoamingSet = false;
    private boolean mVoWIFIRoamingSet = false;
    private boolean mMobileDataEnabled = false;
    private boolean mDataRoamingEnabled = false;
    private int mVoWIFIPreference = -1; // 0: wifi only /1: cellular pref. /2: wifi pref.
    private int mRttMode = -1;
    public CallSettingService() {
    }

    @Override
    public boolean start(IVoLteService voLteService) {
        mVoLteService = voLteService;

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.setISystemAPIWifiCalling(this);
        }

        AgentFactory.setAgentForMIms(this, AgentFactory.CALL_SETTING, getSlotId());

        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.d(getSlotId(), "");

        unregisterObserver(mVoLTESettingObserver);
        mVoLTESettingObserver = null;
        unregisterObserver(mVoWIFISettingObserver);
        mVoWIFISettingObserver = null;
        unregisterObserver(mVideoCallSettingObserver);
        mVideoCallSettingObserver = null;
        unregisterObserver(mVideoCallRoamingSettingObserver);
        mVideoCallRoamingSettingObserver = null;
        unregisterObserver(mVoLTERoamingObserver);
        mVoLTERoamingObserver = null;
        unregisterObserver(mVoWIFIPreferenceObserver);
        mVoWIFIPreferenceObserver = null;
        unregisterObserver(mVoWIFIRoamingSetObserver);
        mVoWIFIRoamingSetObserver = null;
        unregisterObserver(mDataRoamingSettingObserver);
        mDataRoamingSettingObserver = null;
        unregisterObserver(mRttModeSettingObserver);
        mRttModeSettingObserver = null;
    }

    @Override
    public void update(Context context) {
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    @Override
    public int isWifiCallingEnabled4Sys() {
        return isVoWiFiSettingEnabled() ? 1 : 0;
    }

    @Override
    public int isWifiCallingProvisioned4Sys() {
        return isVoWIFIProvisioned() ? 1 : 0;
    }

    @Override
    public int getWifiCallingPreferences4Sys() {
        return getVoWIFIPreference();
    }

    @Override
    public String getWifiCallingAddressID4Sys() {
        return ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_VOWIFI_ENTITLEMENT_ID,
                "",
                getSlotId());
    }

    @Override
    public boolean isWfcEnabled() {
        return isVoWiFiSettingEnabled();
    }

    @Override
    public boolean isVideoCallEnabled() {
        return isVtSettingEnabled();
    }

    @Override
    public int getRTTMode() {
        return getRttMode();
    }

    @Override
    public void notifySystemEvents() {
        notifySystemEventForVoLTE();
        notifySystemEventForVoWIFI();
        notifySystemEventForVoWIFIPreference();
        notifySystemEventForVoWIFIRoaming();
    }

    @Override
    public void updateForSetting() {
        updateForVoLTESetting();
    }

    @Override
    public void updateForRoamingSetting(boolean bRoaming) {
        updateForVoWIFIRoamingSetting(bRoaming);
        updateForVoLTERoamingSetting(bRoaming);
    }

    @Override
    public void registerForMobileDataSettingChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mMobileDataSettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerMobileDataSettingObserver();
    }

    @Override
    public void registerForVideoCallSetChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVideoCallSettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerVideoCallSetObserver();
    }

    @Override
    public void registerForVideoCallRoamingSetChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVideoCallRoamingSettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerVideoCallRoamingSetObserver();
    }

    @Override
    public void registerForVoLTESetChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVoLTESettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerVoLTESetObserver();
    }

    @Override
    public void registerForVoLTERoamingSetChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVoLTERoamingRegistrants.add(new Registrant(h, what, obj));
        }

        registerVoLTERoamingSetObserver();
    }

    @Override
    public void registerForVoWIFISetChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVoWIFISettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerVoWIFISetObserver();
    }

    @Override
    public void registerForVoWIFIPreferenceChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVoWIFIPreferenceRegistrants.add(new Registrant(h, what, obj));
        }

        registerVoWIFIPreferenceObserver();
    }

    @Override
    public void registerForVoWIFIRoamingSetChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mVoWIFIRoamingSetRegistrants.add(new Registrant(h, what, obj));
        }

        registerVoWIFIRoamingSetObserver();
    }

    @Override
    public void registerForDataRoamingSettingChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mDataRoamingSettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerDataRoamingSettingObserver();
    }

    @Override
    public void registerForRttModeSettingChanged(Handler h, int what, Object obj) {
        if (h != null) {
            mRttModeSettingRegistrants.add(new Registrant(h, what, obj));
        }

        registerRttModeSettingObserver();
    }

    @Override
    public void unregisterForMobileDataSettingChanged(Handler h) {
        if (h != null) {
            mMobileDataSettingRegistrants.remove(h);
        }

        if (mMobileDataSettingRegistrants.size() == 0) {
            unregisterObserver(mMobileDataSettingObserver);
            mMobileDataSettingObserver = null;
        }
    }

    @Override
    public void unregisterForVideoCallSetChanged(Handler h) {
        if (h != null) {
            mVideoCallSettingRegistrants.remove(h);
        }

        if (mVideoCallSettingRegistrants.size() == 0) {
            unregisterObserver(mVideoCallSettingObserver);
            mVideoCallSettingObserver = null;
        }
    }

    @Override
    public void unregisterForVideoCallRoamingSetChanged(Handler h) {
        if (h != null) {
            mVideoCallRoamingSettingRegistrants.remove(h);
        }

        if (mVideoCallRoamingSettingRegistrants.size() == 0) {
            unregisterObserver(mVideoCallRoamingSettingObserver);
            mVideoCallRoamingSettingObserver = null;
        }
    }

    @Override
    public void unregisterForVoLTESetChanged(Handler h) {
        if (h != null) {
            mVoLTESettingRegistrants.remove(h);
        }

        if (mVoLTESettingRegistrants.size() == 0) {
            unregisterObserver(mVoLTESettingObserver);
            mVoLTESettingObserver = null;
        }
    }

    @Override
    public void unregisterForVoLTERoamingSetChanged(Handler h) {
        if (h != null) {
            mVoLTERoamingRegistrants.remove(h);
        }

        if (mVoLTERoamingRegistrants.size() == 0) {
            unregisterObserver(mVoLTERoamingObserver);
            mVoLTERoamingObserver = null;
        }
    }

    @Override
    public void unregisterForVoWIFISetChanged(Handler h) {
        if (h != null) {
            mVoWIFISettingRegistrants.remove(h);
        }

        if (mVoWIFISettingRegistrants.size() == 0) {
            unregisterObserver(mVoWIFISettingObserver);
            mVoWIFISettingObserver = null;
        }
    }

    @Override
    public void unregisterForVoWIFIPreferenceChanged(Handler h) {
        if (h != null) {
            mVoWIFIPreferenceRegistrants.remove(h);
        }

        if (mVoWIFIPreferenceRegistrants.size() == 0) {
            unregisterObserver(mVoWIFIPreferenceObserver);
            mVoWIFIPreferenceObserver = null;
        }
    }

    @Override
    public void unregisterForVoWIFIRoamingSetChanged(Handler h) {
        if (h != null) {
            mVoWIFIRoamingSetRegistrants.remove(h);
        }

        if (mVoWIFIRoamingSetRegistrants.size() == 0) {
            unregisterObserver(mVoWIFIRoamingSetObserver);
            mVoWIFIRoamingSetObserver = null;
        }
    }

    @Override
    public void unregisterForDataRoamingSettingChanged(Handler h) {
        if (h != null) {
            mDataRoamingSettingRegistrants.remove(h);
        }

        if (mDataRoamingSettingRegistrants.size() == 0) {
            unregisterObserver(mDataRoamingSettingObserver);
            mDataRoamingSettingObserver = null;
        }
    }

    @Override
    public void unregisterForRttModeSettingChanged(Handler h) {
        if (h != null) {
            mRttModeSettingRegistrants.remove(h);
        }

        if (mRttModeSettingRegistrants.size() == 0) {
            unregisterObserver(mRttModeSettingObserver);
            mRttModeSettingObserver = null;
        }
    }

    protected Handler getHandler() {
        return AppContext.getInstance().getMainHandler();
    }

    private void registerMobileDataSettingObserver() {
        ImsLog.d(getSlotId(), "");

        if (mMobileDataSettingObserver != null) {
            return;
        }

        mMobileDataSettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onMobileDataSettingChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.MOBILE_DATA), true,
                mMobileDataSettingObserver);

        onMobileDataSettingChanged();
    }

    private void registerVideoCallSetObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVideoCallSettingObserver != null) {
            return;
        }

        mVideoCallSettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onVideoCallSetChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, mVideoCallSettingObserver);

        onVideoCallSetChanged();
    }

    private void registerVideoCallRoamingSetObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVideoCallRoamingSettingObserver != null) {
            return;
        }

        mVideoCallRoamingSettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onVideoCallRoamingSetChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                Settings.Secure.getUriFor("data_network_video_calling_status_roaming"), true,
                mVideoCallRoamingSettingObserver);

        onVideoCallRoamingSetChanged();
    }

    private void registerVoLTESetObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVoLTESettingObserver != null) {
            return;
        }

        mVoLTESettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onVoLTESetChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, mVoLTESettingObserver);

        onVoLTESetChanged();
    }

    private void registerVoLTERoamingSetObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVoLTERoamingObserver != null) {
            return;
        }

        mVoLTERoamingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onVoLTERoamingSetChanged();
            }
        };

        ContentResolver cr = getContext().getContentResolver();

        cr.registerContentObserver(Settings.Global.getUriFor("roaming_hdvoice_enabled"), true,
                mVoLTERoamingObserver);

        if (!ImsGlobal.isOperator(getSlotId(), "SKT")) {
            cr.registerContentObserver(Settings.Secure.getUriFor("data_lte_roaming"), true,
                    mVoLTERoamingObserver);
        }

        onVoLTERoamingSetChanged();
    }

    private void registerVoWIFISetObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVoWIFISettingObserver != null) {
            return;
        }

        mVoWIFISettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onVoWIFISetChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, mVoWIFISettingObserver);

        onVoWIFISetChanged();
    }

    private void registerVoWIFIPreferenceObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVoWIFIPreferenceObserver != null) {
            return;
        }

        mVoWIFIPreferenceObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);

                int preference = getVoWIFIPreference();

                if (mVoWIFIPreference == preference) {
                    return;
                }

                ImsLog.i(getSlotId(), "vowifi preference changed : "
                        + mVoWIFIPreference + " => " + preference);
                mVoWIFIPreference = preference;

                mVoWIFIPreferenceRegistrants.notifyResult(mVoWIFIPreference);

                notifySystemEventForVoWIFIPreference();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, mVoWIFIPreferenceObserver);

        mVoWIFIPreference = getVoWIFIPreference();

        notifySystemEventForVoWIFIPreference();
    }

    private void registerVoWIFIRoamingSetObserver() {
        ImsLog.d(getSlotId(), "");

        if (mVoWIFIRoamingSetObserver != null) {
            return;
        }

        mVoWIFIRoamingSetObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);

                boolean roamingEnabled = isVoWiFiRoamingSettingEnabled();
                if (mVoWIFIRoamingSet == roamingEnabled) {
                    return;
                }

                ImsLog.i(getSlotId(), "vowifi roaming set changed : "
                        + mVoWIFIRoamingSet + " => " + roamingEnabled);
                mVoWIFIRoamingSet = roamingEnabled;

                mVoWIFIRoamingSetRegistrants.notifyResult(mVoWIFIRoamingSet);

                notifySystemEventForVoWIFIRoaming();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, mVoWIFIRoamingSetObserver);

        mVoWIFIRoamingSet = isVoWiFiRoamingSettingEnabled();

        notifySystemEventForVoWIFIRoaming();
    }

    private void registerDataRoamingSettingObserver() {
        ImsLog.d(getSlotId(), "");

        if (mDataRoamingSettingObserver != null) {
            return;
        }

        mDataRoamingSettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onDataRoamingSettingChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.DATA_ROAMING), true,
                mDataRoamingSettingObserver);

        onDataRoamingSettingChanged();
    }

    private void registerRttModeSettingObserver() {
        ImsLog.d(getSlotId(), "");

        if (mRttModeSettingObserver != null) {
            return;
        }

        mRttModeSettingObserver = new ContentObserver(getHandler()) {
            @Override
            public void onChange(boolean bChange) {
                super.onChange(bChange);
                onRttModeSettingChanged();
            }
        };

        getContext().getContentResolver().registerContentObserver(
                Settings.System.getUriFor("rtt_option"), true, mRttModeSettingObserver);
        getContext().getContentResolver().registerContentObserver(
                Settings.System.getUriFor("rtt_operation_mode"), true, mRttModeSettingObserver);

        onRttModeSettingChanged();
    }

    private void unregisterObserver(ContentObserver co) {
        if (co == null) {
            return;
        }

        ContentResolver cr = getContext().getContentResolver();
        if (cr != null) {
            cr.unregisterContentObserver(co);
        }
    }

    private boolean getVoLTERoamingSetForKR() {
        return isVoLTEEnabled();
    }

    private boolean getVoLTESetForKT() {
        boolean bNetMode = isVoLTEEnabled();

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return bNetMode;
        }

        boolean bVolteEnable = false;
        int hdVoiceSetting = VALUE_NOT_INITIALIZED;
        boolean bVolteSet = (hdVoiceSetting == 1
                || hdVoiceSetting == VALUE_NOT_INITIALIZED);

        ImsLog.d(getSlotId(), "VoLte :: setting=" + bVolteSet + ", lteMode=" + bNetMode);

        if (bVolteSet && bNetMode) {
            bVolteEnable = true;
        }

        ImsLog.d(getSlotId(), "KT MQI - VoLTE Setting : " + (bVolteEnable ? "On" : "Off"));
        return bVolteEnable;
    }

    private int getVoWIFIPreference() {
        if (ImsGlobal.isOperator(getSlotId(), "ATT")) {
            return ImsEventDef.MODE_WFC_PREFERRED;
        }

        return getVoWiFiModeSetting();
    }

    private int getRttMode() {
        int option = -1;

        if (option == 0) {
            return ImsEventDef.IMS_RTT_CAPABLE_OFF;
        } else if (option == 1) {
            int mode = -1;

            if (mode == 0) {
                return ImsEventDef.IMS_RTT_VISIBLE_DURING_CALLS;
            } else if (mode == 1) {
                return ImsEventDef.IMS_RTT_ALWAYS_VISIBLE;
            }
        }

        return ImsEventDef.IMS_RTT_MODE_NONE;
    }

    private boolean isMobileDataEnabled() {
        TelephonyManager tm = AppContext.getTelephonyManager(MSimUtils.getSubId(getSlotId()));
        return (tm != null) ? tm.isDataEnabled() : false;
    }

    private boolean isVoLTEEnabled() {
        return false;
    }

    private boolean isVoLTERoamingEnabled() {
        return false;
    }

    private boolean isVoWIFIProvisioned() {
        String vwfMDN = "";

        if (TextUtils.isEmpty(vwfMDN)) {
            ImsLog.d(getSlotId(), "VoWIFI MDN is invalid");
            return false;
        }

        SubscriptionManager sm =
                AppContext.getInstance().getSystemService(SubscriptionManager.class);
        String mdn = null;

        try {
            if (sm != null) {
                mdn = sm.getPhoneNumber(MSimUtils.getSubId(getSlotId()),
                        SubscriptionManager.PHONE_NUMBER_SOURCE_UICC);
            }
        } catch (Exception e) {
            ImsLog.w(getSlotId(), "getPhoneNumber: " + e.toString());
        }

        if (TextUtils.isEmpty(mdn)) {
            ImsLog.d(getSlotId(), "Telephony MDN is invalid");
            return false;
        }

        boolean result = mdn.equals(vwfMDN);

        ImsLog.d(getSlotId(), vwfMDN + " " + mdn + " is Wfc Provisioned? "
                + (result ? "Provisioned" : "not Provisioned"));

        return result;
    }

    private boolean isVideoCallRoamingSetEnabled() {
        return false;
    }

    private boolean isDataRoamingSettingEnabled() {
        TelephonyManager tm = AppContext.getTelephonyManager(MSimUtils.getSubId(getSlotId()));
        return (tm != null) ? tm.isDataRoamingEnabled() : false;
    }

    private void notifySystemEventForVoLTE() {
        if (mVoLTESettingObserver == null) {
            return;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                    mVoLTESet ? ImsEventDef.IMS_VOLTE_SETTING_ON
                            : ImsEventDef.IMS_VOLTE_SETTING_OFF,
                    0);
        }
    }

    private void notifySystemEventForVoWIFI() {
        if (mVoWIFISettingObserver == null) {
            return;
        }

        if (isRoaming()
                && mVoWIFIRoamingSetObserver != null
                && isVoWIFIRoamingRequired()) {
            notifySystemEventForVoWIFIRoaming();
            return;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                    mVoWIFISet ? 1 : 0, getVoWIFIPreference());
        }
    }

    private void notifySystemEventForVoWIFIPreference() {
        if (mVoWIFIPreferenceObserver == null) {
            return;
        }

        if (isRoaming()
                && mVoWIFIRoamingSetObserver != null
                && isVoWIFIRoamingRequired()) {
            return;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                    isVoWiFiSettingEnabled() ? 1 : 0, mVoWIFIPreference);
        }
    }

    private void notifySystemEventForVoWIFIRoaming() {
        if (mVoWIFIRoamingSetObserver == null) {
            return;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            if (isRoaming()) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                        isVoWiFiSettingEnabled() ? 1 : 0,
                        mVoWIFIRoamingSet ? ImsEventDef.MODE_WFC_PREFERRED
                                : ImsEventDef.MODE_CELLULAR_PREFERRED);
            }
        }
    }

    private void notifySystemEventForRttModeSettingChanged() {
        if (mRttModeSettingObserver == null) {
            return;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_RTT_SETTING, mRttMode, 0);
        }
    }

    private void onMobileDataSettingChanged() {
        boolean enabled = isMobileDataEnabled();

        if (mMobileDataEnabled == enabled) {
            return;
        }

        ImsLog.i(getSlotId(), "mobile data set changed : "
                + mMobileDataEnabled + " => " + enabled);

        mMobileDataEnabled = enabled;

        mMobileDataSettingRegistrants.notifyResult(Boolean.valueOf(enabled));
    }

    private void onVideoCallSetChanged() {
        boolean setValue = isVtSettingEnabled();

        if (mVideoCallSet == setValue) {
            return;
        }

        if (ImsGlobal.isOperator(getSlotId(), "TEL")) {
            if (isRoaming()) {
                ImsLog.d(getSlotId(), "roaming, ignore it");
                return;
            }
        }

        ImsLog.i(getSlotId(), "video call set changed : " + mVideoCallSet + " => " + setValue);

        mVideoCallSet = setValue;

        mVideoCallSettingRegistrants.notifyResult(mVideoCallSet);
    }

    private void onVideoCallRoamingSetChanged() {
        boolean setValue = isVideoCallRoamingSetEnabled();

        if (mVideoCallRoamingSet == setValue) {
            return;
        }

        if (ImsGlobal.isOperator(getSlotId(), "TEL")) {
            if (!isRoaming()) {
                ImsLog.d(getSlotId(), "not roaming, ignore it");
                return;
            }
        }

        ImsLog.i(getSlotId(), "(roaming) video call set changed : "
                + mVideoCallRoamingSet + " => " + setValue);

        mVideoCallRoamingSet = setValue;

        mVideoCallRoamingSettingRegistrants.notifyResult(mVideoCallRoamingSet);
    }

    private boolean isRoaming() {
        IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(
                DcFactory.NETWORK_WATCHER, getSlotId());
        return (dcnw != null) ? dcnw.isRoaming() : false;
    }

    private void onVoLTESetChanged() {
        boolean bValue = false;
        int nEvent = ImsEventDef.IMS_EVENT_VOLTE_SETTING;
        int nParam1 = -1;

        if (ImsGlobal.isOperator(getSlotId(), "KT")) {
            bValue = getVoLTESetForKT();
        } else {
            bValue = isVoLTEEnabled();
        }

        if (bValue) {
            nParam1 = ImsEventDef.IMS_VOLTE_SETTING_ON;
        } else {
            nParam1 = ImsEventDef.IMS_VOLTE_SETTING_OFF;
        }

        ImsLog.i(getSlotId(), "volte set changed : " + mVoLTESet + " => " + bValue);

        if (mVoLTESet == bValue) {
            return;
        }

        mVoLTESet = bValue;

        mVoLTESettingRegistrants.notifyResult(mVoLTESet);

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.notifyEvent(nEvent, nParam1, 0);
        }
    }

    private void onVoLTERoamingSetChanged() {
        boolean setValue = false;

        if (ImsGlobal.isCountry(getSlotId(), "KR")) {
            setValue = getVoLTERoamingSetForKR();
        } else {
            setValue = isVoLTERoamingEnabled();
        }

        ImsLog.i(getSlotId(), "volte roaming changed : "
                + mVoLTERoamingSet + " => " + setValue);

        if (mVoLTERoamingSet == setValue) {
            return;
        }

        mVoLTERoamingSet = setValue;
        mVoLTERoamingRegistrants.notifyResult(mVoLTERoamingSet);
    }

    private void onVoWIFISetChanged() {
        boolean setValue = isVoWiFiSettingEnabled();

        if (mVoWIFISet == setValue) {
            return;
        }

        ImsLog.i(getSlotId(), "vowifi changed : " + mVoWIFISet + " => " + setValue);

        mVoWIFISet = setValue;

        mVoWIFISettingRegistrants.notifyResult(mVoWIFISet);

        notifySystemEventForVoWIFI();
    }

    private void onDataRoamingSettingChanged() {
        boolean setValue = isDataRoamingSettingEnabled();

        if (mDataRoamingEnabled == setValue) {
            return;
        }

        ImsLog.i(getSlotId(), "Data roaming setting changed : "
                + mDataRoamingEnabled + " => " + setValue);

        mDataRoamingEnabled = setValue;
        mDataRoamingSettingRegistrants.notifyResult(mDataRoamingEnabled);
    }

    private void onRttModeSettingChanged() {
        int rttMode = getRttMode();

        if (rttMode > 0 && mRttMode != rttMode) {
            ImsLog.i(getSlotId(), "RTT mode setting changed : " + mRttMode + " => " + rttMode);

            mRttMode = rttMode;
            mRttModeSettingRegistrants.notifyResult(mRttMode);

            notifySystemEventForRttModeSettingChanged();
        }
    }

    private void updateForVoLTESetting() {
        if (ImsGlobal.isOperator(getSlotId(), "KT") || ImsGlobal.isOperator(getSlotId(), "SKT")) {
            boolean voLteSet = false;

            if (ImsGlobal.isOperator(getSlotId(), "KT")) {
                voLteSet = getVoLTESetForKT();
            } else if (ImsGlobal.isOperator(getSlotId(), "SKT")) {
                voLteSet = isVoLTEEnabled();
            }

            ImsLog.i(getSlotId(), "volte setting : " + mVoLTESet + " => " + voLteSet);

            ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                        voLteSet ? ImsEventDef.IMS_VOLTE_SETTING_ON
                                : ImsEventDef.IMS_VOLTE_SETTING_OFF,
                        0);
            }

            if (mVoLTESet != voLteSet) {
                mVoLTESet = voLteSet;
                mVoLTESettingRegistrants.notifyResult(mVoLTESet);
            }
        }
    }

    private void updateForVoWIFIRoamingSetting(boolean roaming) {
        if (mVoWIFIRoamingSetObserver == null) {
            return;
        }

        if (roaming) {
            notifySystemEventForVoWIFIRoaming();
        } else {
            notifySystemEventForVoWIFI();
        }
    }

    private void updateForVoLTERoamingSetting(boolean roaming) {
        if (roaming) {
            boolean roamingSet = false;

            if (ImsGlobal.isCountry(getSlotId(), "KR")) {
                roamingSet = getVoLTERoamingSetForKR();
            } else {
                return;
            }

            ImsLog.i(getSlotId(), "volte roaming changed : "
                    + mVoLTERoamingSet + " => " + roamingSet);

            if (mVoLTERoamingSet == roamingSet) {
                return;
            }

            mVoLTERoamingSet = roamingSet;
            mVoLTERoamingRegistrants.notifyResult(mVoLTERoamingSet);
        } else {
            updateForVoLTESetting();
        }
    }

    private boolean isVoWIFIRoamingRequired() {
        // TODO_CONFIG
        return false;
    }

    private Context getContext() {
        return (mVoLteService != null) ? mVoLteService.getContext() : ImsGlobal.getInstance();
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : (-1);
    }

    private ImsMmTelManager getImsMmTelManager() {
        int subId = MSimUtils.getSubId(getSlotId());

        if (!SubscriptionManager.isUsableSubscriptionId(subId)) {
            return null;
        }

        ImsManager imsManager = getContext().getSystemService(ImsManager.class);
        return (imsManager != null) ? imsManager.getImsMmTelManager(subId) : null;
    }

    private boolean isVoWiFiSettingEnabled() {
        ImsMmTelManager mmTelManager = getImsMmTelManager();

        try {
            return mmTelManager.isVoWiFiSettingEnabled();
        } catch (Exception e) {
            ImsLog.e(getSlotId(), "isVoWiFiSettingEnabled: " + e.toString());
            return false;
        }
    }

    private int getVoWiFiModeSetting() {
        ImsMmTelManager mmTelManager = getImsMmTelManager();

        try {
            return mmTelManager.getVoWiFiModeSetting();
        } catch (Exception e) {
            ImsLog.e(getSlotId(), "getVoWiFiModeSetting: " + e.toString());
            return ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
        }
    }

    private boolean isVoWiFiRoamingSettingEnabled() {
        ImsMmTelManager mmTelManager = getImsMmTelManager();

        try {
            return mmTelManager.isVoWiFiRoamingSettingEnabled();
        } catch (Exception e) {
            ImsLog.e(getSlotId(), "isVoWiFiRoamingSettingEnabled: " + e.toString());
            return false;
        }
    }

    private boolean isVtSettingEnabled() {
        ImsMmTelManager mmTelManager = getImsMmTelManager();

        try {
            return mmTelManager.isVtSettingEnabled();
        } catch (Exception e) {
            ImsLog.e(getSlotId(), "isVtSettingEnabled: " + e.toString());
            return false;
        }
    }
}
