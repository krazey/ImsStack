/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.enabler.aos;

import android.annotation.NonNull;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.icu.text.SimpleDateFormat;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CellSignalStrength;
import android.telephony.CellSignalStrengthCdma;
import android.telephony.CellSignalStrengthGsm;
import android.telephony.CellSignalStrengthLte;
import android.telephony.CellSignalStrengthNr;
import android.telephony.CellSignalStrengthWcdma;
import android.telephony.DataSpecificRegistrationInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.telephony.VopsSupportInfo;
import android.telephony.data.ApnSetting;
import android.widget.Toast;

import com.android.imsstack.R;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.test.DebugScreen;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * This class updates and manages information to be displayed on the debug screen.
 * It registers and unregisters listeners to receive the information.
 */
public class AosDebug implements IAosDebug {

    @VisibleForTesting
    static final int DEBUG_AIRPLANE_MODE_CHANGED = 1000;
    @VisibleForTesting
    static final int DEBUG_SUBSCRIPTION_CHANGED = 1001;
    @VisibleForTesting
    static final int DEBUG_SIGNALSTRENGTHS_CHANGED = 1002;
    @VisibleForTesting
    static final int DEBUG_WIFI_CONNECTIVITY_CHANGED = 1003;
    @VisibleForTesting
    static final int DEBUG_SERVICE_STATE_CHANGED = 1004;
    @VisibleForTesting
    static final int DEBUG_PRECISE_DATA_CONNECTION_CHANGED = 1005;
    @VisibleForTesting
    static final int DEBUG_NOTIFY_REGISTERED = 1006;
    @VisibleForTesting
    static final int DEBUG_NOTIFY_DEREGISTERED = 1007;
    @VisibleForTesting
    static final int DEBUG_NOTIFY_CAPABILITIES_UPDATED = 1008;
    private static final String NOTIFICATION_CHANNEL_ID_DEBUG = "notification_channel_id_debug";
    private static final int NOTIFICATION_ID_DEBUG = 0;
    public static final int REQUEST_CODE_DEBUG = 1;
    private final int mSlotId;
    @VisibleForTesting
    Context mContext;
    @VisibleForTesting
    DebugData mDebugData;
    @VisibleForTesting
    DebugHandler mHandler;
    @VisibleForTesting
    DebugBroadcastReceiver mDebugBroadcastReceiver;
    @VisibleForTesting
    SignalStrengthsListener mSignalStrengthsListener;
    @VisibleForTesting
    ConnectivityCallback mConnectivityCallback;
    @VisibleForTesting
    DebugImsPhoneStateListener mImsPhoneStateListener;
    @VisibleForTesting
    RegistrationListener mRegistrationListener;
    @VisibleForTesting
    IAosRegistration mAosRegistration;
    @VisibleForTesting
    NotificationManager mNotificationManager;
    @VisibleForTesting
    NativeStateListener mNativeStateListener;
    @VisibleForTesting
    Sim.Listener mSimListener;
    @VisibleForTesting
    int mSubId = MSimUtils.INVALID_SUB_ID;
    @VisibleForTesting
    String mOperator = DebugData.STR_EMPTY;
    @VisibleForTesting
    String mCountry = DebugData.STR_EMPTY;

    private static final int INVALID_RSSI = -127;

    public AosDebug(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init() {
        logi(mSlotId, "init");

        mContext = AppContext.getInstance();
        mNotificationManager = getNotificationManager();
        mDebugData = createDebugData();
        updateCarrierInfo();

        mHandler = createDebugHandler();

        mNativeStateListener = createNativeStateListener();
        mNativeStateListener.register();

        mSimListener = new Sim.Listener() {
            @Override
            public void onSimStateChanged() {
                Message.obtain(mHandler, DEBUG_SUBSCRIPTION_CHANGED).sendToTarget();
            }
        };

        SimInterface si = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (si != null) {
            si.addListener(mSimListener);
        }

        mAosRegistration = AosFactory.getInstance().getAosRegistration(mSlotId);
        if (mAosRegistration != null) {
            mRegistrationListener = createRegistrationListener();
            mAosRegistration.addListener(mRegistrationListener);
        }

        mDebugBroadcastReceiver = createDebugBroadcastReceiver();
        mDebugBroadcastReceiver.register();

        mSubId = MSimUtils.getSubId(mSlotId);
        mDebugData.putInt(DebugKey.SUB_ID, mSubId);

        mSignalStrengthsListener = createSignalStrengthsListener();
        mSignalStrengthsListener.register();

        mConnectivityCallback = createConnectivityCallback();
        mConnectivityCallback.register();

        mImsPhoneStateListener = createDebugImsPhoneStateListener();
        mImsPhoneStateListener.setListener();
    }

    @Override
    public void cleanup() {
        logi(mSlotId, "cleanup");

        if (mImsPhoneStateListener != null) {
            mImsPhoneStateListener.removeListener();
            mImsPhoneStateListener = null;
        }

        if (mConnectivityCallback != null) {
            mConnectivityCallback.unregister();
            mConnectivityCallback = null;
        }

        if (mSignalStrengthsListener != null) {
            mSignalStrengthsListener.unregister();
            mSignalStrengthsListener = null;
        }

        if (mDebugBroadcastReceiver != null) {
            mDebugBroadcastReceiver.unregister();
            mDebugBroadcastReceiver = null;
        }

        if (mAosRegistration != null) {
            if (mRegistrationListener != null) {
                mAosRegistration.removeListener(mRegistrationListener);
                mRegistrationListener = null;
            }
            mAosRegistration = null;
        }

        if (mSimListener != null) {
            SimInterface si = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (si != null) {
                si.removeListener(mSimListener);
            }
            mSimListener = null;
        }

        if (mNativeStateListener != null) {
            mNativeStateListener.unregister();
            mNativeStateListener = null;
        }

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler.getLooper().quit();
            mHandler = null;
        }

        mDebugData = null;
        mNotificationManager = null;
        mContext = null;
    }

    @Override
    public void showOrDismissNotification(Activity activity) {
        boolean isEnabled = isDebugScreenEnabled();
        logi(mSlotId, "showOrDismissNotification - is enabled: " + isEnabled);
        if (!isEnabled) {
            dismissNotification();
            return;
        }

        // When Activity is null, it is called internally.
        // In this case, the permission check is skipped.
        if (activity == null || checkPermission()) {
            createNotificationChannel();
            sendNotification();
        } else {
            requestPermission(activity);
        }
    }

    @VisibleForTesting
    boolean isDebugScreenEnabled() {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED, mSlotId);
    }

    private void dismissNotification() {
        if (mNotificationManager != null) {
            mNotificationManager.cancel(NOTIFICATION_ID_DEBUG + mSlotId);
            mNotificationManager.deleteNotificationChannel(
                    NOTIFICATION_CHANNEL_ID_DEBUG + mSlotId);
        }
    }

    @Override
    public void notifyPermissionsResult(
            int requestCode, String[] permissions, int[] grantResults, Activity activity) {
        logi(mSlotId, "notifyPermissionsResult");

        if (requestCode == REQUEST_CODE_DEBUG && grantResults.length > 0) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                handlePermissionGranted();
            } else {
                handlePermissionDenied(activity);
            }
        }
    }

    private void handlePermissionGranted() {
        createNotificationChannel();
        sendNotification();
    }

    @VisibleForTesting
    void handlePermissionDenied(Activity activity) {
        Toast.makeText(activity, "Notification permission was denied!", Toast.LENGTH_SHORT).show();
        ImsPrivateProperties.Persistent.setBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED, false, mSlotId);
    }

    @Override
    public String getDebugMessage() {
        StringBuilder sb = new StringBuilder(512);
        appendMessage(sb, "Last Update Time", getCurrentTime() + "\n");

        // IMS
        appendMessage(sb, "SlotId/SubId", mSlotId + "/" + mDebugData.get(DebugKey.SUB_ID));
        appendMessage(sb, "Operator/Country", mOperator + "/" + mCountry);

        if (DebugData.STR_IMS_REGISTERED.equals(mDebugData.get(DebugKey.REGISTER))) {
            appendMessage(sb, "IMS Status", DebugData.STR_IMS_REGISTERED
                    + "(" + mDebugData.get(DebugKey.REGISTER_TIME) + ")");
            appendMessage(sb, "Registered FeatureTag", mDebugData.get(DebugKey.FEATURES));
        } else {
            appendMessage(sb, "IMS Status", DebugData.STR_IMS_DEREGISTERED
                    + "(" + mDebugData.get(DebugKey.DEREGISTER_TIME) + ")");
            appendMessage(sb, "Deregistered Reason",
                    mDebugData.get(DebugKey.DEREGISTER_REASON));
        }

        appendMessage(sb, "Registered Network",
                mDebugData.get(DebugKey.REGISTERED_NETWORK_TYPE));
        appendMessage(sb, "Capabilities", "");
        sb.append(mDebugData.get(DebugKey.CAPABILITIES)).append("\n");

        // Data Connection
        sb.append(" # Data Connection #\n");
        String dataConnectionState = mDebugData.get(DebugKey.DATA_CONNECTION_STATE);
        appendMessage(sb, "Connection State", dataConnectionState);

        if (!dataConnectionState.equals(getDataStateToString(TelephonyManager.DATA_DISCONNECTED))
                && !dataConnectionState.equals(DebugData.STR_EMPTY)) {
            appendMessage(sb, "Network Type", mDebugData.get(DebugKey.NETWORK_TYPE));
            appendMessage(sb, "IP Addresses", "");
            sb.append(" -").append(mDebugData.get(DebugKey.IP_ADDRESSES)).append("\n");
            appendMessage(sb, "Interface Name", mDebugData.get(DebugKey.INTERFACE_NAME));
            appendMessage(sb, "MTU", mDebugData.get(DebugKey.MTU));
            appendMessage(sb, "APN Name/Entry", mDebugData.get(DebugKey.APN_NAME) + "/"
                    + mDebugData.get(DebugKey.APN_ENTRY_NAME));
            appendMessage(sb, "APN Types", mDebugData.get(DebugKey.APN_TYPES));
            appendMessage(sb, "P-CSCF", mDebugData.get(DebugKey.PCSCF_ADDRESSES));
            appendMessage(sb, "Reg State (Voice/Data)",
                    mDebugData.get(DebugKey.SERVICE_STATE) + "/"
                            + mDebugData.get(DebugKey.DATA_REG_STATE));

            String cellularDataRAT = mDebugData.get(DebugKey.CELLULAR_DATA_RAT);
            if (cellularDataRAT.equals(getNetworkTypeToString(TelephonyManager.NETWORK_TYPE_LTE))) {
                appendMessage(sb, "LTE Attach Type",
                        mDebugData.get(DebugKey.LTE_ATTACH_TYPE));
            }

            appendMessage(sb, "Roaming State", mDebugData.get(DebugKey.ROAMING_STATE));
            appendMessage(sb, "Roaming Type(Voice/Data)",
                    mDebugData.get(DebugKey.VOICE_ROAMING_TYPE) + "/"
                            + mDebugData.get(DebugKey.DATA_ROAMING_TYPE));
            appendMessage(sb, "RAT (Voice/Data)", mDebugData.get(DebugKey.VOICE_RAT)
                    + "/" + cellularDataRAT);
            appendMessage(sb, "Network Operator", mDebugData.get(DebugKey.NETWORK_OPERATOR)
                    + "(" + mDebugData.get(DebugKey.NETWORK_OPERATOR_NUMERIC) + ")");
            appendMessage(sb, "VOPS", mDebugData.get(DebugKey.NETWORK_SUPPORT_VOPS));
            appendMessage(sb, "EMCBS", mDebugData.get(DebugKey.NETWORK_SUPPORT_EMCBS));

            StringBuilder sbSignal = new StringBuilder();
            String level = mDebugData.get(DebugKey.UTRAN_LEVEL);
            String dbm = mDebugData.get(DebugKey.UTRAN_DBM);
            String rsrp = mDebugData.get(DebugKey.EUTRAN_RSRP);
            String rsrq = mDebugData.get(DebugKey.EUTRAN_RSRQ);
            String ssrsrp = mDebugData.get(DebugKey.NGRAN_SSRSRP);
            String ssrsrq = mDebugData.get(DebugKey.NGRAN_SSRSRQ);

            if (!level.equals(DebugData.STR_EMPTY) || !dbm.equals(DebugData.STR_EMPTY)) {
                appendMessage(sbSignal, " -UTRAN(Level/dbm)", level + " / " + dbm + " dBm");
            }
            if (!rsrp.equals(DebugData.STR_EMPTY) || !rsrq.equals(DebugData.STR_EMPTY)) {
                appendMessage(sbSignal, " -EUTRN(RSRP/RSRQ)", rsrp + " dBm/" + rsrq + " dB");
            }
            if (!ssrsrp.equals(DebugData.STR_EMPTY) || !ssrsrq.equals(DebugData.STR_EMPTY)) {
                appendMessage(sbSignal,
                        " -NGRAN(SSRSRP/SSRSRQ)", ssrsrp + " dBm/" + ssrsrq + " dB");
            }

            if (sbSignal.length() > 0) {
                sb.append(" Signal Strength\n");
                sb.append(sbSignal);
            }
        }

        // WiFi
        sb.append("\n# WiFi #\n");
        String wifiConnectionState = mDebugData.get(DebugKey.WIFI_CONNECTION_STATE);
        appendMessage(sb, "Connection State", wifiConnectionState);

        if (wifiConnectionState.equals(DebugData.STR_CONNECTED)) {
            appendMessage(sb, "Addresses", mDebugData.get(DebugKey.WIFI_ADDRESSES));
            appendMessage(sb, "Interface name", mDebugData.get(DebugKey.WIFI_INTERFACE_NAME));
            appendMessage(sb, "RSSI", mDebugData.get(DebugKey.WIFI_RSSI));
            appendMessage(sb, "BSSID/SSID", mDebugData.get(DebugKey.WIFI_BSSID) + "/"
                    + mDebugData.get(DebugKey.WIFI_SSID));
            appendMessage(sb, "MAC Address", mDebugData.get(DebugKey.WIFI_MAC_ADDRESS));
        }

        return sb.toString();
    }

    private void appendMessage(StringBuilder sb, String key, String value) {
        sb.append(" ").append(key).append(": ").append(value).append("\n");
    }

    @VisibleForTesting
    void updateCarrierInfo() {
        SimCarrierId cid = getCarrierId();
        if (cid != null) {
            ImsCarrierResolver.Carrier c = ImsCarrierResolver.getCarrierFromCarrierId(cid);
            mOperator = c.getOperator();
            mCountry = c.getCountry();
        }
    }

    @VisibleForTesting
    boolean checkPermission() {
        return mContext.checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS)
                == PackageManager.PERMISSION_GRANTED;
    }

    private void createNotificationChannel() {
        if (mNotificationManager != null) {
            String channelId = NOTIFICATION_CHANNEL_ID_DEBUG + mSlotId;
            NotificationChannel channel = new NotificationChannel(
                    channelId,
                    "Register Notification",
                    NotificationManager.IMPORTANCE_HIGH);
            channel.enableLights(true);
            channel.setLightColor(Color.RED);
            channel.enableVibration(true);
            channel.setDescription("ImsStack debug channel");
            mNotificationManager.createNotificationChannel(channel);
        }
    }

    @VisibleForTesting
    void sendNotification() {
        if (mNotificationManager != null) {
            Notification.Builder nb = getNotificationBuilder();
            int notificationId = NOTIFICATION_ID_DEBUG + mSlotId;
            mNotificationManager.notify(notificationId, nb.build());
        }
    }

    @VisibleForTesting
    Notification.Builder getNotificationBuilder() {
        updateCarrierInfo();

        Intent intent = new Intent(mContext, DebugScreen.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra(MSimUtils.EXTRA_KEY_SLOT_ID, mSlotId);

        PendingIntent pendingIntent = PendingIntent.getActivity(
                mContext, NOTIFICATION_ID_DEBUG + mSlotId, intent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        String content =
                (mDebugData.get(DebugKey.REGISTER).equals(DebugData.STR_IMS_REGISTERED))
                        ? DebugData.STR_IMS_REGISTERED + "-"
                        + mDebugData.get(DebugKey.REGISTERED_NETWORK_TYPE) + "\n"
                        + mDebugData.get(DebugKey.FEATURES) : DebugData.STR_IMS_DEREGISTERED;

        return new Notification.Builder(mContext, NOTIFICATION_CHANNEL_ID_DEBUG + mSlotId)
                .setContentTitle("[" + (mSlotId + 1) + "]" + mOperator + "/" + mCountry)
                .setContentText(content)
                .setSmallIcon(R.drawable.ic_notification_ims)
                .setOngoing(true)
                .setContentIntent(pendingIntent);
    }

    @VisibleForTesting
    void requestPermission(Activity activity) {
        if (activity != null) {
            activity.requestPermissions(
                    new String[] {android.Manifest.permission.POST_NOTIFICATIONS},
                    REQUEST_CODE_DEBUG);
        }
    }

    @VisibleForTesting
    String getCurrentTime() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",
                java.util.Locale.getDefault()).format(System.currentTimeMillis());
    }

    @VisibleForTesting
    String getCapabilitiesListToString(IAosRegistration.CapabilityPairs pairs) {
        if (pairs == null) {
            return DebugData.STR_EMPTY;
        }

        StringBuilder sb = new StringBuilder();
        for (Map.Entry<NetworkType, Integer> entry : pairs.getCapabilities().entrySet()) {
            sb.append("  -").append(entry.getKey().toString());
            sb.append(": ")
                    .append(IAosRegistrationListener.Capability.toString(entry.getValue()))
                    .append("\n");
        }
        return sb.toString();
    }

    private void updateNetworkType(NetworkType networkType) {
        mDebugData.put(DebugKey.REGISTERED_NETWORK_TYPE, networkType.toString());
    }

    @VisibleForTesting
    void updateRegisteredData(NetworkType networkType, int featureTagBits) {
        mDebugData.put(DebugKey.REGISTER, DebugData.STR_IMS_REGISTERED);
        mDebugData.put(DebugKey.REGISTER_TIME, getCurrentTime());
        updateNetworkType(networkType);
        mDebugData.put(DebugKey.FEATURES,
                IAosRegistrationListener.FeatureTagMask.toString(featureTagBits));
        showOrDismissNotification(null);
    }

    @VisibleForTesting
    void updateDeregisterData(NetworkType networkType, int reason) {
        mDebugData.put(DebugKey.REGISTER, DebugData.STR_IMS_DEREGISTERED);
        mDebugData.put(DebugKey.DEREGISTER_TIME, getCurrentTime());
        updateNetworkType(networkType);
        mDebugData.put(DebugKey.FEATURES, IAosRegistrationListener.FeatureTagMask.toString(
                IAosRegistrationListener.FeatureTagMask.NONE));
        mDebugData.put(DebugKey.DEREGISTER_REASON,
                IAosRegistrationListener.ReasonCode.of(reason).toString());

        showOrDismissNotification(null);
    }

    @VisibleForTesting
    void selfCheckDebugNotification() {
        logi(mSlotId, "selfCheckDebugNotification");
        showOrDismissNotification(null);
    }

    @VisibleForTesting
    void updateSubscription() {
        SimInterface si = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (si == null || !si.isSimLoadCompleted()) {
            return;
        }

        int subId = si.getSubId();
        if (mSubId == subId) {
            return;
        }

        mSubId = subId;
        logd(mSlotId, "updateSubscription :: subId=" + subId);
        mDebugData.putInt(DebugKey.SUB_ID, mSubId);

        mSignalStrengthsListener.unregister();
        mSignalStrengthsListener.register();
    }

    @VisibleForTesting
    void updateSignalStrengthData(CellSignalStrength cs, int network) {

        switch (network) {
            case AccessNetworkType.UTRAN -> {
                mDebugData.put(DebugKey.UTRAN_LEVEL, getSignalStrength(
                        DebugKey.UTRAN_LEVEL, cs));
                mDebugData.put(DebugKey.UTRAN_DBM, getSignalStrength(
                        DebugKey.UTRAN_DBM, cs));
            }
            case AccessNetworkType.EUTRAN -> {
                mDebugData.put(DebugKey.EUTRAN_RSRP, getSignalStrength(
                        DebugKey.EUTRAN_RSRP, cs));
                mDebugData.put(DebugKey.EUTRAN_RSRQ, getSignalStrength(
                        DebugKey.EUTRAN_RSRQ, cs));
            }
            case AccessNetworkType.NGRAN -> {
                mDebugData.put(DebugKey.NGRAN_SSRSRP, getSignalStrength(
                        DebugKey.NGRAN_SSRSRP, cs));
                mDebugData.put(DebugKey.NGRAN_SSRSRQ, getSignalStrength(
                        DebugKey.NGRAN_SSRSRQ, cs));
            }
            default -> logd(mSlotId, "Network = " + network + " not handled.");
        }
    }

    private String getSignalStrength(DebugKey type, CellSignalStrength css) {
        int ss = Integer.MAX_VALUE;

        if (css instanceof CellSignalStrengthWcdma
                && (type == DebugKey.UTRAN_LEVEL || type == DebugKey.UTRAN_DBM)) {
            ss = (type == DebugKey.UTRAN_LEVEL)
                    ? ((CellSignalStrengthWcdma) css).getLevel() : css.getDbm();
        } else if (css instanceof CellSignalStrengthLte
                && (type == DebugKey.EUTRAN_RSRP || type == DebugKey.EUTRAN_RSRQ)) {
            ss = (type == DebugKey.EUTRAN_RSRP)
                    ? ((CellSignalStrengthLte) css).getRsrp()
                    : ((CellSignalStrengthLte) css).getRsrq();
        } else if (css instanceof CellSignalStrengthNr
                && (type == DebugKey.NGRAN_SSRSRP || type == DebugKey.NGRAN_SSRSRQ)) {
            ss = (type == DebugKey.NGRAN_SSRSRP)
                    ? ((CellSignalStrengthNr) css).getSsRsrp()
                    : ((CellSignalStrengthNr) css).getSsRsrq();
        }

        return (ss != Integer.MAX_VALUE) ? String.valueOf(ss) : DebugData.STR_EMPTY;
    }

    @VisibleForTesting
    void updateServiceState(int state) {
        String text = switch (state) {
            case ServiceState.STATE_IN_SERVICE -> "In Service";
            case ServiceState.STATE_OUT_OF_SERVICE -> "Out of Service";
            case ServiceState.STATE_EMERGENCY_ONLY -> "Emergency call only";
            case ServiceState.STATE_POWER_OFF -> "Radio off";
            default -> DebugData.STR_EMPTY;
        };

        mDebugData.put(DebugKey.SERVICE_STATE, text);
    }

    @VisibleForTesting
    void updateVoiceRat(ServiceState ss) {
        NetworkRegistrationInfo nri =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_CS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        mDebugData.put(DebugKey.VOICE_RAT,
                getNetworkTypeToString((nri == null)
                        ? TelephonyManager.NETWORK_TYPE_UNKNOWN
                                : nri.getAccessNetworkTechnology()));
    }

    @VisibleForTesting
    void updateDataRegState(ServiceState ss) {
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

        String text = DebugData.STR_EMPTY;
        if (ss.getState() == ServiceState.STATE_POWER_OFF) {
            text = "Radio off";
        } else if (nriState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME
                || nriState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING) {
            text = "In Service";
        } else if (wwanRegInfo != null && wwanRegInfo.isEmergencyEnabled()) {
            text = "Emergency call only";
        }

        mDebugData.put(DebugKey.DATA_REG_STATE, text);
    }

    @VisibleForTesting
    void updateCellularDataRat() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);

        mDebugData.put(DebugKey.CELLULAR_DATA_RAT,
                getNetworkTypeToString((telephony != null)
                        ? telephony.getNetworkType() : TelephonyManager.NETWORK_TYPE_UNKNOWN));
    }

    @SuppressLint("MissingPermission")
    @VisibleForTesting
    void updateNetworkOperator(ServiceState ss) {
        mDebugData.put(DebugKey.NETWORK_OPERATOR, ss.getOperatorAlphaLong());
    }

    @SuppressLint("MissingPermission")
    @VisibleForTesting
    void updateOperatorNumeric(ServiceState ss) {
        mDebugData.put(DebugKey.NETWORK_OPERATOR_NUMERIC, ss.getOperatorNumeric());
    }

    @VisibleForTesting
    void updateRoamingState(ServiceState ss) {
        mDebugData.put(DebugKey.ROAMING_STATE, (ss.getRoaming() ? "Roaming" : "Not Roaming"));
    }

    @VisibleForTesting
    void updateVoiceRoamingType(ServiceState ss) {
        NetworkRegistrationInfo regState =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_CS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        mDebugData.put(DebugKey.VOICE_ROAMING_TYPE, (regState == null)
                ? getRoamingTypeToString(ServiceState.ROAMING_TYPE_NOT_ROAMING)
                : getRoamingTypeToString(regState.getRoamingType()));
    }

    @VisibleForTesting
    void updateDataRoamingType(ServiceState ss) {
        NetworkRegistrationInfo regState =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        mDebugData.put(DebugKey.DATA_ROAMING_TYPE, (regState == null)
                ? getRoamingTypeToString(ServiceState.ROAMING_TYPE_NOT_ROAMING)
                : getRoamingTypeToString(regState.getRoamingType()));
    }

    @VisibleForTesting
    void updateNetworkFeature(ServiceState ss) {
        NetworkRegistrationInfo regInfo =
                ss.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        if (regInfo == null) {
            mDebugData.put(DebugKey.NETWORK_SUPPORT_VOPS, DebugData.STR_EMPTY);
            mDebugData.put(DebugKey.NETWORK_SUPPORT_EMCBS, DebugData.STR_EMPTY);
            mDebugData.put(DebugKey.LTE_ATTACH_TYPE, DebugData.STR_EMPTY);
            return;
        }

        DataSpecificRegistrationInfo dsrInfo = regInfo.getDataSpecificInfo();
        if (dsrInfo != null) {
            VopsSupportInfo vsi = dsrInfo.getVopsSupportInfo();
            if (vsi != null) {
                mDebugData.put(DebugKey.NETWORK_SUPPORT_VOPS,
                        (vsi.isVopsSupported()) ? "Support" : "Not Support");
                mDebugData.put(DebugKey.NETWORK_SUPPORT_EMCBS,
                        (vsi.isEmergencyServiceSupported()) ? "Support" : "Not Support");
            }

            mDebugData.put(DebugKey.LTE_ATTACH_TYPE, (dsrInfo.getLteAttachResultType()
                    == DataSpecificRegistrationInfo.LTE_ATTACH_TYPE_COMBINED)
                    ? "Combined" : "EPS Only");
        }
    }

    @VisibleForTesting
    void updatePreciseDataConnectionState(PreciseDataConnectionState state) {
        mDebugData.put(DebugKey.DATA_CONNECTION_STATE, getDataStateToString(state.getState()));
        mDebugData.put(DebugKey.NETWORK_TYPE, getNetworkTypeToString(state.getNetworkType()));

        ApnSetting as = state.getApnSetting();
        mDebugData.put(DebugKey.APN_NAME,
                (as != null) ? as.getApnName() : DebugData.STR_EMPTY);
        mDebugData.put(DebugKey.APN_TYPES,
                (as != null) ? getApnTypesToString(as.getApnTypeBitmask()) : DebugData.STR_EMPTY);
        mDebugData.put(DebugKey.APN_ENTRY_NAME,
                (as != null) ? as.getEntryName() : DebugData.STR_EMPTY);

        LinkProperties lp = state.getLinkProperties();
        mDebugData.put(DebugKey.IP_ADDRESSES,
                (lp != null) ? lp.getAddresses().toString() : DebugData.STR_EMPTY);
        mDebugData.put(DebugKey.INTERFACE_NAME,
                (lp != null) ? lp.getInterfaceName() : DebugData.STR_EMPTY);
        mDebugData.put(DebugKey.MTU,
                (lp != null) ? String.valueOf(lp.getMtu()) : DebugData.STR_EMPTY);
        mDebugData.put(DebugKey.PCSCF_ADDRESSES,
                (lp != null) ? lp.getPcscfServers().toString() : DebugData.STR_EMPTY);
    }

    private String getApnTypesToString(int apnTypeBitmask) {
        StringBuilder sb = new StringBuilder("[");

        final String[] apnStrings = {
                "DEFAULT", "MMS", "SUPL", "IMS", "EMERGENCY", "XCAP", "RCS"};
        final int[] masks = { ApnSetting.TYPE_DEFAULT, ApnSetting.TYPE_MMS,
                ApnSetting.TYPE_SUPL, ApnSetting.TYPE_IMS, ApnSetting.TYPE_EMERGENCY,
                ApnSetting.TYPE_XCAP, ApnSetting.TYPE_RCS };

        for (int i = 0; i < apnStrings.length; i++) {
            if ((apnTypeBitmask & masks[i]) != 0) {
                sb.append(" ").append(apnStrings[i]).append(" ");
            }
        }
        sb.append("]");

        return sb.toString();
    }

    @VisibleForTesting
    static String getNetworkTypeToString(int type) {
        return switch (type) {
            case TelephonyManager.NETWORK_TYPE_GPRS -> "GPRS";
            case TelephonyManager.NETWORK_TYPE_EDGE -> "EDGE";
            case TelephonyManager.NETWORK_TYPE_UMTS -> "UMTS";
            case TelephonyManager.NETWORK_TYPE_HSDPA -> "HSDPA";
            case TelephonyManager.NETWORK_TYPE_HSUPA -> "HSUPA";
            case TelephonyManager.NETWORK_TYPE_HSPA -> "HSPA";
            case TelephonyManager.NETWORK_TYPE_CDMA -> "CDMA";
            case TelephonyManager.NETWORK_TYPE_EVDO_0 -> "CDMA - EvDo rev. 0";
            case TelephonyManager.NETWORK_TYPE_EVDO_A -> "CDMA - EvDo rev. A";
            case TelephonyManager.NETWORK_TYPE_EVDO_B -> "CDMA - EvDo rev. B";
            case TelephonyManager.NETWORK_TYPE_1xRTT -> "CDMA - 1xRTT";
            case TelephonyManager.NETWORK_TYPE_LTE -> "LTE";
            case TelephonyManager.NETWORK_TYPE_EHRPD -> "CDMA - eHRPD";
            case TelephonyManager.NETWORK_TYPE_HSPAP -> "HSPA+";
            case TelephonyManager.NETWORK_TYPE_GSM -> "GSM";
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA -> "TD_SCDMA";
            case TelephonyManager.NETWORK_TYPE_IWLAN -> "IWLAN";
            case TelephonyManager.NETWORK_TYPE_LTE_CA -> "LTE_CA";
            case TelephonyManager.NETWORK_TYPE_NR -> "NR";
            default -> DebugData.STR_EMPTY;
        };
    }

    @VisibleForTesting
    static String getDataStateToString(int state) {
        return switch (state) {
            case TelephonyManager.DATA_DISCONNECTED -> "DISCONNECTED";
            case TelephonyManager.DATA_CONNECTING -> "CONNECTING";
            case TelephonyManager.DATA_CONNECTED -> "CONNECTED";
            case TelephonyManager.DATA_SUSPENDED -> "SUSPENDED";
            case TelephonyManager.DATA_DISCONNECTING -> "DISCONNECTING";
            case TelephonyManager.DATA_HANDOVER_IN_PROGRESS -> "HANDOVER IN PROGRESS";
            default -> DebugData.STR_EMPTY;
        };
    }

    @VisibleForTesting
    static String getRoamingTypeToString(int type) {
        return switch (type) {
            case ServiceState.ROAMING_TYPE_NOT_ROAMING -> "Not Roaming";
            case ServiceState.ROAMING_TYPE_DOMESTIC -> "Domestic";
            case ServiceState.ROAMING_TYPE_INTERNATIONAL -> "International";
            default -> DebugData.STR_EMPTY;
        };
    }

    @VisibleForTesting
    int getAccessNetworkType(CellSignalStrength cs) {
        if (cs instanceof CellSignalStrengthNr) {
            return AccessNetworkType.NGRAN;
        } else if (cs instanceof CellSignalStrengthLte) {
            return AccessNetworkType.EUTRAN;
        } else if (cs instanceof CellSignalStrengthWcdma) {
            return AccessNetworkType.UTRAN;
        } else if (cs instanceof CellSignalStrengthCdma) {
            return AccessNetworkType.CDMA2000;
        } else if (cs instanceof CellSignalStrengthGsm) {
            return AccessNetworkType.GERAN;
        } else {
            return AccessNetworkType.UNKNOWN;
        }
    }

    private static void logi(int slotId, String s) {
        ImsLog.i(slotId, "[Debug] " + s);
    }

    private static void logd(int slotId, String s) {
        ImsLog.d(slotId, "[Debug] " + s);
    }

    @VisibleForTesting
    NotificationManager getNotificationManager() {
        return mContext.getSystemService(NotificationManager.class);
    }

    @VisibleForTesting
    SimCarrierId getCarrierId() {
        return CarrierInfo.getInstance().getCarrierId(mSlotId);
    }

    @VisibleForTesting
    WifiInfo getWifiInfo(NetworkCapabilities capabilities) {
        return (WifiInfo) capabilities.getTransportInfo();
    }

    @VisibleForTesting
    DebugData createDebugData() {
        return new DebugData();
    }

    @VisibleForTesting
    DebugHandler createDebugHandler() {
        HandlerThread thread = new HandlerThread(AosDebug.class.getName());
        thread.start();
        return new DebugHandler(thread.getLooper());
    }
    @VisibleForTesting
    NativeStateListener createNativeStateListener() {
        return new NativeStateListener();
    }
    @VisibleForTesting
    RegistrationListener createRegistrationListener() {
        return new RegistrationListener();
    }
    @VisibleForTesting
    DebugBroadcastReceiver createDebugBroadcastReceiver() {
        return new DebugBroadcastReceiver();
    }
    @VisibleForTesting
    SignalStrengthsListener createSignalStrengthsListener() {
        return new SignalStrengthsListener();
    }
    @VisibleForTesting
    ConnectivityCallback createConnectivityCallback() {
        return new ConnectivityCallback();
    }

    @VisibleForTesting
    DebugImsPhoneStateListener createDebugImsPhoneStateListener() {
        return new DebugImsPhoneStateListener();
    }

    @VisibleForTesting
    int getSlotId(int subId) {
        return MSimUtils.getSlotId(subId);
    }

    @VisibleForTesting
    class NativeStateListener implements NativeStateInterface.Listener {
        @Override
        public void onNativeServiceReady() {
            logi(mSlotId, "NativeState: service ready.");
            selfCheckDebugNotification();
        }

        public void register() {
            NativeStateInterface nsi = getNativeStateInterface();
            if (nsi != null) {
                nsi.addListener(this);
            }
        }

        public void unregister() {
            NativeStateInterface nsi = getNativeStateInterface();
            if (nsi != null) {
                nsi.removeListener(this);
            }
        }

        private NativeStateInterface getNativeStateInterface() {
            return AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        }
    }

    @VisibleForTesting
    class DebugImsPhoneStateListener implements ImsPhoneStateListener {
        private IPhoneStateNotifier mNotifier;
        private PhoneStateInterface mPhoneState;

        DebugImsPhoneStateListener() {}

        public void removeListener() {
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
                mNotifier = mPhoneState.createNotifier(this, mHandler.getLooper());
                mNotifier.setEvents(LISTEN_SERVICE_STATE | LISTEN_PRECISE_DATA_CONNECTION_STATE);

                mPhoneState.addNotifier(mNotifier);
            }
        }

        @Override
        public void onServiceStateChanged(ServiceState ss) {
            Message.obtain(mHandler, DEBUG_SERVICE_STATE_CHANGED, ss).sendToTarget();
        }

        @Override
        public void onPreciseDataConnectionStateChanged(
                PreciseDataConnectionState dataConnectionState) {
            Message.obtain(mHandler, DEBUG_PRECISE_DATA_CONNECTION_CHANGED,
                    dataConnectionState).sendToTarget();
        }
    }

    @VisibleForTesting
    class ConnectivityCallback
            extends ConnectivityManager.NetworkCallback {

        private LinkProperties mLinkProperties;
        private NetworkCapabilities mNetworkCapabilities;
        private boolean mIsConnected = false;

        @Override
        public void onAvailable(Network network) {
            mIsConnected = true;
            Message.obtain(mHandler, DEBUG_WIFI_CONNECTIVITY_CHANGED, network).sendToTarget();
        }

        @Override
        public void onLost(Network network) {
            Message.obtain(mHandler, DEBUG_WIFI_CONNECTIVITY_CHANGED, null).sendToTarget();
            ImsLog.i("Wifi#onLost: network=" + network);
            removeConnectivityInfo();
        }

        @Override
        public void onLinkPropertiesChanged(@NonNull Network network,
                @NonNull LinkProperties linkProperties) {
            mLinkProperties = linkProperties;
            Message.obtain(mHandler, DEBUG_WIFI_CONNECTIVITY_CHANGED, network).sendToTarget();
        }

        @Override
        public void onCapabilitiesChanged(@NonNull Network network,
                @NonNull NetworkCapabilities networkCapabilities) {
            mNetworkCapabilities = networkCapabilities;
            Message.obtain(mHandler, DEBUG_WIFI_CONNECTIVITY_CHANGED, network).sendToTarget();
        }

        public LinkProperties getLinkProperties() {
            return mLinkProperties;
        }

        public NetworkCapabilities getNetworkCapabilities() {
            return mNetworkCapabilities;
        }

        public boolean isConnected() {
            return mIsConnected;
        }

        private void removeConnectivityInfo() {
            mIsConnected = false;
            mLinkProperties = null;
            mNetworkCapabilities = null;
        }

        public void register() {
            ConnectivityManagerProxy cmp =
                    AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
            NetworkRequest nr = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                    .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .build();

            cmp.registerNetworkCallback(nr, this, mHandler);
        }

        public void unregister() {
            ConnectivityManagerProxy cmp =
                    AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
            cmp.unregisterNetworkCallback(this);
        }
    }

    @VisibleForTesting
    class SignalStrengthsListener extends TelephonyCallback implements
            TelephonyCallback.SignalStrengthsListener {

        SignalStrengthsListener() {}

        @Override
        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            Message.obtain(mHandler,
                    DEBUG_SIGNALSTRENGTHS_CHANGED, signalStrength).sendToTarget();
        }

        public void register() {
            if (mSubId == MSimUtils.INVALID_SUB_ID) {
                return;
            }

            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.registerTelephonyCallback(mHandler::post, this);
        }

        public void unregister() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.unregisterTelephonyCallback(this);
        }
    }

    @VisibleForTesting
    class DebugBroadcastReceiver extends BroadcastReceiver {

        public DebugBroadcastReceiver() {}

        public void register() {
            IntentFilter filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
            AppContext.getInstance().getBroadcastReceiverProxy()
                    .registerReceiver(this, filter, mHandler);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logi(mSlotId, ImsLog.lastSubString(action, "."));

            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                Message.obtain(mHandler, DEBUG_AIRPLANE_MODE_CHANGED, intent).sendToTarget();
            }
        }
    }

    @VisibleForTesting
    class RegistrationListener implements IAosRegistrationListener {

        @Override
        public void notifyRegistered(int regType, NetworkType networkType, int featureTagBits,
                java.util.Set<String> featureTags) {
            logi(mSlotId, "notifyRegistered - regType:" + regType
                    + ", networkType:" + networkType
                    + ", featureTagBits:" + featureTagBits + ", featureTags:" + featureTags);
            Message.obtain(mHandler,
                    DEBUG_NOTIFY_REGISTERED, networkType.getValue(), featureTagBits).sendToTarget();
        }

        @Override
        public void notifyDeregistered(
                int regType, NetworkType networkType, ReasonCode reason, String message) {
            logi(mSlotId, "notifyDeregistered - regType:" + regType
                    + ", networkType:" + networkType
                    + ", reason:" + reason.toString() + "message:" + message);
            Message.obtain(mHandler, DEBUG_NOTIFY_DEREGISTERED,
                    networkType.getValue(), reason.getValue()).sendToTarget();
        }

        @Override
        public void notifyRegistering(int regType, NetworkType networkType, int featureTagBits,
                java.util.Set<String> featureTags) {
            // Do nothing.
        }

        @Override
        public void notifyTechnologyChangeFailed(
                int regType, NetworkType networkType, ReasonCode reason, String message) {
            // Do nothing.
        }

        @Override
        public void notifyAssociatedUriChanged(android.net.Uri[] uris) {
            // Do nothing.
        }

        @Override
        public void notifyCapabilitiesUpdateFailed(
                int capabilities, NetworkType networkType, int reason) {
            // Do nothing.
        }

        @Override
        public void notifyCapabilitiesUpdated(IAosRegistration.CapabilityPairs pairs) {
            Message.obtain(mHandler,
                    DEBUG_NOTIFY_CAPABILITIES_UPDATED, pairs).sendToTarget();
        }

        @Override
        public void notifyRegEventStateChanged(int statusCode, @NonNull Set<Uri> impus) {
            // Do nothing.
        }
    }

    @VisibleForTesting
    final class DebugHandler extends Handler {
        private DebugHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg != null) {
                logi(mSlotId, "handleMessage :: msg= " + msg.what);

                switch (msg.what) {
                    case DEBUG_AIRPLANE_MODE_CHANGED -> handleAirplaneModeChanged(msg);
                    case DEBUG_SUBSCRIPTION_CHANGED -> handleSubscriptionChanged();
                    case DEBUG_SIGNALSTRENGTHS_CHANGED -> handleSignalStrengthsChanged(msg);
                    case DEBUG_WIFI_CONNECTIVITY_CHANGED -> handleWifiConnectivityChanged(msg);
                    case DEBUG_SERVICE_STATE_CHANGED -> handleServiceStateChanged(msg);
                    case DEBUG_PRECISE_DATA_CONNECTION_CHANGED ->
                            handlePreciseDataConnectionChanged(msg);
                    case DEBUG_NOTIFY_REGISTERED -> handleNotifyRegistered(msg);
                    case DEBUG_NOTIFY_DEREGISTERED -> handleNotifyDeregistered(msg);
                    case DEBUG_NOTIFY_CAPABILITIES_UPDATED -> handleNotifyCapabilitiesUpdated(msg);
                }
            }
        }

        private void handleAirplaneModeChanged(Message msg) {
            Intent intent = (Intent) msg.obj;
            if (intent.getBooleanExtra("state", false)) {
                mDebugData.clear();
            }
        }

        private void handleSubscriptionChanged() {
            updateSubscription();
            updateCarrierInfo();
            selfCheckDebugNotification();
        }

        private void handleSignalStrengthsChanged(Message msg) {
            List<CellSignalStrength> cellSignalStrengths =
                    ((SignalStrength) msg.obj).getCellSignalStrengths();

            for (CellSignalStrength cs : cellSignalStrengths) {
                updateSignalStrengthData(cs, getAccessNetworkType(cs));
            }
        }

        private void handleWifiConnectivityChanged(Message msg) {
            if (msg == null || !mConnectivityCallback.isConnected()) {
                clearWifiConnectivityData();
                return;
            }

            mDebugData.put(DebugKey.WIFI_CONNECTION_STATE, DebugData.STR_CONNECTED);

            LinkProperties lp = mConnectivityCallback.getLinkProperties();
            String addresses = (lp != null) ? lp.getAddresses().toString() : DebugData.STR_EMPTY;
            String interfaceName = (lp != null) ? lp.getInterfaceName() : DebugData.STR_EMPTY;
            mDebugData.put(DebugKey.WIFI_ADDRESSES, addresses);
            mDebugData.put(DebugKey.WIFI_INTERFACE_NAME, interfaceName);

            NetworkCapabilities capabilities = mConnectivityCallback.getNetworkCapabilities();
            if (capabilities != null) {
                WifiInfo wifiInfo = getWifiInfo(capabilities);
                int rssi = (wifiInfo != null) ? wifiInfo.getRssi() : INVALID_RSSI;
                String bssId = (wifiInfo != null) ? wifiInfo.getBSSID() : "";
                String ssId = (wifiInfo != null) ? wifiInfo.getSSID() : "";
                @SuppressLint("HardwareIds")
                String macAddress = (wifiInfo != null) ? wifiInfo.getMacAddress() : "";

                mDebugData.put(DebugKey.WIFI_RSSI, rssi + " dBm");
                mDebugData.put(DebugKey.WIFI_BSSID, (bssId.length() != 0)
                        ? bssId : DebugData.STR_EMPTY);
                mDebugData.put(DebugKey.WIFI_SSID, (ssId.length() != 0)
                        ? ssId : DebugData.STR_EMPTY);
                mDebugData.put(DebugKey.WIFI_MAC_ADDRESS,
                        (macAddress.length() != 0) ? macAddress : DebugData.STR_EMPTY);
            }
        }

        private void clearWifiConnectivityData() {
            mDebugData.put(DebugKey.WIFI_CONNECTION_STATE, DebugData.STR_DISCONNECTED);
            mDebugData.put(DebugKey.WIFI_ADDRESSES, DebugData.STR_EMPTY);
            mDebugData.put(DebugKey.WIFI_INTERFACE_NAME, DebugData.STR_EMPTY);
            mDebugData.put(DebugKey.WIFI_RSSI, INVALID_RSSI + " dBm");
            mDebugData.put(DebugKey.WIFI_BSSID, DebugData.STR_EMPTY);
            mDebugData.put(DebugKey.WIFI_SSID, DebugData.STR_EMPTY);
            mDebugData.put(DebugKey.WIFI_MAC_ADDRESS, DebugData.STR_EMPTY);
        }

        private void handleServiceStateChanged(Message msg) {
            ServiceState ss = (ServiceState) msg.obj;

            updateServiceState(ss.getState());
            updateVoiceRat(ss);
            updateDataRegState(ss);
            updateCellularDataRat();
            updateNetworkOperator(ss);
            updateOperatorNumeric(ss);
            updateRoamingState(ss);
            updateVoiceRoamingType(ss);
            updateDataRoamingType(ss);
            updateNetworkFeature(ss);
        }

        private void handlePreciseDataConnectionChanged(Message msg) {
            PreciseDataConnectionState pdc = (PreciseDataConnectionState) msg.obj;
            updatePreciseDataConnectionState(pdc);
        }

        private void handleNotifyRegistered(Message msg) {
            updateRegisteredData(NetworkType.of(msg.arg1), msg.arg2);
        }

        private void handleNotifyDeregistered(Message msg) {
            updateDeregisterData(NetworkType.of(msg.arg1), msg.arg2);
        }

        private void handleNotifyCapabilitiesUpdated(Message msg) {
            mDebugData.put(DebugKey.CAPABILITIES,
                    getCapabilitiesListToString((IAosRegistration.CapabilityPairs) msg.obj));
        }
    }
}
