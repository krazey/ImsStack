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
package com.android.imsstack.core.agents;

import android.annotation.NonNull;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;

import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A class for tracking the Wi-Fi settings and connection state.
 */
public class WifiAgent implements WifiInterface {
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();
    private Handler mHandler;
    private WifiBroadcastReceiver mBroadcastReceiver;
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private Network mNetwork;
    private LinkProperties mLinkProperties;
    private WifiInfo mWifiInfo;
    private boolean mIsWifiServiceRequested;
    private boolean mIsWifiEnabled;
    private boolean mIsWifiConnected;

    public WifiAgent() {
    }

    @Override
    public void init(Context context) {
        mHandler = new Handler(AppContext.getInstance().getMainLooper());

        mBroadcastReceiver = new WifiBroadcastReceiver();
        mBroadcastReceiver.register();

        registerNetworkCallback();
    }

    @Override
    public void cleanup() {
        unregisterNetworkCallback();

        if (mBroadcastReceiver != null) {
            mBroadcastReceiver.unregister();
            mBroadcastReceiver = null;
        }

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
    }

    @Override
    public boolean isWifiEnabled() {
        return mIsWifiEnabled;
    }

    @Override
    public boolean isWifiConnected() {
        return mIsWifiConnected;
    }

    @Override
    public Network getNetwork() {
        return mNetwork;
    }

    @Override
    public int getIfaceId() {
        if (mNetwork == null) {
            ImsLog.w("Wifi: Network is not available.");
            return -1;
        }

        return mNetwork.getNetId();
    }

    @Override
    public String getIfaceName() {
        return (mLinkProperties != null) ? mLinkProperties.getInterfaceName() : null;
    }

    @Override
    public int getMtu() {
        return (mLinkProperties != null) ? mLinkProperties.getMtu() : 0;
    }

    @Override
    public String getLocalAddress(int ipVersion) {
        List<InetAddress> inetAddresses =
                (mLinkProperties != null) ? mLinkProperties.getAddresses() : null;
        String ipv4Address = null;
        String ipv6Address = null;

        if (inetAddresses != null) {
            for (InetAddress inetAddr : inetAddresses) {
                if (inetAddr == null
                        || inetAddr.isAnyLocalAddress()
                        || inetAddr.isLinkLocalAddress()
                        || inetAddr.isLoopbackAddress()) {
                    continue;
                }

                // Cache first address for each IP version.
                if (inetAddr instanceof Inet6Address) {
                    if (ipv6Address == null) {
                        ipv6Address = inetAddr.getHostAddress();
                    }
                } else if (inetAddr instanceof Inet4Address) {
                    if (ipv4Address == null) {
                        ipv4Address = inetAddr.getHostAddress();
                    }
                }
            }
        }

        ImsLog.d("Wifi: ipVersion=" + ipVersion
                + ", ipv4=" + ipv4Address + ", ipv6=" + ipv6Address);

        if (ipVersion == EIpVersion.IPV6.getInt()) {
            return ipv6Address;
        } else if (ipVersion == EIpVersion.IPV4.getInt()) {
            return ipv4Address;
        } else if (ipVersion == EIpVersion.IPV6V4.getInt()) {
            return (ipv6Address != null) ? ipv6Address : ipv4Address;
        } else if (ipVersion == EIpVersion.IPV4V6.getInt()) {
            return (ipv4Address != null) ? ipv4Address : ipv6Address;
        }

        return null;
    }

    @Override
    public String[] getHostByName(int ipVersion, String host) {
        if (mNetwork == null) {
            ImsLog.w("Wifi: Network is not available.");
            return null;
        }

        InetAddress[] inetAddrs = null;

        try {
            inetAddrs = mNetwork.getAllByName(host);
        } catch (UnknownHostException e) {
            ImsLog.e("Wifi: UnknownHostException:" + e.toString());
        }

        if (inetAddrs == null || inetAddrs.length == 0) {
            ImsLog.w("Wifi: InetAddress[] is null or zero-length.");
            return null;
        }

        List<String> ipAddrs = new ArrayList<String>();

        if (ipVersion == EIpVersion.IPV4.getInt()) {
            for (int i = 0; i < inetAddrs.length; ++i) {
                if (inetAddrs[i] instanceof Inet4Address) {
                    ipAddrs.add(inetAddrs[i].getHostAddress());
                }
            }
        } else {
            for (int i = 0; i < inetAddrs.length; ++i) {
                if (inetAddrs[i] instanceof Inet6Address) {
                    ipAddrs.add(inetAddrs[i].getHostAddress());
                }
            }
        }

        ImsLog.i("Wifi: getHostByName - ipVersion=" + ipVersion + ", size=" + ipAddrs.size());

        if (ipAddrs.isEmpty()) {
            return null;
        }

        return ipAddrs.toArray(new String[ipAddrs.size()]);
    }

    @Override
    public String getBssId() {
        return (mWifiInfo != null) ? mWifiInfo.getBSSID() : null;
    }

    @Override
    public String getSsId() {
        String ssid = (mWifiInfo != null) ? mWifiInfo.getSSID() : null;

        if ((ssid != null) && ssid.startsWith("\"") && ssid.endsWith("\"")) {
            ssid = ssid.substring(1, ssid.length() - 1);
        }

        return ssid;
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
    public int bindSocket(FileDescriptor sockFd) {
        ImsLog.d("Wifi#bindSocket: network=" + mNetwork);

        if (mNetwork != null && sockFd != null) {
            try {
                mNetwork.bindSocket(sockFd);
                return 1;
            } catch (IOException e) {
                ImsLog.e("Wifi#bindSocket: " + e.toString());
            }
        }

        return 0;
    }

    @Override
    public void requestWifiService(boolean serviceRequested) {
        mHandler.post(() -> {
            setWifiServiceRequested(serviceRequested);

            if (isWifiServiceRequested()) {
                SystemInterface.getInstance().notifyWifiStateChanged(
                        isWifiEnabled()
                        ? WifiInterface.STATE_ENABLED
                        : WifiInterface.STATE_DISABLED);
                SystemInterface.getInstance().notifyWifiConnectionStateChanged(
                        isWifiConnected()
                        ? WifiInterface.CONNECTION_STATE_CONNECTED
                        : WifiInterface.CONNECTION_STATE_DISCONNECTED);
            }
        });
    }

    private void setWifiEnabled(boolean enabled) {
        if (mIsWifiEnabled != enabled) {
            ImsLog.d("Wifi#setWifiEnabled: " + mIsWifiEnabled + " >> " + enabled);
            mIsWifiEnabled = enabled;

            if (isWifiServiceRequested()) {
                SystemInterface.getInstance().notifyWifiStateChanged(
                        mIsWifiEnabled
                        ? WifiInterface.STATE_ENABLED
                        : WifiInterface.STATE_DISABLED);
            }

            notifyWifiStateChanged();
        }
    }

    private void setWifiConnected(boolean connected) {
        if (mIsWifiConnected != connected) {
            ImsLog.d("Wifi#setWifiConnected: " + mIsWifiConnected + " >> " + connected);
            mIsWifiConnected = connected;

            if (isWifiServiceRequested()) {
                SystemInterface.getInstance().notifyWifiConnectionStateChanged(
                        mIsWifiConnected
                        ? WifiInterface.CONNECTION_STATE_CONNECTED
                        : WifiInterface.CONNECTION_STATE_DISCONNECTED);
            }

            notifyWifiConnectionStateChanged();
        }
    }

    private boolean isWifiServiceRequested() {
        return mIsWifiServiceRequested;
    }

    private void setWifiServiceRequested(boolean serviceRequested) {
        ImsLog.d("Wifi#setWifiServiceRequested: " + serviceRequested);
        mIsWifiServiceRequested = serviceRequested;
    }

    private void notifyWifiStateChanged() {
        for (Listener listener : mListeners) {
            listener.onWifiStateChanged();
        }
    }

    private void notifyWifiConnectionStateChanged() {
        for (Listener listener : mListeners) {
            listener.onWifiConnectionStateChanged();
        }
    }

    private void registerNetworkCallback() {
        ConnectivityManager cm =
                AppContext.getInstance().getSystemService(ConnectivityManager.class);
        if (cm != null) {
            if (mNetworkCallback == null) {
                mNetworkCallback = new ConnectivityManager.NetworkCallback(
                        ConnectivityManager.NetworkCallback.FLAG_INCLUDE_LOCATION_INFO) {
                    @Override
                    public void onLost(@NonNull Network network) {
                        ImsLog.i("Wifi#onLost: network=" + network);
                        setWifiConnected(false);
                        mNetwork = null;
                        mLinkProperties = null;
                        mWifiInfo = null;
                    }

                    @Override
                    public void onLinkPropertiesChanged(@NonNull Network network,
                            @NonNull LinkProperties linkProperties) {
                        ImsLog.i("Wifi#onLinkPropertiesChanged: network=" + network);
                        mNetwork = network;
                        mLinkProperties = linkProperties;
                        setWifiConnected(true);
                    }

                    @Override
                    public void onCapabilitiesChanged(@NonNull Network network,
                            @NonNull NetworkCapabilities networkCapabilities) {
                        ImsLog.d("Wifi#onCapabilitiesChanged: network=" + network);
                        mWifiInfo = (WifiInfo) networkCapabilities.getTransportInfo();
                    }
                };
            }

            NetworkRequest networkRequest = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                    .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .build();
            cm.registerNetworkCallback(networkRequest, mNetworkCallback, mHandler);
        }
    }

    private void unregisterNetworkCallback() {
        if (mNetworkCallback != null) {
            ConnectivityManager cm =
                    AppContext.getInstance().getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.unregisterNetworkCallback(mNetworkCallback);
            }
            mNetworkCallback = null;
        }

        setWifiConnected(false);
        mNetwork = null;
        mLinkProperties = null;
        mWifiInfo = null;
    }

    private class WifiBroadcastReceiver extends BroadcastReceiver {
        public void register() {
            IntentFilter filter = new IntentFilter(WifiManager.WIFI_STATE_CHANGED_ACTION);
            AppContext.getInstance().registerReceiver(
                    this, filter, null, mHandler, Context.RECEIVER_EXPORTED);
        }

        public void unregister() {
            AppContext.getInstance().unregisterReceiver(this);
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i("Wifi: " + ImsLog.lastSubString(action, "."));

            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                int wifiState = intent.getIntExtra(
                        WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN);

                if (wifiState == WifiManager.WIFI_STATE_ENABLED) {
                    setWifiEnabled(true);
                } else {
                    setWifiEnabled(false);
                }
            }
        }
    }
}
