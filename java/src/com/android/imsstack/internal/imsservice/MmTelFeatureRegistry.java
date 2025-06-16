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

package com.android.imsstack.internal.imsservice;

import static android.telephony.ims.ImsService.ImsServiceCapability;

import android.annotation.Nullable;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ImsService;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ContentProviderProxy;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsMmTelManagerProxy;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A storage for MmTelFeature's states.
 * This manages the MmTelFeature related data such as SRVCC status,
 * and notifies the components who monitor this class that its related data has changed.
 */
public class MmTelFeatureRegistry {
    /** SRVCC state information */
    public static final int SRVCC_STATE_NONE = -1;
    public static final int SRVCC_STATE_STARTED = 0;
    public static final int SRVCC_STATE_COMPLETED = 1;
    public static final int SRVCC_STATE_FAILED = 2;
    public static final int SRVCC_STATE_CANCELED = 3;

    public static @ImsServiceCapability long getTerminalBasedServiceCapabilities() {
        return ImsService.CAPABILITY_TERMINAL_BASED_CALL_WAITING;
    }

    public static @ImsServiceCapability long getSimultaneousCallingCapabilities() {
        return ImsService.CAPABILITY_SUPPORTS_SIMULTANEOUS_CALLING;
    }

    /**
     * Notifies the components who monitor this class that any states have changed.
     */
    public interface Listener {
        /**
         * Notifies the components who monitor the ImsService related data that
         * the terminal-based call waiting status has changed.
         */
        default void onTerminalBasedCallWaitingStatusChanged() {
        }

        /**
         * Notifies the components who monitor the MmTelFeature related data that
         * the SRVCC state has changed.
         *
         * @param srvccState The new SRVCC state.
         */
        default void onSrvccStateChanged(int srvccState) {
        }

        /**
         * Notifies the components who monitor the advanced calling setting that it's changed.
         */
        default void onAdvancedCallingSettingChanged() {
        }

        /**
         * Notifies the components who monitor the video call setting when it's changed.
         */
        default void onVtSettingChanged() {
        }

        /**
         * Notifies the components who monitor the Wi-Fi calling setting when it's changed.
         */
        default void onVoWiFiSettingChanged() {
        }

        /**
         * Notifies the components who monitor the RTT calling mode when it's changed.
         */
        default void onRttModeChanged() {
        }
    }

    /**
     * A class to access the user's setting related methods of {@link ImsMmTelManager}.
     */
    public class UserSettings {
        private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        private ImsMmTelManagerProxy mMmTelManagerProxy;
        private Uri mAdvancedCallingSettingUri;
        private Uri mVtSettingUri;
        private Uri mVoWiFiSettingUri;
        private Uri mVoWiFiRoamingSettingUri;
        private Uri mVoWiFiModeUri;
        private Uri mVoWiFiRoamingModeUri;
        private Uri mRttModeUri;
        private ContentObserver mSettingsObserver;

        protected void init() {
            int subId = MSimUtils.getSubId(mSlotId);

            if (mSubId != subId) {
                ImsLog.i(mSlotId, "UserSettings#init: " + mSubId + " >> " + subId);
                unregisterSettingsObserver();

                mSubId = subId;
                mMmTelManagerProxy = getImsMmTelManagerProxy();

                if (MSimUtils.isValidSubId(mSubId)) {
                    registerSettingsObserver();
                }
            }
        }

        /**
         * Returns the {@link ContentObserver} for monitoring the user's settings.
         */
        @VisibleForTesting
        protected ContentObserver getContentObserver() {
            return mSettingsObserver;
        }

        /**
         * Checks whether the advanced calling(VoLTE) setting is enabled or not.
         *
         * @return {@code true} if the advanced calling setting is enabled, {@code false} otherwise.
         */
        public boolean isAdvancedCallingSettingEnabled() {
            try {
                return (mMmTelManagerProxy != null)
                        ? mMmTelManagerProxy.isAdvancedCallingSettingEnabled()
                        : false;
            } catch (Exception e) {
                ImsLog.e(mSlotId, "isAdvancedCallingSettingEnabled: " + e.toString());
                return false;
            }
        }

        /**
         * Checks whether the video call(VT) setting is enabled or not.
         *
         * @return {@code true} if the video call setting is enabled, {@code false} otherwise.
         */
        public boolean isVtSettingEnabled() {
            try {
                return (mMmTelManagerProxy != null)
                        ? mMmTelManagerProxy.isVtSettingEnabled()
                        : false;
            } catch (Exception e) {
                ImsLog.e(mSlotId, "isVtSettingEnabled: " + e.toString());
                return false;
            }
        }

        /**
         * Checks whether the Wi-Fi calling setting is enabled or not.
         *
         * @return {@code true} if the Wi-Fi calling setting is enabled, {@code false} otherwise.
         */
        public boolean isVoWiFiSettingEnabled() {
            try {
                return (mMmTelManagerProxy != null)
                        ? mMmTelManagerProxy.isVoWiFiSettingEnabled()
                        : false;
            } catch (Exception e) {
                ImsLog.e(mSlotId, "isVoWiFiSettingEnabled: " + e.toString());
                return false;
            }
        }

        /**
         * Returns the Wi-Fi calling mode.
         *
         * @return The current Wi-Fi calling mode. Possible values are:
         *         {@link ImsMmTelManager#WIFI_MODE_WIFI_ONLY},
         *         {@link ImsMmTelManager#WIFI_MODE_CELLULAR_PREFERRED},
         *         {@link ImsMmTelManager#WIFI_MODE_WIFI_PREFERRED}
         */
        public int getVoWiFiModeSetting() {
            try {
                return (mMmTelManagerProxy != null)
                        ? mMmTelManagerProxy.getVoWiFiModeSetting()
                        : ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
            } catch (Exception e) {
                ImsLog.e(mSlotId, "getVoWiFiModeSetting: " + e.toString());
                return ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
            }
        }

        /**
         * Checks whether the Wi-Fi calling setting is enabled or not at the roaming network.
         *
         * @return {@code true} if the Wi-Fi calling setting is enabled, {@code false} otherwise.
         */
        public boolean isVoWiFiRoamingSettingEnabled() {
            try {
                return (mMmTelManagerProxy != null)
                        ? mMmTelManagerProxy.isVoWiFiRoamingSettingEnabled()
                        : false;
            } catch (Exception e) {
                ImsLog.e(mSlotId, "isVoWiFiRoamingSettingEnabled: " + e.toString());
                return false;
            }
        }

        /**
         * Returns the Wi-Fi calling mode at the roaming network.
         *
         * @return The current Wi-Fi calling mode. Possible values are:
         *         {@link ImsMmTelManager#WIFI_MODE_WIFI_ONLY},
         *         {@link ImsMmTelManager#WIFI_MODE_CELLULAR_PREFERRED},
         *         {@link ImsMmTelManager#WIFI_MODE_WIFI_PREFERRED}
         */
        public int getVoWiFiRoamingModeSetting() {
            try {
                return (mMmTelManagerProxy != null)
                        ? mMmTelManagerProxy.getVoWiFiRoamingModeSetting()
                        : ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
            } catch (Exception e) {
                ImsLog.e(mSlotId, "getVoWiFiRoamingModeSetting: " + e.toString());
                return ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
            }
        }

        private ImsMmTelManagerProxy getImsMmTelManagerProxy() {
            ImsManagerProxy imp =
                    AppContext.getInstance().getSystemServiceProxy(ImsManagerProxy.class);
            return imp.getImsMmTelManagerProxy(mSubId);
        }

        private void registerSettingsObserver() {
            if (mSettingsObserver == null) {
                mSettingsObserver = new ContentObserver(mHandler) {
                    @Override
                    public void onChange(boolean selfChange, @Nullable Uri uri) {
                        if (uri == null) {
                            return;
                        }

                        if (uri.equals(mAdvancedCallingSettingUri)) {
                            updateAdvancedCallingSetting();
                        } else if (uri.equals(mVtSettingUri)) {
                            updateVtSetting();
                        } else if (uri.equals(mVoWiFiSettingUri)) {
                            updateVoWiFiSetting();
                        } else if (uri.equals(mVoWiFiModeUri)) {
                            updateVoWiFiMode();
                        } else if (uri.equals(mVoWiFiRoamingSettingUri)) {
                            updateVoWiFiRoamingSetting();
                        } else if (uri.equals(mVoWiFiRoamingModeUri)) {
                            updateVoWiFiRoamingMode();
                        } else if (uri.equals(mRttModeUri)) {
                            updateRttMode();
                        } else {
                            ImsLog.d(mSlotId, "Unknown URI: " + uri);
                        }
                    }
                };
            }

            ContentProviderProxy cpp = AppContext.getInstance().getContentProviderProxy();

            mAdvancedCallingSettingUri =
                    getUriFor(SubscriptionManager.ADVANCED_CALLING_ENABLED_CONTENT_URI);
            cpp.registerContentObserver(mAdvancedCallingSettingUri, mSettingsObserver);

            mVtSettingUri = getUriFor(SubscriptionManager.VT_ENABLED_CONTENT_URI);
            cpp.registerContentObserver(mVtSettingUri, mSettingsObserver);

            mVoWiFiSettingUri = getUriFor(SubscriptionManager.WFC_ENABLED_CONTENT_URI);
            cpp.registerContentObserver(mVoWiFiSettingUri, mSettingsObserver);

            mVoWiFiModeUri = getUriFor(SubscriptionManager.WFC_MODE_CONTENT_URI);
            cpp.registerContentObserver(mVoWiFiModeUri, mSettingsObserver);

            mVoWiFiRoamingSettingUri =
                    getUriFor(SubscriptionManager.WFC_ROAMING_ENABLED_CONTENT_URI);
            cpp.registerContentObserver(mVoWiFiRoamingSettingUri, mSettingsObserver);

            mVoWiFiRoamingModeUri = getUriFor(SubscriptionManager.WFC_ROAMING_MODE_CONTENT_URI);
            cpp.registerContentObserver(mVoWiFiRoamingModeUri, mSettingsObserver);

            mRttModeUri = Settings.Secure.getUriFor(RTT_MODE_SETTING);
            cpp.registerContentObserver(mRttModeUri, mSettingsObserver);
        }

        private void unregisterSettingsObserver() {
            if (mSettingsObserver != null) {
                AppContext.getInstance().getContentProviderProxy()
                        .unregisterContentObserver(mSettingsObserver);

                mAdvancedCallingSettingUri = null;
                mVtSettingUri = null;
                mVoWiFiSettingUri = null;
                mVoWiFiModeUri = null;
                mVoWiFiRoamingSettingUri = null;
                mVoWiFiRoamingModeUri = null;
                mRttModeUri = null;
                mSettingsObserver = null;
            }
        }

        private Uri getUriFor(Uri uri) {
            return Uri.withAppendedPath(uri, String.valueOf(mSubId));
        }
    }

    /**
     * For the dialer's RTT configuration, use "dialer_rtt_configuration".
     * Otherwise, use {@link Settings.Secure#RTT_CALLING_MODE}.
     */
    @VisibleForTesting
    public static final String RTT_MODE_SETTING = "dialer_rtt_configuration";
    private final int mSlotId;
    private volatile boolean mTerminalBasedCallWaitingEnabled =
            (ImsService.CAPABILITY_TERMINAL_BASED_CALL_WAITING
            & getTerminalBasedServiceCapabilities()) > 0;
    private volatile int mSrvccState = SRVCC_STATE_NONE;
    private volatile int mTtyMode = TelecomManager.TTY_MODE_OFF;
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    // User's settings
    private final Handler mHandler;
    private final UserSettings mUserSettings = new UserSettings();
    private volatile boolean mAdvancedCallingSettingEnabled;
    private volatile boolean mVtSettingEnabled;
    private volatile boolean mVoWiFiSettingEnabled;
    private volatile int mVoWiFiMode = ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
    private volatile boolean mVoWiFiRoamingSettingEnabled;
    private volatile int mVoWiFiRoamingMode = ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
    private volatile int mRttMode = 0; // OFF

    MmTelFeatureRegistry(int slotId) {
        mSlotId = slotId;
        mHandler = new Handler(AppContext.getInstance().getMainLooper());
    }

    /**
     * Initializes the user's settings for MmTelFeature.
     */
    public void initUserSettings() {
        // Updates all the settings.
        mUserSettings.init();
        updateAdvancedCallingSetting();
        updateVtSetting();
        updateVoWiFiSettings();
        updateRttMode();
    }

    /**
     * Checks if the terminal-based call waiting is enabled or not.
     *
     * @return true if the terminal-based call waiting is enabled, false otherwise.
     */
    public boolean isTerminalBasedCallWaitingEnabled() {
        return mTerminalBasedCallWaitingEnabled;
    }

    /**
     * Sets the terminal-based call waiting status.
     *
     * @param enabled The flag specifying that the service is capable.
     */
    public void setTerminalBasedCallWaitingStatus(boolean enabled) {
        if (mTerminalBasedCallWaitingEnabled != enabled) {
            ImsLog.i(mSlotId, "setTerminalBasedCallWaitingStatus: "
                    + mTerminalBasedCallWaitingEnabled + " >> " + enabled);
            mTerminalBasedCallWaitingEnabled = enabled;
            notifyTerminalBasedCallWaitingStatusChanged();
        }
    }

    /**
     * Returns the current SRVCC state.
     *
     * When the SRVCC state is transited to the state except for {@link #SRVCC_STATE_STARTED},
     * the change notification sends the currently changed state, but the managed SRVCC state
     * is set to {@link #SRVCC_STATE_NONE} because the SRVCC operation is finished.
     *
     * @return The SRVCC state. Valid values are
     *         {@link #SRVCC_STATE_STARTED},
     *         {@link #SRVCC_STATE_NONE}.
     */
    public int getSrvccState() {
        return mSrvccState;
    }

    /**
     * Sets the SRVCC state.
     *
     * @param srvccState The SRVCC state to be set.
     */
    public void setSrvccState(int srvccState) {
        if (mSrvccState != srvccState) {
            ImsLog.i(mSlotId, "setSrvccState: " + srvccStateToString(mSrvccState)
                    + " >> " + srvccStateToString(srvccState));
            if (srvccState == SRVCC_STATE_STARTED) {
                mSrvccState = srvccState;
            } else {
                mSrvccState = SRVCC_STATE_NONE;
            }
            notifySrvccStateChanged(srvccState);
        }
    }

    /**
     * Returns the preferred TTY mode.
     *
     * @return The current TTY mode. Possible values are:
     *         {@link TelecomManager#TTY_MODE_OFF}
     *         {@link TelecomManager#TTY_MODE_FULL}
     *         {@link TelecomManager#TTY_MODE_HCO}
     *         {@link TelecomManager#TTY_MODE_VCO}
     */
    public int getTtyMode() {
        return mTtyMode;
    }

    /**
     * Sets the preferred TTY mode. This is the preferred TTY mode that the user sets in the call
     * settings screen.
     *
     * @param ttyMode The current TTY mode. Possible values are:
     *                {@link TelecomManager#TTY_MODE_OFF}
     *                {@link TelecomManager#TTY_MODE_FULL}
     *                {@link TelecomManager#TTY_MODE_HCO}
     *                {@link TelecomManager#TTY_MODE_VCO}
     */
    public void setTtyMode(int ttyMode) {
        if (mTtyMode != ttyMode) {
            ImsLog.i(mSlotId, "setTtyMode: " + mTtyMode + " >> " + ttyMode);
            mTtyMode = ttyMode;
        }
    }

    /**
     * Returns the {@link UserSettings} instance to access the setting related methods of
     * {@link ImsMmTelManager}.
     *
     * @return A {@link UserSettings} instance.
     */
    public UserSettings getUserSettings() {
        synchronized (mUserSettings) {
            // Checks whether the subscription is valid or not first
            // and returns the {@link UserSettings} instance.
            mUserSettings.init();
        }

        return mUserSettings;
    }

    /**
     * Checks whether the advanced calling(VoLTE) setting is enabled or not.
     *
     * @return {@code true} if the advanced calling setting is enabled, {@code false} otherwise.
     */
    public boolean isAdvancedCallingSettingEnabled() {
        return mAdvancedCallingSettingEnabled;
    }

    /**
     * Checks whether the video call(VT) setting is enabled or not.
     *
     * @return {@code true} if the video call setting is enabled, {@code false} otherwise.
     */
    public boolean isVtSettingEnabled() {
        return mVtSettingEnabled;
    }

    /**
     * Checks whether the Wi-Fi calling setting is enabled or not.
     *
     * @return {@code true} if the Wi-Fi calling setting is enabled, {@code false} otherwise.
     */
    public boolean isVoWiFiSettingEnabled() {
        return mVoWiFiSettingEnabled;
    }

    /**
     * Returns the Wi-Fi calling mode.
     *
     * @return The current Wi-Fi calling mode. Possible values are:
     *         {@link ImsMmTelManager#WIFI_MODE_WIFI_ONLY},
     *         {@link ImsMmTelManager#WIFI_MODE_CELLULAR_PREFERRED},
     *         {@link ImsMmTelManager#WIFI_MODE_WIFI_PREFERRED}
     */
    public int getVoWiFiModeSetting() {
        return mVoWiFiMode;
    }

    /**
     * Checks whether the Wi-Fi calling setting is enabled or not at the roaming network.
     *
     * @return {@code true} if the Wi-Fi calling setting is enabled, {@code false} otherwise.
     */
    public boolean isVoWiFiRoamingSettingEnabled() {
        return mVoWiFiRoamingSettingEnabled;
    }

    /**
     * Returns the Wi-Fi calling mode at the roaming network.
     *
     * @return The current Wi-Fi calling mode. Possible values are:
     *         {@link ImsMmTelManager#WIFI_MODE_WIFI_ONLY},
     *         {@link ImsMmTelManager#WIFI_MODE_CELLULAR_PREFERRED},
     *         {@link ImsMmTelManager#WIFI_MODE_WIFI_PREFERRED}
     */
    public int getVoWiFiRoamingModeSetting() {
        return mVoWiFiRoamingMode;
    }

    /**
     * Returns the RTT calling mode.
     *
     * @return The RTT calling mode (1 if the RTT is enabled, 0 otherwise).
     */
    public int getRttMode() {
        return mRttMode;
    }

    /**
     * Reloads all the user's settings.
     * This method will update the user's settings based on its working thread.
     */
    public void reloadAllUserSettings() {
        mHandler.post(() -> {
            synchronized (mUserSettings) {
                mUserSettings.init();
            }

            updateAdvancedCallingSetting();
            updateVtSetting();
            updateVoWiFiSettings();
            updateRttMode();
        });
    }

    /**
     * Adds the listener to monitor the state of this class.
     *
     * @param listener The listener to be aded.
     */
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    private void notifySrvccStateChanged(int srvccState) {
        for (Listener l : mListeners) {
            l.onSrvccStateChanged(srvccState);
        }
    }

    private void notifyTerminalBasedCallWaitingStatusChanged() {
        for (Listener l : mListeners) {
            l.onTerminalBasedCallWaitingStatusChanged();
        }
    }

    private void notifyAdvancedCallingSettingChanged() {
        for (Listener l : mListeners) {
            l.onAdvancedCallingSettingChanged();
        }
    }

    private void updateAdvancedCallingSetting() {
        boolean enabled = mUserSettings.isAdvancedCallingSettingEnabled();

        if (mAdvancedCallingSettingEnabled != enabled) {
            ImsLog.i(mSlotId, "isAdvancedCallingSettingEnabled: " + mAdvancedCallingSettingEnabled
                    + " >> " + enabled);
            mAdvancedCallingSettingEnabled = enabled;
            notifyAdvancedCallingSettingChanged();
        }
    }

    private void notifyVtSettingChanged() {
        for (Listener l : mListeners) {
            l.onVtSettingChanged();
        }
    }

    private void updateVtSetting() {
        boolean enabled = mUserSettings.isVtSettingEnabled();

        if (mVtSettingEnabled != enabled) {
            ImsLog.i(mSlotId, "isVtSettingEnabled: " + mVtSettingEnabled + " >> " + enabled);
            mVtSettingEnabled = enabled;
            notifyVtSettingChanged();
        }
    }

    private void notifyVoWiFiSettingChanged() {
        for (Listener l : mListeners) {
            l.onVoWiFiSettingChanged();
        }
    }

    private void updateVoWiFiSetting() {
        boolean enabled = mUserSettings.isVoWiFiSettingEnabled();

        if (mVoWiFiSettingEnabled != enabled) {
            ImsLog.i(mSlotId, "updateVoWiFiSetting: " + mVoWiFiSettingEnabled + " >> " + enabled);
            mVoWiFiSettingEnabled = enabled;
            notifyVoWiFiSettingChanged();
        }
    }

    private void updateVoWiFiMode() {
        int voWiFiMode = mUserSettings.getVoWiFiModeSetting();

        if (mVoWiFiMode != voWiFiMode) {
            ImsLog.i(mSlotId, "updateVoWiFiMode: " + mVoWiFiMode + " >> " + voWiFiMode);
            mVoWiFiMode = voWiFiMode;
            notifyVoWiFiSettingChanged();
        }
    }

    private void updateVoWiFiRoamingSetting() {
        boolean enabled = mUserSettings.isVoWiFiRoamingSettingEnabled();

        if (mVoWiFiRoamingSettingEnabled != enabled) {
            ImsLog.i(mSlotId, "updateVoWiFiRoamingSetting: " + mVoWiFiSettingEnabled
                    + " >> " + enabled);
            mVoWiFiRoamingSettingEnabled = enabled;
            notifyVoWiFiSettingChanged();
        }
    }

    private void updateVoWiFiRoamingMode() {
        int voWiFiMode = mUserSettings.getVoWiFiRoamingModeSetting();

        if (mVoWiFiRoamingMode != voWiFiMode) {
            ImsLog.i(mSlotId, "updateVoWiFiRoamingMode: " + mVoWiFiRoamingMode
                    + " >> " + voWiFiMode);
            mVoWiFiRoamingMode = voWiFiMode;
            notifyVoWiFiSettingChanged();
        }
    }

    private void updateVoWiFiSettings() {
        updateVoWiFiSetting();
        updateVoWiFiMode();
        updateVoWiFiRoamingSetting();
        updateVoWiFiRoamingMode();
    }

    private void notifyRttModeChanged() {
        for (Listener l : mListeners) {
            l.onRttModeChanged();
        }
    }

    private void updateRttMode() {
        int rttMode = AppContext.getInstance().getContentProviderProxy().getSecureSettings()
                .getInt(RTT_MODE_SETTING, 0);

        if (mRttMode != rttMode) {
            ImsLog.i(mSlotId, "updateRttMode: " + mRttMode + " >> " + rttMode);
            mRttMode = rttMode;
            notifyRttModeChanged();
        }
    }

    private static String srvccStateToString(int srvccState) {
        switch (srvccState) {
            case SRVCC_STATE_STARTED:
                return "STARTED";
            case SRVCC_STATE_COMPLETED:
                return "COMPLETED";
            case SRVCC_STATE_FAILED:
                return "FAILED";
            case SRVCC_STATE_CANCELED:
                return "CANCELED";
            default:
                return "NONE";
        }
    }
}
