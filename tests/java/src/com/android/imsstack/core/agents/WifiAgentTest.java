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

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.InetAddresses;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;
import java.util.List;

@RunWith(JUnit4.class)
public class WifiAgentTest {
    private static final int TEST_NETWORK_ID = 100;
    private static final String TEST_IFACE_NAME = "test_wlan0";
    private static final int TEST_MTU = 1492;
    private static final String TEST_ADDRV4 = "192.168.0.1";
    private static final String TEST_ADDRV6 = "fc01:abab:cdcd:efe3:d8cc:877c:ba3:6741";
    private static final LinkAddress TEST_LINK_ADDRV4 = new LinkAddress(TEST_ADDRV4 + "/32");
    private static final LinkAddress TEST_LINK_ADDRV6 =
            new LinkAddress(TEST_ADDRV6 + "/64");
    private static final String TEST_HOST_NAME = "ims.com";
    private static final String TEST_SSID = "TestSsid1";
    private static final String TEST_BSSID = "01:02:03:04:05:06";

    @Mock Network mNetwork;
    @Mock SystemInterface mSystemInterface;
    @Mock WifiInterface.Listener mWifiListener;
    // Mock objects
    private ConnectivityManager mConnectivityManager;

    private final LinkProperties mLinkProperties;
    private final NetworkCapabilities mNetworkCapabilities;
    private final List<InetAddress> mHostAddrs;
    private ContextFixture mContextFixture;
    private Context mContext;
    private TestableLooper mTestableLooper;
    private WifiAgent mWifiAgent;

    public WifiAgentTest() {
        mLinkProperties = new LinkProperties();
        mLinkProperties.setInterfaceName(TEST_IFACE_NAME);
        mLinkProperties.setMtu(TEST_MTU);
        mLinkProperties.setLinkAddresses(List.of(TEST_LINK_ADDRV6, TEST_LINK_ADDRV4));

        WifiInfo wifiInfo = new WifiInfo.Builder()
                .setSsid(TEST_SSID.getBytes(StandardCharsets.UTF_8))
                .setBssid(TEST_BSSID)
                .build();
        mNetworkCapabilities = new NetworkCapabilities.Builder()
                .setTransportInfo(wifiInfo)
                .build();

        mHostAddrs = List.of(
                InetAddresses.parseNumericAddress(TEST_ADDRV6),
                InetAddresses.parseNumericAddress(TEST_ADDRV4));
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        AppContext.init(mContext);
        mConnectivityManager = mContext.getSystemService(ConnectivityManager.class);
        SystemInterface.setSystemInterface(mSystemInterface);
        mTestableLooper = new TestableLooper(AppContext.getInstance().getMainLooper());

        when(mNetwork.getNetId()).thenReturn(TEST_NETWORK_ID);
        when(mNetwork.getAllByName(eq(TEST_HOST_NAME)))
                .thenReturn(mHostAddrs.toArray(new InetAddress[0]));

        mWifiAgent = new WifiAgent();
        mWifiAgent.init(mContext);
        mWifiAgent.addListener(mWifiListener);
    }

    @After
    public void tearDown() throws Exception {
        if (mWifiAgent != null) {
            mWifiAgent.removeListener(mWifiListener);
            mWifiAgent.cleanup();
            mWifiAgent = null;
        }

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        SystemInterface.setSystemInterface(null);
        mSystemInterface = null;
        mWifiListener = null;
        mNetwork = null;

        AppContext.deinit();
        mContext = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testInit() {
        ArgumentCaptor<IntentFilter> intentFilterCaptor =
                ArgumentCaptor.forClass(IntentFilter.class);
        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                intentFilterCaptor.capture(), any(), any(Handler.class), anyInt());
        IntentFilter intentFilter = intentFilterCaptor.getValue();
        assertNotNull(intentFilter);
        assertTrue(intentFilter.hasAction(WifiManager.WIFI_STATE_CHANGED_ACTION));

        ArgumentCaptor<NetworkRequest> networkRequestCaptor =
                ArgumentCaptor.forClass(NetworkRequest.class);
        verify(mConnectivityManager).registerNetworkCallback(networkRequestCaptor.capture(),
                any(ConnectivityManager.NetworkCallback.class), any(Handler.class));
        NetworkRequest networkRequest = networkRequestCaptor.getValue();
        assertNotNull(networkRequest);
        assertTrue(networkRequest.hasTransport(NetworkCapabilities.TRANSPORT_WIFI));
    }

    @Test
    @SmallTest
    public void testCleanup() {
        mWifiAgent.cleanup();

        verify(mConnectivityManager).unregisterNetworkCallback(
                any(ConnectivityManager.NetworkCallback.class));
        verify(mContext).unregisterReceiver(any(BroadcastReceiver.class));
    }

    @Test
    @SmallTest
    public void testWifiStateChanged() {
        assertFalse(mWifiAgent.isWifiEnabled());

        BroadcastReceiver receiver = getBroadcastReceiver();
        assertNotNull(receiver);

        receiver.onReceive(mContext, createWifiStateChangedIntent(WifiManager.WIFI_STATE_ENABLED));

        assertTrue(mWifiAgent.isWifiEnabled());

        receiver.onReceive(mContext, createWifiStateChangedIntent(WifiManager.WIFI_STATE_DISABLED));

        assertFalse(mWifiAgent.isWifiEnabled());
        // 2 times: enabled, disabled
        verify(mWifiListener, times(2)).onWifiStateChanged();
        verify(mSystemInterface, never()).notifyWifiStateChanged(anyInt());

        mWifiAgent.removeListener(mWifiListener);
        receiver.onReceive(mContext, createWifiStateChangedIntent(WifiManager.WIFI_STATE_ENABLED));

        assertTrue(mWifiAgent.isWifiEnabled());
        verifyNoMoreInteractions(mWifiListener);
    }

    @Test
    @SmallTest
    public void testWifiStateChangedWhenWifiServiceRequested() {
        mWifiAgent.requestWifiService(true);
        processAllMessages();

        assertFalse(mWifiAgent.isWifiEnabled());

        BroadcastReceiver receiver = getBroadcastReceiver();
        assertNotNull(receiver);

        receiver.onReceive(mContext, createWifiStateChangedIntent(WifiManager.WIFI_STATE_ENABLED));

        assertTrue(mWifiAgent.isWifiEnabled());
        verify(mSystemInterface).notifyWifiStateChanged(eq(WifiInterface.STATE_ENABLED));

        receiver.onReceive(mContext, createWifiStateChangedIntent(WifiManager.WIFI_STATE_DISABLED));

        assertFalse(mWifiAgent.isWifiEnabled());
        // 2 times: enabled, disabled
        verify(mWifiListener, times(2)).onWifiStateChanged();
        // 2 times: requestWifiService, disabled
        verify(mSystemInterface, times(2)).notifyWifiStateChanged(eq(WifiInterface.STATE_DISABLED));
    }

    @Test
    @SmallTest
    public void testWifiConnectionStateChanged() {
        assertFalse(mWifiAgent.isWifiConnected());
        assertNull(mWifiAgent.getNetwork());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertTrue(mWifiAgent.isWifiConnected());
        assertNotNull(mWifiAgent.getNetwork());

        networkCallback.onLost(mNetwork);

        assertFalse(mWifiAgent.isWifiConnected());
        assertNull(mWifiAgent.getNetwork());
        // 2 times: connected, disconnected
        verify(mWifiListener, times(2)).onWifiConnectionStateChanged();
        verify(mSystemInterface, never()).notifyWifiConnectionStateChanged(anyInt());

        mWifiAgent.removeListener(mWifiListener);
        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertTrue(mWifiAgent.isWifiConnected());
        assertNotNull(mWifiAgent.getNetwork());
        verifyNoMoreInteractions(mWifiListener);
    }

    @Test
    @SmallTest
    public void testWifiConnectionStateChangedWhenWifiServiceRequested() {
        mWifiAgent.requestWifiService(true);
        processAllMessages();

        assertFalse(mWifiAgent.isWifiConnected());
        assertNull(mWifiAgent.getNetwork());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertTrue(mWifiAgent.isWifiConnected());
        assertNotNull(mWifiAgent.getNetwork());
        verify(mSystemInterface).notifyWifiConnectionStateChanged(
                eq(WifiInterface.CONNECTION_STATE_CONNECTED));

        networkCallback.onLost(mNetwork);

        assertFalse(mWifiAgent.isWifiConnected());
        assertNull(mWifiAgent.getNetwork());
        // 2 times: connected, disconnected
        verify(mWifiListener, times(2)).onWifiConnectionStateChanged();
        // 2 times: requestWifiService, disconnected
        verify(mSystemInterface, times(2)).notifyWifiConnectionStateChanged(
                    eq(WifiInterface.CONNECTION_STATE_DISCONNECTED));
    }

    @Test
    @SmallTest
    public void testGetIfaceId() {
        assertEquals(-1, mWifiAgent.getIfaceId());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertEquals(TEST_NETWORK_ID, mWifiAgent.getIfaceId());

        networkCallback.onLost(mNetwork);

        assertEquals(-1, mWifiAgent.getIfaceId());
    }

    @Test
    @SmallTest
    public void testGetIfaceName() {
        assertNull(mWifiAgent.getIfaceName());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertEquals(TEST_IFACE_NAME, mWifiAgent.getIfaceName());

        networkCallback.onLost(mNetwork);

        assertNull(mWifiAgent.getIfaceName());
    }

    @Test
    @SmallTest
    public void testGetMtu() {
        assertEquals(0, mWifiAgent.getMtu());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertEquals(TEST_MTU, mWifiAgent.getMtu());

        networkCallback.onLost(mNetwork);

        assertEquals(0, mWifiAgent.getMtu());
    }

    @Test
    @SmallTest
    public void testGetLocalAddress() {
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV4.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV4V6.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV6.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV6V4.getInt()));

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertEquals(TEST_ADDRV4, mWifiAgent.getLocalAddress(EIpVersion.IPV4.getInt()));
        assertEquals(TEST_ADDRV4, mWifiAgent.getLocalAddress(EIpVersion.IPV4V6.getInt()));
        assertEquals(TEST_ADDRV6, mWifiAgent.getLocalAddress(EIpVersion.IPV6.getInt()));
        assertEquals(TEST_ADDRV6, mWifiAgent.getLocalAddress(EIpVersion.IPV6V4.getInt()));

        // IPv6 only
        LinkProperties lp = new LinkProperties();
        lp.setLinkAddresses(List.of(TEST_LINK_ADDRV6));
        networkCallback.onLinkPropertiesChanged(mNetwork, lp);

        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV4.getInt()));
        assertEquals(TEST_ADDRV6, mWifiAgent.getLocalAddress(EIpVersion.IPV4V6.getInt()));
        assertEquals(TEST_ADDRV6, mWifiAgent.getLocalAddress(EIpVersion.IPV6.getInt()));
        assertEquals(TEST_ADDRV6, mWifiAgent.getLocalAddress(EIpVersion.IPV6V4.getInt()));

        // IPv4 only
        lp = new LinkProperties();
        lp.setLinkAddresses(List.of(TEST_LINK_ADDRV4));
        networkCallback.onLinkPropertiesChanged(mNetwork, lp);

        assertEquals(TEST_ADDRV4, mWifiAgent.getLocalAddress(EIpVersion.IPV4.getInt()));
        assertEquals(TEST_ADDRV4, mWifiAgent.getLocalAddress(EIpVersion.IPV4V6.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV6.getInt()));
        assertEquals(TEST_ADDRV4, mWifiAgent.getLocalAddress(EIpVersion.IPV6V4.getInt()));

        networkCallback.onLost(mNetwork);

        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV4.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV4V6.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV6.getInt()));
        assertNull(mWifiAgent.getLocalAddress(EIpVersion.IPV6V4.getInt()));
    }

    @Test
    @SmallTest
    public void testGetHostByName() throws UnknownHostException {
        final String[] ipv4Address = { TEST_ADDRV4 };
        final String[] ipv6Address = { TEST_ADDRV6 };

        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV4.getInt(), TEST_HOST_NAME));
        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV6.getInt(), TEST_HOST_NAME));

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertArrayEquals(ipv4Address,
                mWifiAgent.getHostByName(EIpVersion.IPV4.getInt(), TEST_HOST_NAME));
        assertArrayEquals(ipv6Address,
                mWifiAgent.getHostByName(EIpVersion.IPV6.getInt(), TEST_HOST_NAME));

        // IPv6 only
        List<InetAddress> hostAddrs = List.of(InetAddresses.parseNumericAddress(TEST_ADDRV6));
        when(mNetwork.getAllByName(eq(TEST_HOST_NAME)))
                .thenReturn(hostAddrs.toArray(new InetAddress[0]));

        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV4.getInt(), TEST_HOST_NAME));
        assertArrayEquals(ipv6Address,
                mWifiAgent.getHostByName(EIpVersion.IPV6.getInt(), TEST_HOST_NAME));

        // IPv4 only
        hostAddrs = List.of(InetAddresses.parseNumericAddress(TEST_ADDRV4));
        when(mNetwork.getAllByName(eq(TEST_HOST_NAME)))
                .thenReturn(hostAddrs.toArray(new InetAddress[0]));

        assertArrayEquals(ipv4Address,
                mWifiAgent.getHostByName(EIpVersion.IPV4.getInt(), TEST_HOST_NAME));
        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV6.getInt(), TEST_HOST_NAME));

        // UnknownHostException
        doAnswer((invocation) -> {
            throw new UnknownHostException("Unknown host name.");
        }).when(mNetwork).getAllByName(eq(TEST_HOST_NAME));

        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV4.getInt(), TEST_HOST_NAME));
        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV6.getInt(), TEST_HOST_NAME));

        networkCallback.onLost(mNetwork);

        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV4.getInt(), TEST_HOST_NAME));
        assertNull(mWifiAgent.getHostByName(EIpVersion.IPV6.getInt(), TEST_HOST_NAME));
    }

    @Test
    @SmallTest
    public void testGetBssId() {
        assertNull(mWifiAgent.getBssId());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertEquals(TEST_BSSID, mWifiAgent.getBssId());

        networkCallback.onLost(mNetwork);

        assertNull(mWifiAgent.getBssId());
    }

    @Test
    @SmallTest
    public void testGetSsId() {
        assertNull(mWifiAgent.getSsId());

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertEquals(TEST_SSID, mWifiAgent.getSsId());

        networkCallback.onLost(mNetwork);

        assertNull(mWifiAgent.getSsId());
    }

    @Test
    @SmallTest
    public void testBindSocket() throws IOException {
        FileDescriptor sockFd = new FileDescriptor();
        assertFalse(mWifiAgent.bindSocket(sockFd));

        ConnectivityManager.NetworkCallback networkCallback = getNetworkCallback();
        assertNotNull(networkCallback);

        networkCallback.onCapabilitiesChanged(mNetwork, mNetworkCapabilities);
        networkCallback.onLinkPropertiesChanged(mNetwork, mLinkProperties);

        assertTrue(mWifiAgent.bindSocket(sockFd));

        // IOException
        doAnswer((invocation) -> {
            throw new IOException("bindSocket failed.");
        }).when(mNetwork).bindSocket(eq(sockFd));

        assertFalse(mWifiAgent.bindSocket(sockFd));

        networkCallback.onLost(mNetwork);

        assertFalse(mWifiAgent.bindSocket(sockFd));
    }

    private Intent createWifiStateChangedIntent(int state) {
        return new Intent(WifiManager.WIFI_STATE_CHANGED_ACTION)
                .putExtra(WifiManager.EXTRA_WIFI_STATE, state);
    }

    private BroadcastReceiver getBroadcastReceiver() {
        ArgumentCaptor<BroadcastReceiver> receiverCaptor =
                ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mContext).registerReceiver(receiverCaptor.capture(),
                any(IntentFilter.class), any(), any(Handler.class), anyInt());
        return receiverCaptor.getValue();
    }

    private ConnectivityManager.NetworkCallback getNetworkCallback() {
        ArgumentCaptor<ConnectivityManager.NetworkCallback> networkCallbackCaptor =
                ArgumentCaptor.forClass(ConnectivityManager.NetworkCallback.class);
        verify(mConnectivityManager).registerNetworkCallback(any(NetworkRequest.class),
                networkCallbackCaptor.capture(), any(Handler.class));
        return networkCallbackCaptor.getValue();
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
