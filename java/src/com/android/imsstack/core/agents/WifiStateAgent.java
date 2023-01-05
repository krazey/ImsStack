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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.imsstack.system.ISystemAPIWifi;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

import java.util.Locale;
import java.util.Map;

/**
 * A class for tracking the state of Wi-Fi connection.
 */
@SuppressWarnings("deprecation")
public class WifiStateAgent implements IWifiState, ISystemAPIWifi {
    // Internal Event
    private static final int EVENT_NETWORK_STATE_CHAGED_ACTION = 1001;
    private static final int EVENT_WIFI_STATE_CHANGED_ACTION = 1002;
    private static final int EVENT_RSSI_CHANGED_ACTION = 1003;
    private static final int EVENT_TURN_ON_OFF = 1004;

    // NOTICE: This needs to be synchronize with native constant values.
    private static final Map<NetworkInfo.DetailedState, Integer> DETAILED_STATE_MAP = Map.ofEntries(
            Map.entry(NetworkInfo.DetailedState.IDLE, 0),
            Map.entry(NetworkInfo.DetailedState.SCANNING, 1),
            Map.entry(NetworkInfo.DetailedState.CONNECTING, 2),
            Map.entry(NetworkInfo.DetailedState.AUTHENTICATING, 3),
            Map.entry(NetworkInfo.DetailedState.OBTAINING_IPADDR, 4),
            Map.entry(NetworkInfo.DetailedState.CONNECTED, 5),
            Map.entry(NetworkInfo.DetailedState.SUSPENDED, 6),
            Map.entry(NetworkInfo.DetailedState.DISCONNECTING, 7),
            Map.entry(NetworkInfo.DetailedState.DISCONNECTED, 8),
            Map.entry(NetworkInfo.DetailedState.FAILED, 9),
            Map.entry(NetworkInfo.DetailedState.BLOCKED, 10),
            Map.entry(NetworkInfo.DetailedState.VERIFYING_POOR_LINK, 11),
            Map.entry(NetworkInfo.DetailedState.CAPTIVE_PORTAL_CHECK, 12));

    private static IWifiState sWifiStateAgent;

    private WifiStateReceiver mWifiStateReceiver;
    private WifiStateHandler mWifiStateHandler;

    private RegistrantList mWifiStateRegistrants = new RegistrantList();

    private int mWifiState = WifiManager.WIFI_STATE_UNKNOWN;
    private int mWifiDetailedStatus = 9; // WHY??

    private boolean mWifiSupported = false;
    private boolean mIsWifiConnected = false;

    public WifiStateAgent() {
    }

    public static IWifiState getInstance() {
        if (sWifiStateAgent == null) {
            sWifiStateAgent = new WifiStateAgent();
        }

        return sWifiStateAgent;
    }

    @Override
    public void init(Context context) {
        SystemInterface.getInstance().setISystemAPIWifi(this);

        if (isWifiConnectionRequired()) {
            mWifiStateReceiver = new WifiStateReceiver();
            AppContext.getInstance().registerReceiver(mWifiStateReceiver,
                    mWifiStateReceiver.getFilter(), Context.RECEIVER_EXPORTED);

            mWifiStateHandler = new WifiStateHandler(Looper.myLooper());
        }
    }

    @Override
    public void cleanup() {
        ImsLog.d("");

        if (isWifiConnectionRequired()) {
            if (mWifiStateHandler != null) {
                mWifiStateHandler.removeCallbacksAndMessages(null);
                mWifiStateHandler = null;
            }

            if (mWifiStateReceiver != null) {
                AppContext.getInstance().unregisterReceiver(mWifiStateReceiver);
                mWifiStateReceiver = null;
            }
        }

        SystemInterface.getInstance().setISystemAPIWifi(null);
    }

    @Override
    public void registerForWifiStateChanged(Handler h, int what, Object obj) {
        mWifiStateRegistrants.add(new Registrant(h, what, obj));
    }

    @Override
    public void unregisterForWifiStateChanged(Handler h) {
        mWifiStateRegistrants.remove(h);
    }

    @Override
    public boolean isWifiConnected() {
        ImsLog.i("wifi is" + (mIsWifiConnected ? " " : " not ") + "connected");
        return mIsWifiConnected;
    }

    @Override
    public int getWifiDetailedStatus() {
        ImsLog.i("mWifiDetailedStatus : " + mWifiDetailedStatus);
        return mWifiDetailedStatus;
    }

    @Override
    public String getWifiSSID() {
        WifiInfo wifiInfo = getWifiInfo();

        if (wifiInfo == null) {
            ImsLog.w("WifiInfo is null");
            return null;
        }

        String ssid = wifiInfo.getSSID();

        if ((ssid != null) && ssid.startsWith("\"") && ssid.endsWith("\"")) {
            ssid = ssid.substring(1, ssid.length() - 1);
        }

        return ssid;
    }

    @Override
    public void setWifiSupported(boolean input) {
        ImsLog.i("wifi is supported = " + input);
        mWifiSupported = input;
    }

    @Override
    public String getLocalAddress() {
        WifiInfo wifiInfo = getWifiInfo();

        if (wifiInfo == null) {
            ImsLog.w("WifiInfo is null");
            return "";
        }

        int ipAddress = wifiInfo.getIpAddress();

        String strWifiIp = String.format(Locale.US, "%d.%d.%d.%d",
                (ipAddress & 0xff),
                (ipAddress >> 8 & 0xff),
                (ipAddress >> 16 & 0xff),
                (ipAddress >> 24 & 0xff));

        ImsLog.i("Wifi IP : " + strWifiIp);
        return strWifiIp;
    }

    @Override
    public int getWifiState4Sys() {
        return getWifiState();
    }

    @Override
    public int getWifiDetailedState4Sys() {
        return getWifiDetailedStatus();
    }

    @Override
    public String getWifiBssId4Sys() {
        return getWifiBSSID();
    }

    @Override
    public String getWifiSsId4Sys() {
        return getWifiSSID();
    }

    @Override
    public int getWifiIfaceId4Sys() {
        Network network = getCurrentNetwork();

        if (network == null) {
            ImsLog.w("Network is null");
            return -1;
        }

        return network.getNetId();
    }

    @Override
    public int getWifiMtu4Sys() {
        Network network = getCurrentNetwork();
        if (network == null) {
            return 0;
        }

        ConnectivityManager cm =
                AppContext.getInstance().getSystemService(ConnectivityManager.class);
        LinkProperties lp = (cm != null) ? cm.getLinkProperties(network) : null;
        return (lp != null) ? lp.getMtu() : 0;
    }

    @Override
    public void turnOnOff(boolean on) {
        if (mWifiStateHandler == null) {
            handleWifiSupported(on);
        } else {
            Message.obtain(mWifiStateHandler, EVENT_TURN_ON_OFF, on ? 1 : 0, 0).sendToTarget();
        }
    }

    private Network getCurrentNetwork() {
        WifiManager wm = getWifiManager();
        return (wm != null) ? wm.getCurrentNetwork() : null;
    }

    private WifiManager getWifiManager() {
        return AppContext.getInstance().getSystemService(WifiManager.class);
    }

    private WifiInfo getWifiInfo() {
        WifiManager wm = getWifiManager();
        return (wm != null) ? wm.getConnectionInfo() : null;
    }

    private int getWifiState() {
        ImsLog.i("wifi state = " + mWifiState);
        return mWifiState;
    }

    private String getWifiBSSID() {
        WifiInfo wifiInfo = getWifiInfo();

        if (wifiInfo == null) {
            ImsLog.w("WifiInfo is null");
            return null;
        }

        String bssID = wifiInfo.getBSSID();

        ImsLog.i("BSS ID is " + bssID);

        return bssID;
    }

    private void handleNetworkStateChanged(Message msg) {
        Intent intent = (Intent)msg.obj;

        if (intent == null) {
            return;
        }

        final NetworkInfo networkInfo = (NetworkInfo)intent.getParcelableExtra(
            WifiManager.EXTRA_NETWORK_INFO);

        if (networkInfo == null) {
            ImsLog.w("NetworkInfo is NULL" );
            return;
        }

        ImsLog.d("NETWORK_STATE_CHANGED_ACTION : networkinfo (" + networkInfo.toString() + ")");

        /*
            CONNECTING, CONNECTED, SUSPENDED, DISCONNECTING, DISCONNECTED, UNKNOWN
         */
        //eState = networkInfo.getState();

        /*
            IDLE, SCANNING, CONNECTING, AUTHENTICATING,
            OBTAINING_IPADDR, CONNECTED, SUSPENDED,
            DISCONNECTING, DISCONNECTED, FAILED
        */
        setWifiConnectedState(networkInfo);

        mWifiDetailedStatus = getNetworkInfoDetailedState(networkInfo.getDetailedState());

        // if wifi service is supported
        if (mWifiSupported == true) {
            SystemInterface.getInstance().notifyWifiDetailedStateChanged(mWifiDetailedStatus);
        }
    }

    private int getNetworkInfoDetailedState(NetworkInfo.DetailedState state) {
        Integer niDetailedState = DETAILED_STATE_MAP.get(state);
        return (niDetailedState != null) ? niDetailedState.intValue() : (-1);
    }

    private void handleWifiStateChanged(Message msg) {
        Intent intent = (Intent)msg.obj;

        if (intent == null) {
            return;
        }

        /*
            NetworkInfo networkInfo = (NetworkInfo)intent.getParcelableExtra(
                WifiManager.EXTRA_NETWORK_INFO);
        */
        mWifiState = intent.getIntExtra(
            WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN);

        ImsLog.i("WIFI_STATE_CHANGED_ACTION : nStatus (" + mWifiState + ")");

        // if wifi service is supported
        if (mWifiSupported == true) {
            SystemInterface.getInstance().notifyWifiStateChanged(mWifiState);
        }
    }

    private void handleRSSIChanged(Message msg) {
        Intent intent = (Intent)msg.obj;

        if (intent == null) {
            return;
        }

        int newRssi = intent.getIntExtra(WifiManager.EXTRA_NEW_RSSI, -200);
        newRssi = newRssi * (-1);

        ImsLog.i("RSSI_CHANGED_ACTION : [" + newRssi + "]");
    }

    private void handleWifiSupported(boolean on) {
        if (on) {
            SystemInterface.getInstance().notifyWifiStateChanged(mWifiState);
            SystemInterface.getInstance().notifyWifiDetailedStateChanged(mWifiDetailedStatus);

            setWifiSupported(true);
        } else {
            setWifiSupported(false);
        }
    }

    private boolean isWifiConnectionRequired() {
        return true;
    }

    private void setWifiConnectedState(NetworkInfo netInfo) {
        boolean oldWifiConnected = mIsWifiConnected;

        if (netInfo == null) {
            return;
        }

        NetworkInfo.DetailedState detailedState = netInfo.getDetailedState();
        if (detailedState == null) {
            ImsLog.w("DetailedState is null");
            return;
        }

        if (detailedState == NetworkInfo.DetailedState.CONNECTED
                || detailedState == NetworkInfo.DetailedState.CAPTIVE_PORTAL_CHECK) {
            mIsWifiConnected = true;
        } else {
            if ((mIsWifiConnected)
                    && (detailedState == NetworkInfo.DetailedState.OBTAINING_IPADDR
                            || detailedState == NetworkInfo.DetailedState.VERIFYING_POOR_LINK)) {
                mIsWifiConnected = true;
            } else {
                mIsWifiConnected = false;
            }
        }

        if (oldWifiConnected != mIsWifiConnected) {
            mWifiStateRegistrants.notifyResult(Integer.valueOf((mIsWifiConnected) ? 1 : 0));
        }

        ImsLog.i("DetailedState = " + detailedState + " , mIsWifiConnected = " + mIsWifiConnected);
    }

    private class WifiStateReceiver extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        WifiStateReceiver() {
            mIntentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
            mIntentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
            mIntentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            if (mWifiStateHandler == null || intent == null) {
                return;
            }

            String action = intent.getAction();
            int nMsg = 0;
            if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
                nMsg = EVENT_NETWORK_STATE_CHAGED_ACTION;
            } else if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                nMsg = EVENT_WIFI_STATE_CHANGED_ACTION;
            } else if (WifiManager.RSSI_CHANGED_ACTION.equals(action)) {
                nMsg = EVENT_RSSI_CHANGED_ACTION;
            } else if (ConnectivityManager.CONNECTIVITY_ACTION.equals(action)) {
                NetworkInfo netInfo = (NetworkInfo)intent.getParcelableExtra(
                    ConnectivityManager.EXTRA_NETWORK_INFO);

                if (netInfo != null) {
                    /* ImsStack-Build_ConnectivityManager#TYPE_
                    if (netInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                        ImsLog.d(netInfo.toString());
                        nMsg = EVENT_NETWORK_STATE_CHAGED_ACTION;
                    }*/
                }
            }

            if (nMsg != 0) {
                ImsLog.i(ImsLog.lastSubString(action, "."));

                Message msg = Message.obtain();
                if (msg == null) {
                    return;
                }

                msg.what = nMsg;
                msg.obj = (Object)intent;

                mWifiStateHandler.sendMessage(msg);
            }
        }
    }
    // -----------------------------------------------------------
    private class WifiStateHandler extends Handler {
        WifiStateHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i("WifiState :: msg=" + msg.what);

            switch(msg.what) {
                // Receiver Event
                case EVENT_NETWORK_STATE_CHAGED_ACTION:
                    handleNetworkStateChanged(msg);
                    break;
                case EVENT_WIFI_STATE_CHANGED_ACTION:
                    handleWifiStateChanged(msg);
                    break;
                case EVENT_RSSI_CHANGED_ACTION:
                    handleRSSIChanged(msg);
                    break;
                case EVENT_TURN_ON_OFF:
                    handleWifiSupported((msg.arg1 == 1) ? true : false);
                    break;
                default:
                    break;
            }
        }
    }
}
