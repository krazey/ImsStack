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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.InetAddresses;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.os.Message;
import android.telephony.PreciseDataConnectionState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.ISharedState;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.SubscriptionListener;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileDescriptor;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class DcApnTest {
    private static final int SLOT_0 = 0;
    private static final int SLOT_1 = 1;
    private static final int SUB_ID_0 = 1;
    private static final int SUB_ID_1 = 2;
    private static final int[] SUB_IDS = { SUB_ID_0 };
    private static final int INVALID_APN = -1;

    private Context mContext;
    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private ConnectivityManager mConnectivityManager;
    private FakeDcApn mDcApn;

    @Mock IApn mMockIApn;
    @Mock ISharedState mMockISharedState;
    @Mock ISubscription mMockISubscription;
    @Mock Network mMockNetwork;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContext = new ContextFixture().getTestDouble();
        AppContext.init(mContext);

        mConnectivityManager = mContext.getSystemService(ConnectivityManager.class);
        mSubscriptionManager = mContext.getSystemService(SubscriptionManager.class);
        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);

        when(mSubscriptionManager.getSubscriptionIds(anyInt())).thenReturn(SUB_IDS);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_0)).thenReturn(mTelephonyManager);

        // create the instance to test
        mDcApn = new FakeDcApn(SLOT_0);
        mDcApn.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        if (mDcApn != null) {
            mDcApn.cleanup();
            mDcApn = null;
        }

        mTelephonyManager = null;
        mSubscriptionManager = null;
        mConnectivityManager = null;

        AppContext.deinit();
    }

    @Test
    public void testInit() throws Exception {
        assertEquals(mContext, mDcApn.mContext);
        assertNotNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        assertNotNull(mDcApn.getApnControl(EApnType.EMERGENCY.getType()));
        assertNotNull(mDcApn.getApnControl(EApnType.XCAP.getType()));
        assertNotNull(mDcApn.getApnControl(EApnType.INTERNET.getType()));

        assertNotNull(mDcApn.mSubscriptionListener);
        verify(mMockISubscription).addListener(mDcApn.mSubscriptionListener);

        assertNotNull(mDcApn.mPreciseDcStateListener);
        verify(mTelephonyManager).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }

    @Test
    public void testCleanup() throws Exception {
        mDcApn.cleanup();

        verify(mMockISubscription).removeListener(any(SubscriptionListener.class));
        assertNull(mDcApn.mSubscriptionListener);

        verify(mTelephonyManager).unregisterTelephonyCallback(
                any(DcApn.PreciseDcStateListener.class));
        assertNull(mDcApn.mPreciseDcStateListener);

        assertNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        assertNull(mDcApn.getApnControl(EApnType.EMERGENCY.getType()));
        assertNull(mDcApn.getApnControl(EApnType.XCAP.getType()));
        assertNull(mDcApn.getApnControl(EApnType.INTERNET.getType()));
    }

    @Test
    public void testConnect_NativeReady() throws Exception {
        when(mMockISharedState.isNativeBootCompleted()).thenReturn(true);
        when(mMockIApn.connect()).thenReturn(true);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertFalse(mDcApn.connect(INVALID_APN));
        assertTrue(mDcApn.connect(EApnType.IMS.getType()));
        verify(mMockIApn, times(1)).connect();
    }

    @Test
    public void testConnect_NativeNotReady() throws Exception {
        when(mMockISharedState.isNativeBootCompleted()).thenReturn(false);
        when(mMockIApn.connect()).thenReturn(true);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertFalse(mDcApn.connect(EApnType.IMS.getType()));
        verify(mMockIApn, never()).connect();
    }

    @Test
    public void testDisconnect() throws Exception {
        when(mMockIApn.disconnect()).thenReturn(true);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertFalse(mDcApn.disconnect(INVALID_APN));
        assertTrue(mDcApn.disconnect(EApnType.IMS.getType()));
        verify(mMockIApn, times(1)).disconnect();
    }

    @Test
    public void testGetDataState() throws Exception {
        when(mMockIApn.getDataState()).thenReturn(TelephonyManager.DATA_CONNECTED);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(EDataState.DATA_STATE_DISCONNECTED.getState(),
                mDcApn.getDataState(INVALID_APN));
        assertEquals(EDataState.DATA_STATE_CONNECTED.getState(),
                mDcApn.getDataState(EApnType.IMS.getType()));
    }

    @Test
    public void testIsConnected() throws Exception {
        when(mMockIApn.isConnected()).thenReturn(true);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertFalse(mDcApn.isConnected(INVALID_APN));
        assertTrue(mDcApn.isConnected(EApnType.IMS.getType()));
    }

    @Test
    public void testGetApn() throws Exception {
        when(mMockIApn.getApn()).thenReturn("TestApn");
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getApn(INVALID_APN));
        assertEquals("TestApn", mDcApn.getApn(EApnType.IMS.getType()));
    }

    @Test
    public void testGetHostByName_InvalidContext() throws Exception {
        InetAddress[] inet6Addrs = { (Inet6Address) InetAddresses.parseNumericAddress("2001::1") };

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost")).thenReturn(inet6Addrs);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);
        setContext(null);

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_InvalidNetwork() throws Exception {
        when(mMockIApn.getCachedNetwork()).thenReturn(null);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_InvalidAddress() throws Exception {
        InetAddress[] inet6Addrs = {};

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost"))
                .thenReturn(null)
                .thenReturn(inet6Addrs);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_DifferentIpVersion() throws Exception {
        InetAddress[] inet4Addrs = { (Inet4Address) InetAddresses.parseNumericAddress("1.1.1.1") };

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost")).thenReturn(inet4Addrs);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_Success() throws Exception {
        InetAddress[] inet4Addrs = { (Inet4Address) InetAddresses.parseNumericAddress("1.1.1.1") };
        InetAddress[] inet6Addrs = { (Inet6Address) InetAddresses.parseNumericAddress("2001::1") };

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost"))
                .thenReturn(inet4Addrs)
                .thenReturn(inet6Addrs);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNotNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV4.getInt(),
                "TestHost"));
        assertNotNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetIfaceId_InvalidContext() throws Exception {
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getNetId()).thenReturn(100);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);
        setContext(null);

        assertEquals(-1, mDcApn.getIfaceId(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceId_InvalidNetwork() throws Exception {
        when(mMockIApn.getCachedNetwork()).thenReturn(null);
        when(mMockNetwork.getNetId()).thenReturn(100);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(-1, mDcApn.getIfaceId(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceId_Success() throws Exception {
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getNetId()).thenReturn(100);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(100, mDcApn.getIfaceId(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceName_NullLinkProperties() throws Exception {
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(null);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getIfaceName(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceName_NullIfaceName() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.setInterfaceName(null);

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getIfaceName(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceName_Success() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.setInterfaceName("TestIface");

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNotNull(mDcApn.getIfaceName(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIpcanCategory_InvalidApn() throws Exception {
        mDcApn.mMapApn.clear();

        assertNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        assertEquals(IApn.IPCAN_CATEGORY_MOBILE, mDcApn.getIpcanCategory(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIpcanCategory_Success() throws Exception {
        when(mMockIApn.getIpcanCategory()).thenReturn(IApn.IPCAN_CATEGORY_WLAN);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(IApn.IPCAN_CATEGORY_WLAN, mDcApn.getIpcanCategory(EApnType.IMS.getType()));
    }

    @Test
    public void testGetLocalAddress_NullLinkProperties() throws Exception {
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(null);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getLocalAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetLocalAddress_EmptyLinkAddresses() throws Exception {
        LinkProperties linkProperties = new LinkProperties();

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getLocalAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetLocalAddress_DifferentIpVersion() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addLinkAddress(new LinkAddress("1.1.1.1/8"));

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);

        mDcApn.mMapLocalIP.put(EApnType.IMS.getType(), "2.2.2.2");
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals("2.2.2.2", mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
        assertNull(mDcApn.getLocalAddress(EApnType.IMS.getType(), -1));
        assertNull(mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetLocalAddress_SuccessButNotCached() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addLinkAddress(new LinkAddress("1.1.1.1/8"));

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals("1.1.1.1", mDcApn.getLocalAddress(EApnType.IMS.getType(), 0));
        assertNull(mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetLocalAddress_SuccessAndCachedIpv4() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addLinkAddress(new LinkAddress("2001::1/63"));
        linkProperties.addLinkAddress(new LinkAddress("1.1.1.1/8"));

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals("1.1.1.1", mDcApn.getLocalAddress(EApnType.IMS.getType(), -1));
        assertEquals("1.1.1.1", mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetLocalAddress_SuccessAndCachedIpv6() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addLinkAddress(new LinkAddress("1.1.1.1/8"));
        linkProperties.addLinkAddress(new LinkAddress("2001::1/63"));

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV6V4.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals("2001::1", mDcApn.getLocalAddress(EApnType.IMS.getType(), -1));
        assertEquals("2001::1", mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetCachedLocalAddress() throws Exception {
        assertNull(mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
        mDcApn.mMapLocalIP.put(EApnType.IMS.getType(), "1.1.1.1");
        assertEquals("1.1.1.1", mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetPcscfAddress_NullLinkProperties() throws Exception {
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(null);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetPcscfAddress_EmptyPcscfAddress() throws Exception {
        LinkProperties linkProperties = new LinkProperties();

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetPcscfAddress_SuccessIpv4() throws Exception {
        String[] inet4Addrs = {"1.1.1.1"};
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addPcscfServer(InetAddresses.parseNumericAddress(inet4Addrs[0]));

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV6V4.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(inet4Addrs, mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetPcscfAddress_SuccessIpv6() throws Exception {
        String[] inet6Addrs = {"2001::1"};
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addPcscfServer((Inet6Address) InetAddresses.parseNumericAddress(
                inet6Addrs[0]));

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(inet6Addrs, mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetMtu_NullLinkProperties() throws Exception {
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(null);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(0, mDcApn.getMtu(EApnType.IMS.getType()));
    }

    @Test
    public void testGetMtu_Success() throws Exception {
        int testMtu = 1500;
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.setMtu(testMtu);

        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(testMtu, mDcApn.getMtu(EApnType.IMS.getType()));
    }

    @Test
    public void testBindSocket_NullNetwork() throws Exception {
        FileDescriptor sockFd = new FileDescriptor();

        when(mMockIApn.getCachedNetwork()).thenReturn(null);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(0, mDcApn.bindSocket(EApnType.IMS.getType(), sockFd));
    }

    @Test
    public void testBindSocket_Success() throws Exception {
        FileDescriptor sockFd = new FileDescriptor();

        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertEquals(1, mDcApn.bindSocket(EApnType.IMS.getType(), sockFd));
        verify(mMockNetwork).bindSocket(sockFd);
    }

    @Test
    public void testApnControl() throws Exception {
        assertNotEquals(mMockIApn, mDcApn.getApnControl(EApnType.IMS.getType()));

        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);
        assertEquals(mMockIApn, mDcApn.getApnControl(EApnType.IMS.getType()));

        // null apn is not updated
        mDcApn.setApn(EApnType.IMS.getType(), null);
        assertEquals(mMockIApn, mDcApn.getApnControl(EApnType.IMS.getType()));
    }

    @Test
    public void testGetNetworkByCapability() throws Exception {
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        assertNull(mDcApn.getNetworkByCapability(INVALID_APN));
        assertEquals(mMockNetwork, mDcApn.getNetworkByCapability(EApnType.IMS.getType()));
    }

    @Test
    public void testPreciseDataConnectionStateChanged_NullApnSetting() throws Exception {
        PreciseDataConnectionState dataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(TelephonyManager.DATA_CONNECTED)
                .setNetworkType(TelephonyManager.NETWORK_TYPE_LTE)
                .build();
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn, never()).sendMessage(any(Message.class));
    }

    @Test
    public void testPreciseDataConnectionStateChanged_SuspendedState() throws Exception {
        PreciseDataConnectionState dataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(TelephonyManager.DATA_SUSPENDED)
                .setNetworkType(TelephonyManager.NETWORK_TYPE_LTE)
                .setApnSetting(new ApnSetting.Builder()
                        .setApnTypeBitmask(ApnSetting.TYPE_IMS)
                        .setApnName("TestApn")
                        .setEntryName("Test")
                        .setProtocol(ApnSetting.PROTOCOL_IPV6)
                        .build())
                .build();
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn, never()).sendMessage(any(Message.class));
    }

    @Test
    public void testPreciseDataConnectionStateChanged_ImsType() throws Exception {
        PreciseDataConnectionState dataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(TelephonyManager.DATA_CONNECTED)
                .setNetworkType(TelephonyManager.NETWORK_TYPE_LTE)
                .setApnSetting(new ApnSetting.Builder()
                        .setApnTypeBitmask(ApnSetting.TYPE_IMS)
                        .setApnName("TestApn")
                        .setEntryName("Test")
                        .setProtocol(ApnSetting.PROTOCOL_IPV6)
                        .build())
                .build();
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn).sendMessage(any(Message.class));
    }

    @Test
    public void testPreciseDataConnectionStateChanged_EmergencyType() throws Exception {
        PreciseDataConnectionState dataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(TelephonyManager.DATA_CONNECTED)
                .setNetworkType(TelephonyManager.NETWORK_TYPE_LTE)
                .setApnSetting(new ApnSetting.Builder()
                        .setApnTypeBitmask(ApnSetting.TYPE_EMERGENCY)
                        .setApnName("TestApn")
                        .setEntryName("Test")
                        .setProtocol(ApnSetting.PROTOCOL_IPV6)
                        .build())
                .build();
        mDcApn.setApn(EApnType.EMERGENCY.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn).sendMessage(any(Message.class));
    }

    @Test
    public void testPreciseDataConnectionStateChanged_XcapType() throws Exception {
        PreciseDataConnectionState dataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(TelephonyManager.DATA_CONNECTED)
                .setNetworkType(TelephonyManager.NETWORK_TYPE_LTE)
                .setApnSetting(new ApnSetting.Builder()
                        .setApnTypeBitmask(ApnSetting.TYPE_XCAP)
                        .setApnName("TestApn")
                        .setEntryName("Test")
                        .setProtocol(ApnSetting.PROTOCOL_IPV6)
                        .build())
                .build();
        mDcApn.setApn(EApnType.XCAP.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn).sendMessage(any(Message.class));
    }

    @Test
    public void testSubscriptionListener_onSimLoadCompleted() throws Exception {
        mDcApn.mFakeSlotIndex = SLOT_1;
        when(mMockISubscription.getSubId(SLOT_0)).thenReturn(SUB_ID_1);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_1)).thenReturn(mTelephonyManager);

        mDcApn.mSubscriptionListener.onSimLoadCompleted(SLOT_0);

        verify(mTelephonyManager).unregisterTelephonyCallback(mDcApn.mPreciseDcStateListener);
        // register callback when init() and updateSubscription()
        verify(mTelephonyManager, times(2)).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }

    @Test
    public void testSubscriptionListener_onDefaultSubscriptionChanged() throws Exception {
        mDcApn.mFakeSlotIndex = SLOT_1;
        when(mMockISubscription.getSubId(SLOT_0)).thenReturn(SUB_ID_1);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_1)).thenReturn(mTelephonyManager);

        mDcApn.mSubscriptionListener.onDefaultSubscriptionChanged(SUB_ID_1);

        verify(mTelephonyManager).unregisterTelephonyCallback(mDcApn.mPreciseDcStateListener);
        // register callback when init() and updateSubscription()
        verify(mTelephonyManager, times(2)).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }

    @Test
    public void testSubscriptionListener_onDefaultDataSubscriptionChanged() throws Exception {
        mDcApn.mFakeSlotIndex = SLOT_1;
        when(mMockISubscription.getSubId(SLOT_0)).thenReturn(SUB_ID_1);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_1)).thenReturn(mTelephonyManager);

        mDcApn.mSubscriptionListener.onDefaultDataSubscriptionChanged(SUB_ID_1);

        verify(mTelephonyManager).unregisterTelephonyCallback(mDcApn.mPreciseDcStateListener);
        // register callback when init() and updateSubscription()
        verify(mTelephonyManager, times(2)).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }


    @Test
    public void testSubscriptionListener_updateSubscriptionFail() throws Exception {
        mMockISubscription = null;

        mDcApn.mSubscriptionListener.onDefaultDataSubscriptionChanged(SUB_ID_0);
        mDcApn.mFakeSlotIndex = SLOT_1;
        mDcApn.mSubscriptionListener.onDefaultDataSubscriptionChanged(SUB_ID_0);

        verify(mTelephonyManager, never()).unregisterTelephonyCallback(
                mDcApn.mPreciseDcStateListener);
    }

    private class FakeDcApn extends DcApn {
        int mFakeSlotIndex = SLOT_0;

        FakeDcApn(int slotId) {
            super(slotId);
        }

        @Override
        protected ISubscription getSubscription() {
            super.getSubscription();
            return mMockISubscription;
        }

        @Override
        protected ISharedState getSharedState(int slotId) {
            super.getSharedState(slotId);
            return mMockISharedState;
        }

        @Override
        protected int getSlotId(int subId) {
            super.getSlotId(subId);
            return mFakeSlotIndex;
        }
    }

    private void setContext(Context context) {
        mDcApn.mContext = context;
    }
}
