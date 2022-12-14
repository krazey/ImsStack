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
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.SettingsUtils;
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
    // constants
    private static final int DISABLED = 0;
    private static final int ENABLED = 1;
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
    private int mNetworkMode = -1;
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
        return isVoWIFIEnabled() ? 1 : 0;
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
        return isVoWIFIEnabled();
    }

    @Override
    public boolean isVideoCallEnabled() {
        return isVideoCallSetEnabled();
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

    @Override
    public boolean isNetworkMode3GOnly() {
        return false;
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

        SettingsUtils.registerObserverForSecure(getContext().getContentResolver(),
                Settings.Global.MOBILE_DATA, mMobileDataSettingObserver);

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

        SettingsUtils.registerObserverForCallSettings(getContext(),
                SubscriptionManager.VT_IMS_ENABLED,
                mVideoCallSettingObserver, getSlotId());

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

        SettingsUtils.registerObserverForSecure(getContext().getContentResolver(),
                "data_network_video_calling_status_roaming"
                /*SettingsConstants.Secure.DATA_NETWORK_VIDEO_CALLING_STATUS_ROAMING*/,
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

        SettingsUtils.registerObserverForCallSettings(getContext(),
                SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                mVoLTESettingObserver, getSlotId());

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

        SettingsUtils.registerObserverForGlobal(cr,
                "roaming_hdvoice_enabled"
                /*SettingsConstants.Global.ROAMING_HDVOICE_ENABLED*/,
                mVoLTERoamingObserver);

        if (!ImsGlobal.isOperator(getSlotId(), "SKT")) {
            SettingsUtils.registerObserverForSecure(cr,
                    "data_lte_roaming"
                    /*SettingsConstants.Secure.DATA_LTE_ROAMING*/,
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

        SettingsUtils.registerObserverForCallSettings(getContext(),
                SubscriptionManager.WFC_IMS_ENABLED,
                mVoWIFISettingObserver, getSlotId());

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

        SettingsUtils.registerObserverForCallSettings(getContext(),
                SubscriptionManager.WFC_IMS_MODE,
                mVoWIFIPreferenceObserver, getSlotId());

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

                boolean roamingEnabled = isVoWIFIRoamingEnabled();
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

        SettingsUtils.registerObserverForCallSettings(getContext(),
                SubscriptionManager.WFC_IMS_ROAMING_ENABLED,
                mVoWIFIRoamingSetObserver, getSlotId());

        mVoWIFIRoamingSet = isVoWIFIRoamingEnabled();

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

        SettingsUtils.registerObserverForSecure(getContext().getContentResolver(),
                Settings.Global.DATA_ROAMING,
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

        SettingsUtils.registerObserverForSystem(getContext().getContentResolver(),
                "rtt_option"
                /*SettingsConstants.System.RTT_OPTION*/,
                mRttModeSettingObserver);
        SettingsUtils.registerObserverForSystem(getContext().getContentResolver(),
                "rtt_operation_mode"
                /*SettingsConstants.System.RTT_OPERATION_MODE*/,
                mRttModeSettingObserver);

        onRttModeSettingChanged();
    }

    private void unregisterObserver(ContentObserver co) {
        SettingsUtils.unregisterObserver(getContext().getContentResolver(), co);
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
        int hdVoiceSetting = SettingsUtils.VALUE_NOT_INITIALIZED;
        boolean bVolteSet = (hdVoiceSetting == 1
                || hdVoiceSetting == SettingsUtils.VALUE_NOT_INITIALIZED);

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

        return SettingsUtils.getWFCImsMode(getContext(), getSlotId());
    }

    private String getVoWIFIAddressID() {
        String value = null;
        Cursor c = null;

        try {
            c = getContext().getContentResolver().query(VoWIFI.CONTENT_URI_AID,
                new String[] {"value"},
                "name = ?", new String[]{ VoWIFI.KEY_ADDRESSID }, null);

            if ((c != null) && c.moveToNext()) {
                String buffer = c.getString(0);
                if (!TextUtils.isEmpty(buffer)) {
                    value = buffer;
                }
            }
        } catch (Exception e) {
            ImsLog.e(getSlotId(), e.toString());
        } finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }

        return value;
    }

    private int getRttMode() {
        ContentResolver cr = getContext().getContentResolver();
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
        return SettingsUtils.isMobileDataEnabled(getContext().getContentResolver());
    }

    private boolean isNetworkMode3GOnly(int mode) {
        return (mode < TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO) ? true : false;
    }

    private boolean isNetworkModeSupportsCDMA(int nNetworkMode) {
        return false;
    }

    private boolean isVoLTEEnabled() {
        return false;
    }

    private boolean isVoLTERoamingEnabled() {
        return false;
    }

    private boolean isVoWIFIEnabled() {
        return SettingsUtils.isWFCImsEnabled(getContext(), getSlotId());
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

    private boolean isVoWIFIRoamingEnabled() {
        return SettingsUtils.isWFCImsRoamingEnabled(getContext(), getSlotId());
    }

    private boolean isVideoCallSetEnabled() {
        return SettingsUtils.isVtImsEnabled(getContext(), getSlotId());
    }

    private boolean isVideoCallRoamingSetEnabled() {
        return false;
    }

    private boolean isDataRoamingSettingEnabled() {
        return SettingsUtils.isDataRoamingEnabled(getContext().getContentResolver());
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
                    isVoWIFIEnabled() ? 1 : 0, mVoWIFIPreference);
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
                        isVoWIFIEnabled() ? 1 : 0,
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
        boolean setValue = isVideoCallSetEnabled();

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
        boolean setValue = isVoWIFIEnabled();

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

    private boolean contains(String[] stringArray, String value) {
        if (TextUtils.isEmpty(value)) {
            return false;
        }

        for (String item : stringArray) {
            if (value.contains(item)) {
                ImsLog.d(ImsLog.hiddenString(value) + "is contained");
                return true;
            }
        }
        return false;
    }

    private boolean isKTRoamingUISpec_v1_3_6_applied() {
        return false;
    }

    private Context getContext() {
        return (mVoLteService != null) ? mVoLteService.getContext() : ImsGlobal.getInstance();
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : (-1);
    }

    /** this class is the vowifi related data */
    public static final class VoWIFI {
        public static final  Uri CONTENT_URI_AID = Uri.parse(
                "content://com.xxx.wfcprovider/wfcconfig_att");
        public static final  String KEY_ADDRESSID = "AID";

        public static final int MODE_WIFI_ONLY = 0;
        public static final int MODE_CELL_PREF = 1;
        public static final int MODE_WIFI_PREF = 2;

        public static final int MODE_ROAMING_CELL_PREF = 0;
        public static final int MODE_ROAMING_WIFI_PREF = 1;
    }
}
