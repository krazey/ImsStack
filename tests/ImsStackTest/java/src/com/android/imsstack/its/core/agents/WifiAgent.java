/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.core.agents;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.its.util.SingleLatch;
import com.android.imsstack.util.Log;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

/**
 * A class for managing Wi-Fi settings and monitoring connection state.
 * This class provides functionalities to track Wi-Fi settings and connection state changes,
 * as well as to register listeners for receiving notifications about these changes.
 */
public class WifiAgent {

    interface Listener {

        /**
         * Callback method invoked when the Wi-Fi state changes.
         * Implementations of this method should handle changes in the Wi-Fi state.
         * To obtain the updated Wi-Fi state, use the method {@link #isWifiEnabled()}.
         */
        default void onWifiStateChanged() {}

        /**
         * Callback method invoked when the Wi-Fi connection state changes.
         * Implementations of this method should handle changes in the Wi-Fi connection state.
         * To obtain the updated Wi-Fi connection state, use the method {@link #isWifiConnected()}.
         */
        default void onWifiConnectionStateChanged() {}
    }

    private Context mContext;
    private Handler mHandler;
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private IntentReceiver mIntentReceiver;
    private Network mNetwork;
    private LinkProperties mLinkProperties;
    private boolean mIsWifiEnabled;
    private boolean mIsWifiConnected;
    private final Set<Listener> mListeners = new HashSet<>();
    private final SingleLatch mWifiConnectedLatch = new SingleLatch("Wi-Fi connected");

    /**
     * Returns the singleton instance of {@link WifiAgent}.
     * <p>
     * This method returns the singleton instance of {@link WifiAgent} class.
     * This instance can be used to access Wi-Fi related functionalities
     * and monitor Wi-Fi state changes by adding listeners.
     * </p>
     *
     * @return the singleton instance of {@link WifiAgent}.
     */
    public static WifiAgent getInstance() {
        return Holder.sWifiAgent;
    }

    // Holder is loaded on the first execution of WifiAgent.getInstance(), not before.
    private static class Holder {
        private static final WifiAgent sWifiAgent = new WifiAgent();
    }

    // Private constructor. Prevents instantiation from other classes.
    private WifiAgent() {
    }

    /**
     * Initializes the {@link WifiAgent} with the specified {@link Context}.
     *
     * @param context the {@link Context} to use for initializing the {@link WifiAgent}
     * @throws NullPointerException if the provided {@code context} is {@code null}
     */
    public void init(@NonNull Context context) {
        Objects.requireNonNull(context, "The provided context cannot be null.");

        mContext = context;
        mHandler = new Handler(Looper.myLooper());

        mIntentReceiver = new IntentReceiver();
        mIntentReceiver.register();

        registerNetworkCallback();
    }

    /**
     * Releases resources and resets state associated with the {@link WifiAgent}.
     */
    public void cleanup() {
        unregisterNetworkCallback();

        if (mIntentReceiver != null) {
            mIntentReceiver.unregister();
            mIntentReceiver = null;
        }

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
    }

    /**
     * Adds a listener to monitor the Wi-Fi connection state change.
     *
     * @param listener The {@link Listener} to be set.
     * @throws NullPointerException if the provided {@link Listener} is {@code null}
     */
    public void addListener(@NonNull Listener listener) {
        Objects.requireNonNull(listener, "Listener cannot be null");
        mListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The {@link Listener} to be removed.
     */
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    /**
     * Returns the current activation status of the Wi-Fi setting.
     *
     * @return {@code true} if the Wi-Fi is enabled, {@code false} otherwise.
     */
    public boolean isWifiEnabled() {
        return mIsWifiEnabled;
    }

    /**
     * Returns whether the device is currently connected to Wi-Fi.
     *
     * @return {@code true} if the device is connected to Wi-Fi, {@code false} otherwise.
     */
    public boolean isWifiConnected() {
        return mIsWifiConnected;
    }

    /**
     * Returns the {@link Network} object.
     *
     * @return a {@link Network} object representing the currently used network.
     *         or {@code null} if no network is currently in use.
     */
    @Nullable public Network getNetwork() {
        return mNetwork;
    }

    /**
     * Returns the {@link LinkProperties} object.
     *
     * @return a {@link LinkProperties} object representing the currently used LinkProperties.
     *         or {@code null} if no LinkProperties is currently in use.
     */
    @Nullable public LinkProperties getLinkProperties() {
        return mLinkProperties;
    }

    /**
     * Returns the local IP address of the specified IP version.
     * <p>
     * This method retrieves the local IP address of the specified IP version (IPv4 or IPv6) from
     * the current network link properties. If the requested IP version is available and matches
     * the specified version, it returns the first IP address of that version found in the link
     * properties.
     * <p>
     * If the link properties are not available or do not contain any IP addresses of the specified
     * version, {@code null} is returned.
     *
     * @param version the IP version (IPv4 or IPv6) of the local address to retrieve
     * @return the local IP address of the specified version, or {@code null} if not available
     */
    @Nullable public String getLocalAddress(int version) {
        if (mLinkProperties == null) {
            return null;
        }

        List<InetAddress> inetAddresses = mLinkProperties.getAddresses();

        for (InetAddress inetAddress : inetAddresses) {
            if (inetAddress == null || inetAddress.isAnyLocalAddress()
                    || inetAddress.isLinkLocalAddress() || inetAddress.isLoopbackAddress()) {
                continue;
            }

            if (version == EIpVersion.IPV6.getInt() && inetAddress instanceof Inet6Address) {
                return inetAddress.getHostAddress(); // Return the first IPv6 address found
            } else if (version == EIpVersion.IPV4.getInt() && inetAddress instanceof Inet4Address) {
                return inetAddress.getHostAddress(); // Return the first IPv4 address found
            }
        }

        return null;
    }

    /**
     * Resolves the IP addresses of the specified host based on the given IP version.
     *
     * @param version The IP version to use for resolving the host addresses.
     * @param host The hostname to resolve.
     * @return An array of IP addresses of the specified host, filtered by the given IP version.
     *         Returns {@code null} if the network is not available, the hostname cannot be
     *         resolved, or no IP addresses are found for the specified host and IP version.
     */
    @Nullable public String[] getHostByName(int version, String host) {
        if (mNetwork == null) {
            loge("getHostByName: Network is not available.");
            return null;
        }

        try {
            InetAddress[] inetAddresses = mNetwork.getAllByName(host);

            if (inetAddresses == null || inetAddresses.length == 0) {
                loge("getHostByName: Failed to resolve host or no IP addresses found for " + host);
                return null;
            }

            List<String> ipAddresses = new ArrayList<>();

            for (InetAddress inetAddr : inetAddresses) {
                if ((version == EIpVersion.IPV4.getInt() && inetAddr instanceof Inet4Address)
                        || (version == EIpVersion.IPV6.getInt()
                                && inetAddr instanceof Inet6Address)) {
                    ipAddresses.add(inetAddr.getHostAddress());
                }
            }

            logi("getHostByName: ipVersion=" + version + ", size=" + ipAddresses.size());

            return ipAddresses.toArray(new String[ipAddresses.size()]);
        } catch (UnknownHostException e) {
            loge("getHostByName: UnknownHostException occurred while resolving host: " + host);
            return null;
        } catch (SecurityException e) {
            loge("getHostByName: SecurityException occurred while resolving host: " + host);
            return null;
        }
    }

    /**
     * Binds the specified socket file descriptor to the current network.
     *
     * @param sockFd The non-null socket file descriptor to bind.
     * @return {@code true} if the socket was successfully bound, {@code false} otherwise.
     */
    public boolean bindSocket(@NonNull FileDescriptor sockFd) {
        Objects.requireNonNull(sockFd, "sockFd cannot be null");

        if (mNetwork == null) {
            loge("bindSocket: Failed to bind socket. Network is null.");
            return false;
        }

        logi("bindSocket: network=" + mNetwork);

        try {
            mNetwork.bindSocket(sockFd);
            return true;
        } catch (IOException e) {
            loge("bindSocket: Failed to bind socket: " + e.getMessage());
            return false;
        }
    }

    /**
     * Waits for the Wi-Fi connection ready.
     */
    public void waitForWifiConnected() {
        mWifiConnectedLatch.await();
    }

    private void registerNetworkCallback() {
        if (mNetworkCallback != null) {
            return;
        }

        ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
        if (cm == null) {
            return;
        }

        mNetworkCallback = new ConnectivityManager.NetworkCallback() {

            @Override
            public void onLost(@NonNull Network network) {
                handleNetworkLost(network);
            }

            @Override
            public void onLinkPropertiesChanged(@NonNull Network network,
                    @NonNull LinkProperties linkProperties) {
                handleLinkPropertiesChanged(network, linkProperties);
            }
        };

        NetworkRequest networkRequest = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .build();
        try {
            cm.registerNetworkCallback(networkRequest, mNetworkCallback, mHandler);
        } catch (NullPointerException e) {
            loge("registerNetworkCallback: Null pointer exception occurred");
        }
    }

    private void handleNetworkLost(Network network) {
        logi("handleNetworkLost: network=" + network);
        setWifiConnected(false);
        resetNetworkInfo();
        mWifiConnectedLatch.init();
    }

    private void handleLinkPropertiesChanged(Network network, LinkProperties linkProperties) {
        logi("handleLinkPropertiesChanged: network=" + network);
        mNetwork = network;
        mLinkProperties = linkProperties;
        setWifiConnected(true);
        mWifiConnectedLatch.countDown();
    }

    private void unregisterNetworkCallback() {
        if (mNetworkCallback != null) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.unregisterNetworkCallback(mNetworkCallback);
            }
            mNetworkCallback = null;
        }

        setWifiConnected(false);
        resetNetworkInfo();
    }

    private void setWifiConnected(boolean isConnected) {
        if (mIsWifiConnected != isConnected) {
            logi("setWifiConnected: " + mIsWifiConnected + " >> " + isConnected);
            mIsWifiConnected = isConnected;
            notifyWifiConnectionStateChanged();
        }
    }

    private void notifyWifiConnectionStateChanged() {
        for (Listener listener : mListeners) {
            listener.onWifiConnectionStateChanged();
        }
    }

    private void resetNetworkInfo() {
        mNetwork = null;
        mLinkProperties = null;
    }

    private class IntentReceiver extends BroadcastReceiver {
        private void register() {
            IntentFilter filter = new IntentFilter(WifiManager.WIFI_STATE_CHANGED_ACTION);
            mContext.registerReceiver(this, filter, null, mHandler, Context.RECEIVER_EXPORTED);
        }

        private void unregister() {
            mContext.unregisterReceiver(this);
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logi(Log.lastSubString(action, "."));

            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                int state = intent.getIntExtra(
                        WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN);
                setWifiEnabled(state == WifiManager.WIFI_STATE_ENABLED);
            }
        }
    }

    private void setWifiEnabled(boolean isEnabled) {
        if (mIsWifiEnabled != isEnabled) {
            logi("setWifiEnabled: " + mIsWifiEnabled + " >> " + isEnabled);
            mIsWifiEnabled = isEnabled;
            notifyWifiStateChanged();

            if (!mIsWifiEnabled) {
                mWifiConnectedLatch.init();
            }
        }
    }

    private void notifyWifiStateChanged() {
        for (Listener listener : mListeners) {
            listener.onWifiStateChanged();
        }
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "WifiAgent: " + s);
    }

    private static void loge(String s) {
        Log.e(Log.TAG, "WifiAgent: " + s);
    }
}
