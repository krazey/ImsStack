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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.ConnectivityManager;
import android.net.InetAddresses;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.wifi.WifiManager;
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
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileDescriptor;
import java.lang.reflect.Field;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.Hashtable;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class DcApnTest {
    private static final int SLOT_0 = 0;
    private static final int[] SUB_ID = { 1 };
    static ContextFixture sContext;
    FakeDcApn mDcApn;

    @Mock private Hashtable mMockMapApn;
    @Mock private IApn mMockIApn;
    @Mock private ISharedState mMockISharedState;
    @Mock private ISubscription mMockISubscription;
    @Mock private Network mMockNetwork;
    @Mock private WifiManager mMockWifiManager;
    @Mock private DcApn.PreciseDcStateListener mMockPreciseDcStateListener;

    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private ConnectivityManager mConnectivityManager;
    private WifiManager mWifiManager;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        // create the instance to test
        mDcApn = new FakeDcApn(SLOT_0);

        mTelephonyManager = AppContext.getInstance().getSystemService(TelephonyManager.class);
        mSubscriptionManager = AppContext.getInstance().getSystemService(SubscriptionManager.class);
        mConnectivityManager = AppContext.getInstance().getSystemService(ConnectivityManager.class);
        mMockWifiManager = AppContext.getInstance().getSystemService(WifiManager.class);
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
        mMockWifiManager = null;
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    public void testInitAndCleanup() throws Exception {
        when(mSubscriptionManager.getSubscriptionIds(anyInt())).thenReturn(SUB_ID);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);

        mDcApn.init(AppContext.getInstance());
        assertNotNull(mDcApn.mSubscriptionListener);
        verify(mMockISubscription).addListener(mDcApn.mSubscriptionListener);
        assertNotNull(mDcApn.mPreciseDcStateListener);
        verify(mTelephonyManager).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
        assertNotNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        assertNotNull(mDcApn.getApnControl(EApnType.EMERGENCY.getType()));
        assertNotNull(mDcApn.getApnControl(EApnType.XCAP.getType()));
        assertNotNull(mDcApn.getApnControl(EApnType.INTERNET.getType()));

        DcApn.PreciseDcStateListener backupListener = mDcApn.mPreciseDcStateListener;
        mDcApn.cleanup();
        verify(mMockISubscription).removeListener(any(SubscriptionListener.class));
        assertNull(mDcApn.mSubscriptionListener);
        verify(mTelephonyManager).unregisterTelephonyCallback(backupListener);
        assertNull(mDcApn.mPreciseDcStateListener);
        assertNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        assertNull(mDcApn.getApnControl(EApnType.EMERGENCY.getType()));
        assertNull(mDcApn.getApnControl(EApnType.XCAP.getType()));
        assertNull(mDcApn.getApnControl(EApnType.INTERNET.getType()));
    }

    @Test
    public void testConnect_NativeReady() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockISharedState.isNativeBootCompleted()).thenReturn(true);
        when(mMockMapApn.get(anyInt()))
                .thenReturn(null)
                .thenReturn(mMockIApn);
        when(mMockIApn.connect()).thenReturn(true);

        assertFalse(mDcApn.connect(EApnType.IMS.getType()));
        assertTrue(mDcApn.connect(EApnType.IMS.getType()));
        verify(mMockIApn, times(1)).connect();
    }

    @Test
    public void testConnect_NativeNotReady() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockISharedState.isNativeBootCompleted()).thenReturn(false);
        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.connect()).thenReturn(true);

        assertFalse(mDcApn.connect(EApnType.IMS.getType()));
        verify(mMockIApn, times(0)).connect();
    }

    @Test
    public void testDisconnect() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt()))
                .thenReturn(null)
                .thenReturn(mMockIApn);
        when(mMockIApn.disconnect()).thenReturn(true);

        assertFalse(mDcApn.disconnect(EApnType.IMS.getType()));
        assertTrue(mDcApn.disconnect(EApnType.IMS.getType()));
        verify(mMockIApn, times(1)).disconnect();
    }

    @Test
    public void testGetDataState() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt()))
                .thenReturn(null)
                .thenReturn(mMockIApn);
        when(mMockIApn.getDataState()).thenReturn(TelephonyManager.DATA_CONNECTED);

        assertEquals(EDataState.DATA_STATE_DISCONNECTED.getState(), mDcApn.getDataState(anyInt()));
        assertEquals(EDataState.DATA_STATE_CONNECTED.getState(), mDcApn.getDataState(anyInt()));
    }

    @Test
    public void testIsConnected() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt()))
                .thenReturn(null)
                .thenReturn(mMockIApn);
        when(mMockIApn.isConnected()).thenReturn(true);

        assertFalse(mDcApn.isConnected(anyInt()));
        assertTrue(mDcApn.isConnected(anyInt()));
    }

    @Test
    public void testGetApn() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt()))
                .thenReturn(null)
                .thenReturn(mMockIApn);
        when(mMockIApn.getApn()).thenReturn("TestApn");

        assertNull(mDcApn.getApn(anyInt()));
        assertEquals("TestApn", mDcApn.getApn(anyInt()));
    }

    @Test
    public void testGetHostByName_InvalidApn() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(null);
        mDcApn.setContext();

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_InvalidAddress() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost")).thenReturn(null);
        mDcApn.setContext();

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_DifferentIpVersion() throws Exception {
        InetAddress[] inet4Addrs = { (Inet4Address) InetAddresses.parseNumericAddress("1.1.1.1") };
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost")).thenReturn(inet4Addrs);
        mDcApn.setContext();
        mDcApn.getSubscription();
        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_InvalidContext() throws Exception {
        InetAddress[] inet6Addrs = { (Inet6Address) InetAddresses.parseNumericAddress("2001::1") };
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);
        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost")).thenReturn(inet6Addrs);

        assertNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetHostByName_Success() throws Exception {
        InetAddress[] inet6Addrs = { (Inet6Address) InetAddresses.parseNumericAddress("2001::1") };
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getAllByName("TestHost")).thenReturn(inet6Addrs);
        mDcApn.setContext();

        assertNotNull(mDcApn.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6.getInt(),
                "TestHost"));
    }

    @Test
    public void testGetIfaceId_InvalidApn() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(null);
        mDcApn.setContext();

        assertEquals(-1, mDcApn.getIfaceId(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceId_InvalidContext() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);
        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getNetId()).thenReturn(100);

        assertEquals(-1, mDcApn.getIfaceId(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceId_Success() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mMockNetwork.getNetId()).thenReturn(100);
        mDcApn.setContext();

        assertEquals(100, mDcApn.getIfaceId(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceName_NullLinkProperties() throws Exception {
        assertNull(mDcApn.getIfaceName(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceName_NullIfaceName() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertNull(mDcApn.getIfaceName(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIfaceName_Success() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.setInterfaceName("TestIface");
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);
        mDcApn.setContext();

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);

        assertNotNull(mDcApn.getIfaceName(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIpcanCategory_InvalidApn() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(null);

        assertEquals(IApn.IPCAN_CATEGORY_MOBILE, mDcApn.getIpcanCategory(EApnType.IMS.getType()));
    }

    @Test
    public void testGetIpcanCategory_Success() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpcanCategory()).thenReturn(IApn.IPCAN_CATEGORY_WLAN);

        assertEquals(IApn.IPCAN_CATEGORY_WLAN, mDcApn.getIpcanCategory(EApnType.IMS.getType()));
    }

    @Test
    public void testGetLocalAddress_NullLinkProperties() throws Exception {
        assertNull(mDcApn.getLocalAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetLocalAddress_EmptyLinkAddresses() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertNull(mDcApn.getLocalAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetLocalAddress_Success() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addLinkAddress(new LinkAddress("1.1.1.1/8"));
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertEquals("1.1.1.1", mDcApn.getLocalAddress(EApnType.IMS.getType(), 0));
        assertNull(mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetLocalAddress_SuccessAndCached() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addLinkAddress(new LinkAddress("1.1.1.1/8"));
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertEquals("1.1.1.1", mDcApn.getLocalAddress(EApnType.IMS.getType(), -1));
        assertEquals("1.1.1.1", mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetCachedLocalAddress() throws Exception {
        assertNull(mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
        mDcApn.mMapLocalIP.put(EApnType.IMS.getType(), "1.1.1.1");
        assertEquals("1.1.1.1", mDcApn.getCachedLocalAddress(EApnType.IMS.getType()));
    }

    @Test
    public void testGetPcscfAddress_NullLinkProperties() throws Exception {
        assertNull(mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetPcscfAddress_EmptyPcscfAddress() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertNull(mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetPcscfAddress_Success() throws Exception {
        String[] inet6Addrs = {"2001::1"};
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.addPcscfServer((Inet6Address) InetAddresses.parseNumericAddress("2001::1"));
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertEquals(inet6Addrs, mDcApn.getPcscfAddress(EApnType.IMS.getType(), 0));
    }

    @Test
    public void testGetMtu_NullLinkProperties() throws Exception {
        assertEquals(0, mDcApn.getMtu(EApnType.IMS.getType()));
    }

    @Test
    public void testGetMtu_Success() throws Exception {
        LinkProperties linkProperties = new LinkProperties();
        linkProperties.setMtu(1500);
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getIpVersion()).thenReturn(EIpVersion.IPV4V6.getInt());
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        when(mConnectivityManager.getLinkProperties(mMockNetwork)).thenReturn(linkProperties);
        mDcApn.setContext();

        assertEquals(1500, mDcApn.getMtu(EApnType.IMS.getType()));
    }

    @Test
    public void testBindSocket_NullNetwork() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(null);

        assertEquals(0, mDcApn.bindSocket(EApnType.IMS.getType(), new FileDescriptor()));
    }

    @Test
    public void testBindSocket_Success() throws Exception {
        int sockFd = 100;
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);

        assertEquals(1, mDcApn.bindSocket(EApnType.IMS.getType(), new FileDescriptor()));
        //assertEquals(1, mDcApn.bindSocket(EApnType.IMS.getType(), any(FileDescriptor.class)));
    }

    @Test
    public void testApnControl() throws Exception {
        assertNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        mDcApn.setApn(EApnType.IMS.getType(), null);
        assertNull(mDcApn.getApnControl(EApnType.IMS.getType()));
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);
        assertEquals(mMockIApn, mDcApn.getApnControl(EApnType.IMS.getType()));
    }

    @Test
    public void testGetNetworkByCapability_WifiType() throws Exception {
        when(mMockWifiManager.getCurrentNetwork()).thenReturn(mMockNetwork);

        assertNull(mDcApn.getNetworkByCapability(EApnType.WIFI.getType()));
        mDcApn.setContext();
        assertEquals(mMockNetwork, mDcApn.getNetworkByCapability(EApnType.WIFI.getType()));
    }

    @Test
    public void testGetNetworkByCapability_NonWifiType() throws Exception {
        replaceInstance(DcApn.class, "mMapApn", mDcApn, mMockMapApn);

        when(mMockMapApn.get(anyInt())).thenReturn(mMockIApn);
        when(mMockIApn.getCachedNetwork()).thenReturn(mMockNetwork);
        mDcApn.setContext();

        assertEquals(mMockNetwork, mDcApn.getNetworkByCapability(EApnType.IMS.getType()));
    }

    @Test
    public void testChangeApnEmployState() throws Exception {
        // Do not handle changeApnEmployState() if apntype is null or can not get ApnControl
        mDcApn.changeApnEmployState(null, true);
        mDcApn.changeApnEmployState(EApnType.IMS, true);

        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);
        mDcApn.changeApnEmployState(EApnType.IMS, true);
        verify(mMockIApn, times(1)).employApn();

        mDcApn.changeApnEmployState(EApnType.IMS, false);
        verify(mMockIApn, times(1)).dismissApn();
    }

    @Test
    public void testPreciseDataConnectionStateChanged_NullApnSetting() throws Exception {
        PreciseDataConnectionState dataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(TelephonyManager.DATA_CONNECTED)
                .setNetworkType(TelephonyManager.NETWORK_TYPE_LTE)
                .build();
        mDcApn.init(AppContext.getInstance());
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn, times(0)).sendMessage(any(Message.class));
    }

    @Test
    public void testPreciseDataConnectionStateChanged_NullApn() throws Exception {
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
        mDcApn.init(AppContext.getInstance());

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn, times(0)).sendMessage(any(Message.class));
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
        mDcApn.init(AppContext.getInstance());
        mDcApn.setApn(EApnType.IMS.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn, times(0)).sendMessage(any(Message.class));
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
        mDcApn.init(AppContext.getInstance());
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
        mDcApn.init(AppContext.getInstance());
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
        mDcApn.init(AppContext.getInstance());
        mDcApn.setApn(EApnType.XCAP.getType(), mMockIApn);

        mDcApn.mPreciseDcStateListener.onPreciseDataConnectionStateChanged(dataConnectionState);
        verify(mMockIApn).sendMessage(any(Message.class));
    }

    @Test
    public void testSubscriptionListener_onSimLoadCompleted() throws Exception {
        int invalidSlotId = -1;
        int[] changedSubId = { 2 };

        when(mSubscriptionManager.getSubscriptionIds(anyInt()))
                .thenReturn(SUB_ID)
                .thenReturn(changedSubId);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        mDcApn.init(AppContext.getInstance());

        // ignore if slotId is different
        mDcApn.mSubscriptionListener.onSimLoadCompleted(invalidSlotId);
        mDcApn.mSubscriptionListener.onSimLoadCompleted(SLOT_0);

        verify(mTelephonyManager).unregisterTelephonyCallback(mDcApn.mPreciseDcStateListener);
        // register callback when init() and updateSubscription()
        verify(mTelephonyManager, times(2)).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }

    @Test
    public void testSubscriptionListener_onDefaultSubscriptionChanged() throws Exception {
        int changedSubId = 2;

        when(mSubscriptionManager.getSubscriptionIds(anyInt())).thenReturn(SUB_ID);
        when(mMockISubscription.getSubId(SLOT_0)).thenReturn(changedSubId);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        mDcApn.init(AppContext.getInstance());

        mDcApn.mSubscriptionListener.onDefaultSubscriptionChanged(changedSubId);

        verify(mTelephonyManager).unregisterTelephonyCallback(mDcApn.mPreciseDcStateListener);
        // register callback when init() and updateSubscription()
        verify(mTelephonyManager, times(2)).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }

    @Test
    public void testSubscriptionListener_onDefaultDataSubscriptionChanged() throws Exception {
        int changedSubId = 2;

        when(mSubscriptionManager.getSubscriptionIds(anyInt())).thenReturn(SUB_ID);
        when(mMockISubscription.getSubId(SLOT_0)).thenReturn(changedSubId);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        mDcApn.init(AppContext.getInstance());

        mDcApn.mSubscriptionListener.onDefaultDataSubscriptionChanged(changedSubId);

        verify(mTelephonyManager).unregisterTelephonyCallback(mDcApn.mPreciseDcStateListener);
        // register callback when init() and updateSubscription()
        verify(mTelephonyManager, times(2)).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(), mDcApn.mPreciseDcStateListener);
    }

    private class FakeDcApn extends DcApn {
        FakeDcApn(int slotId) {
            super(slotId);
        }

        @Override
        protected ISubscription getSubscription() {
            return mMockISubscription;
        }

        @Override
        protected ISharedState getSharedState(int slotId) {
            return mMockISharedState;
        }

        public void setContext() {
            mContext = AppContext.getInstance();
        }
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
