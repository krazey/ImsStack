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

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.net.InetAddresses;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Message;
import android.telephony.DataFailCause;
import android.telephony.PreciseDataConnectionState;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.system.ISystem;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;
import java.net.InetAddress;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ApnTest {
    FakeApn mApn;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private Apn.ImsNetworkCallback mMockNetworkMonitoringCallback;
    @Mock private IApn.Listener mMockApnListener;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcSettings mMockIDcSettings;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private ISystem mMockISystem;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private Network mMockNetwork;
    @Mock private MsgProcInterface mMockMsgProc;

    private ContextFixture mContextFixture;
    private Context mContext;
    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private ConnectivityManagerProxy mConnectivityManagerProxy;
    private PreciseDataConnectionState mPreciseDataConnectionState;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        // create the instance to test
        mApn = new FakeApn(mContext, SLOT0);
        mConnectivityManagerProxy =
                mTestAppContext.getSystemServiceProxy(ConnectivityManagerProxy.class);

        mTestableLooper = TestableLooper.get(ApnTest.this);
        mTestableLooper.processAllMessages();

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT0);
        replaceInstance(Apn.class, "mSystem", mApn, mMockISystem);
    }

    @After
    public void tearDown() throws Exception {
        if (mApn != null) {
            mApn.cleanup();
            mApn = null;
        }
        if (mPreciseDataConnectionState != null) {
            mPreciseDataConnectionState = null;
        }
        mTestableLooper = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        mContextFixture = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testCleanup() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApn, mMockNetworkCallback);
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);
        mApn.registerConfigListener();
        mApn.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        mApn.cleanup();
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkMonitoringCallback);
        verify(mMockNetworkMonitoringCallback).cleanUp();
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        verify(mMockNetworkCallback).cleanUp();

        assertNull(mApn.mNetworkMonitoringCallback);
        assertNull(mApn.mNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApn.getApnReqState());
        verify(mMockISystem).notifyDataConnectionStateChanged(
                mApn.mType.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testGetContext() throws Exception {
        assertEquals(mContext, mApn.getContext());
    }

    @Test
    public void testListener() throws Exception {
        int ipcanCategory = 0;
        int handoverState = 0;
        int networkType = 0;
        int failCause = 0;

        mApn.addListener(mMockApnListener);
        mApn.notifyIpcanCategoryChanged(ipcanCategory);
        mApn.notifyHandoverStateChanged(handoverState, networkType, failCause);

        mApn.removeListener(mMockApnListener);
        mApn.notifyIpcanCategoryChanged(ipcanCategory);
        mApn.notifyHandoverStateChanged(handoverState, networkType, failCause);

        verify(mMockApnListener).onIpcanCategoryChanged(mApn.mType.getType(), ipcanCategory);
        verify(mMockApnListener).onHandoverStateChanged(handoverState, networkType, failCause);
    }

    @Test
    public void testConnect() throws Exception {
        assertFalse(mApn.connect());
    }

    @Test
    public void testDisconnect() throws Exception {
        assertFalse(mApn.disconnect());
    }

    @Test
    public void testGetApn_BeforeUpdate() throws Exception {
        assertEquals(EApnType.IMS.getString(), mApn.getApn());
    }

    @Test
    public void testGetApn_afterUpdate() throws Exception {
        String testApnString = "TestApn";

        mApn.mApnString = testApnString;
        assertEquals(testApnString, mApn.getApn());
    }

    @Test
    public void testDataStateInterface() throws Exception {
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.getDataState());
        mApn.setDataState(TelephonyManager.DATA_CONNECTED);
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.getDataState());
    }

    @Test
    public void testGetIpcanCategory() throws Exception {
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.getIpcanCategory());
        mApn.notifyDataConnectionIpcanChanged(TelephonyManager.NETWORK_TYPE_IWLAN);
        assertEquals(Apn.IPCAN_CATEGORY_WLAN, mApn.getIpcanCategory());
    }

    @Test
    public void testGetIpVersion_imsApn() throws Exception {
        replaceInstance(Apn.class, "mDcSettings", mApn, mMockIDcSettings);
        when(mMockIDcSettings.getPreferredIpVersion())
                .thenReturn(CarrierConfig.Assets.IPV4_PREFERRED);

        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApn.mApnProtocol = ApnSetting.PROTOCOL_IPV4V6;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());

        mApn.mApnProtocol = ApnSetting.PROTOCOL_IP;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());

        mApn.mApnProtocol = ApnSetting.PROTOCOL_IPV6;
        assertEquals(EIpVersion.IPV6V4.getInt(), mApn.getIpVersion());

        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());
    }

    @Test
    public void testGetIpVersion_emergencyApn() throws Exception {
        mApn.mType = EApnType.EMERGENCY;
        replaceInstance(Apn.class, "mDcSettings", mApn, mMockIDcSettings);
        when(mMockIDcSettings.getEmergencyPreferredIpVersion())
                .thenReturn(CarrierConfig.Assets.IPV4_PREFERRED);

        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApn.mApnProtocol = ApnSetting.PROTOCOL_IPV4V6;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());

        mApn.mApnProtocol = ApnSetting.PROTOCOL_IP;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());

        mApn.mApnProtocol = ApnSetting.PROTOCOL_IPV6;
        assertEquals(EIpVersion.IPV6V4.getInt(), mApn.getIpVersion());

        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());
    }

    @Test
    public void testGetIpVersion_internetApn() throws Exception {
        mApn.mType = EApnType.INTERNET;
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApn.mApnProtocol = ApnSetting.PROTOCOL_IPV4V6;
        assertEquals(EIpVersion.IPV6V4.getInt(), mApn.getIpVersion());

        mApn.mApnProtocol = ApnSetting.PROTOCOL_IP;
        assertEquals(EIpVersion.IPV4V6.getInt(), mApn.getIpVersion());

        mApn.mApnProtocol = ApnSetting.PROTOCOL_IPV6;
        assertEquals(EIpVersion.IPV6V4.getInt(), mApn.getIpVersion());

        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;
        assertEquals(EIpVersion.IPV6V4.getInt(), mApn.getIpVersion());
    }

    @Test
    public void testGetSlotId() throws Exception {
        assertEquals(SLOT0, mApn.getSlotId());
    }

    @Test
    public void testGetCachedNetwork_allCallbackNull() throws Exception {
        // return null when both NetworkCallback and NetworkMonitoringCallback is null
        assertNull(mApn.mNetworkCallback);
        assertNull(mApn.mNetworkMonitoringCallback);
        assertNull(mApn.getCachedNetwork());
    }

    @Test
    public void testGetCachedNetwork_nullNetworkCallback() throws Exception {
        // get from NetworkMonitoringCallback when NetworkCallback is null
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);
        when(mMockNetworkMonitoringCallback.getCachedNetwork()).thenReturn(mMockNetwork);
        assertEquals(mMockNetwork, mApn.getCachedNetwork());
    }

    @Test
    public void testGetCachedNetwork_notNullNetworkCallback() throws Exception {
        // get from NetworkCallback when NetworkCallback is not null
        replaceInstance(Apn.class, "mNetworkCallback", mApn, mMockNetworkCallback);
        when(mMockNetworkCallback.getCachedNetwork()).thenReturn(mMockNetwork);
        assertEquals(mMockNetwork, mApn.getCachedNetwork());
    }

    @Test
    public void testToString() throws Exception {
        String expectation = ", ApnType= " + mApn.mType
                + ", DataState= " + mApn.mDataState
                + ", APNState= " + mApn.mAPNState;

        assertEquals(expectation, mApn.toString());
    }

    @Test
    public void testRegisterCallback() throws Exception {
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();

        mApn.registerCallback(Apn.ImsNetworkCallback.EVENT_LOST);

        verify(mConnectivityManagerProxy).registerNetworkCallback(
                eq(nr), eq(mMockNetworkMonitoringCallback), eq(mApn));
        verify(mMockNetworkMonitoringCallback).setEvents(Apn.ImsNetworkCallback.EVENT_LOST);
        assertTrue(mApn.mIsMonitoringCallbackRegistered);
    }

    @Test
    public void testRegisterCallbackForXcap() throws Exception {
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);
        mApn.mType = EApnType.XCAP;

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP).build();

        mApn.registerCallback(Apn.ImsNetworkCallback.EVENT_LOST);

        verify(mConnectivityManagerProxy).registerNetworkCallback(
                eq(nr), eq(mMockNetworkMonitoringCallback), eq(mApn));
        verify(mMockNetworkMonitoringCallback).setEvents(Apn.ImsNetworkCallback.EVENT_LOST);
        assertTrue(mApn.mIsMonitoringCallbackRegistered);
    }

    @Test
    public void testRegisterCallbackForEmergency() throws Exception {
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);
        mApn.mType = EApnType.EMERGENCY;

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS).build();

        mApn.registerCallback(Apn.ImsNetworkCallback.EVENT_LOST);

        verify(mConnectivityManagerProxy).registerNetworkCallback(
                eq(nr), eq(mMockNetworkMonitoringCallback), eq(mApn));
        verify(mMockNetworkMonitoringCallback).setEvents(Apn.ImsNetworkCallback.EVENT_LOST);
        assertTrue(mApn.mIsMonitoringCallbackRegistered);
    }

    @Test
    public void testUnregisterCallback() throws Exception {
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);
        mApn.unregisterCallback();

        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkMonitoringCallback);
        assertFalse(mApn.mIsMonitoringCallbackRegistered);
    }

    @Test
    public void requestNetworkForImsTypeIncludesImsAndMmtelCapability() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApn, mMockNetworkCallback);
        mApn.mType = EApnType.IMS;

        mApn.requestNetwork();

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_MMTEL);
        NetworkRequest nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();
        verify(mConnectivityManagerProxy)
                .requestNetwork(eq(nr), eq(mMockNetworkCallback), eq(mApn));
    }

    @Test
    public void testRequestNetworkCapability() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApn, mMockNetworkCallback);

        mApn.mType = EApnType.EMERGENCY;
        NetworkRequest.Builder nrbEmergency = new NetworkRequest.Builder();
        nrbEmergency.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nrEmergency = nrbEmergency
                .addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS).build();
        mApn.requestNetwork();
        verify(mConnectivityManagerProxy)
                .requestNetwork(eq(nrEmergency), eq(mMockNetworkCallback), eq(mApn));

        mApn.mType = EApnType.XCAP;
        NetworkRequest.Builder nrbXcap = new NetworkRequest.Builder();
        nrbXcap.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nrXcap = nrbXcap
                .addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP).build();
        mApn.requestNetwork();
        verify(mConnectivityManagerProxy)
                .requestNetwork(eq(nrXcap), eq(mMockNetworkCallback), eq(mApn));

        mApn.mType = EApnType.INTERNET;
        NetworkRequest.Builder nrbInternet = new NetworkRequest.Builder();
        nrbInternet.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nrInternet = nrbInternet
                .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).build();
        mApn.requestNetwork();
        verify(mConnectivityManagerProxy)
                .requestNetwork(eq(nrInternet), eq(mMockNetworkCallback), eq(mApn));
    }

    @Test
    public void testReleaseNetwork() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApn, mMockNetworkCallback);

        mApn.releaseNetwork();
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        verify(mMockNetworkCallback).cleanUp();
    }

    @Test
    public void testRegisterHandler() throws Exception {
        int testEvent = 0;
        Message testMsg = Message.obtain();
        testMsg.what = testEvent;

        mApn.registerHandler(testEvent, mMockMsgProc);
        mApn.handleMessage(testMsg);
        verify(mMockMsgProc).procMsg(eq(testMsg));
    }

    @Test
    public void testApnRequestState() throws Exception {
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApn.getApnReqState());
        mApn.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApn.getApnReqState());
    }

    @Test
    public void testRegisterConfigListener() throws Exception {
        mApn.registerConfigListener();
        verify(mMockConfigInterface).addListener(any(ConfigInterface.Listener.class));
        mApn.unregisterConfigListener();
        verify(mMockConfigInterface).removeListener(any(ConfigInterface.Listener.class));
    }

    @Test
    public void testOnCarrierConfigChanged() throws Exception {
        replaceInstance(Apn.class, "mNetworkMonitoringCallback", mApn,
                mMockNetworkMonitoringCallback);
        when(mMockNetworkMonitoringCallback.getEvents())
                .thenReturn(Apn.ImsNetworkCallback.EVENT_LOST);
        mApn.mIsMonitoringCallbackRegistered = true;
        mApn.registerConfigListener();
        ArgumentCaptor<ConfigInterface.Listener> captor =
                ArgumentCaptor.forClass(ConfigInterface.Listener.class);
        verify(mMockConfigInterface).addListener(captor.capture());
        ConfigInterface.Listener configListener = captor.getValue();
        configListener.onCarrierConfigChanged(SLOT0, SUB_ID_1);

        // perform unregisterCallback()
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkMonitoringCallback);

        // perform registerCallback() again
        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        NetworkRequest nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();

        verify(mConnectivityManagerProxy).registerNetworkCallback(
                eq(nr), eq(mMockNetworkMonitoringCallback), eq(mApn));
        verify(mMockNetworkMonitoringCallback).setEvents(Apn.ImsNetworkCallback.EVENT_LOST);
        assertTrue(mApn.mIsMonitoringCallbackRegistered);
    }

    @Test
    public void testNotifyDataConnectionIpcanChanged_NotChanged() throws Exception {
        // IPCAN category is not changed
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
        assertFalse(mApn.notifyDataConnectionIpcanChanged(TelephonyManager.NETWORK_TYPE_LTE));
    }

    @Test
    public void testNotifyDataConnectionIpcanChanged_Changed() throws Exception {
        // IPCAN category is changed
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
        assertTrue(mApn.notifyDataConnectionIpcanChanged(TelephonyManager.NETWORK_TYPE_IWLAN));
        verify(mMockISystem).notifyDataConnectionIpcanChanged(
                mApn.mType.getType(), Apn.IPCAN_CATEGORY_WLAN);
        assertEquals(Apn.IPCAN_CATEGORY_WLAN, mApn.mIpcanCategory);
    }

    @Test
    public void testNotifyPdnConnectionFailed() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApn, mMockIDcNetWatcher);

        mApn.notifyPdnConnectionFailed(mApn.mType, 33);
        verify(mMockIDcNetWatcher).notifyPdnConnectionFailed(mApn.mType, 33);
    }

    @Test
    public void testHasLocalAddress() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApn, mMockIDcApn);
        when(mMockIDcApn.getLocalAddress(mApn.mType.getType(), eq(anyInt())))
                .thenReturn(null)
                .thenReturn("0.0.0.0");
        assertFalse(mApn.hasLocalAddress(eq(anyInt())));
        assertTrue(mApn.hasLocalAddress(eq(anyInt())));
    }

    @Test
    public void testIsIpChanged() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApn, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(mApn.mType.getType()))
                .thenReturn(null)
                .thenReturn("0.0.0.0")
                .thenReturn("0.0.0.0");
        when(mMockIDcApn.getLocalAddress(mApn.mType.getType(), eq(anyInt())))
                .thenReturn(null)
                .thenReturn("0.0.0.0")
                .thenReturn("1.1.1.1");

        // both cached ip and current ip is null
        assertTrue(mApn.isIpChanged());

        // cached ip and current ip is same
        assertFalse(mApn.isIpChanged());

        // cached ip and current ip is different
        assertTrue(mApn.isIpChanged());
    }

    @Test
    public void testUpdateDataState() throws Exception {
        // update to DATA_CONNECTED when mPreciseDcState is DATA_CONNECTED
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);

        // update to DATA_DISCONNECTED when mPreciseDcState is DATA_DISCONNECTED
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_DISCONNECTED;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);

        // update to DATA_CONNECTED when mPreciseDcState is DATA_SUSPENDED
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_SUSPENDED;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);

        // update to DATA_DISCONNECTED when mPreciseDcState is DATA_UNKNOWN
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_UNKNOWN;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);

        // update to DATA_CONNECTED when mPreciseDcState is DATA_DISCONNECTING
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_DISCONNECTING;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);

        // update to DATA_DISCONNECTED when mPreciseDcState is DATA_CONNECTING
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);

        // update to DATA_CONNECTED when mPreciseDcState is DATA_HANDOVER_IN_PROGRESS
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_HANDOVER_IN_PROGRESS;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);

        // Not update when mPreciseDcState is same with current data state
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApn.updateDataState();
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mDataState);
    }

    @Test
    public void testSendDataStateUpdateMessage() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApn, mMockIDcNetWatcher);

        mApn.sendDataStateUpdateMessage(mApn.mType, EDataState.DATA_STATE_CONNECTED);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher).notifyDataConnectionState(
                mApn.mType, EDataState.DATA_STATE_CONNECTED);
        verify(mMockISystem).notifyDataConnectionStateChanged(
                mApn.mType.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleIpcanCategory_notChanged() throws Exception {
        // IPCAN category is not changed
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
        assertFalse(mApn.handleIpcanCategory(TelephonyManager.NETWORK_TYPE_LTE));
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
    }

    @Test
    public void testHandleIpcanCategory_changed() throws Exception {
        // IPCAN category is changed
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
        assertTrue(mApn.handleIpcanCategory(TelephonyManager.NETWORK_TYPE_IWLAN));
        verify(mMockISystem).notifyDataConnectionIpcanChanged(
                mApn.mType.getType(), Apn.IPCAN_CATEGORY_WLAN);
        assertEquals(Apn.IPCAN_CATEGORY_WLAN, mApn.mIpcanCategory);
    }

    @Test
    public void testHandleIpcanCategory_notHandledApnType() throws Exception {
        // Do no handle IPCAN cagegory change when apn type is not ims or emergency
        mApn.mType = EApnType.INTERNET;
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
        assertFalse(mApn.handleIpcanCategory(TelephonyManager.NETWORK_TYPE_IWLAN));
        assertEquals(Apn.IPCAN_CATEGORY_MOBILE, mApn.mIpcanCategory);
    }

    @Test
    public void testHandleDataStateChanged_connected() throws Exception {
        // handle DATA_STATE_CONNECTED
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_NOTIFY_DATA_STATE_CHANGED;
        msg.arg1 = mApn.mType.getType();
        msg.arg2 = EDataState.DATA_STATE_CONNECTED.getState();
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionStateChanged(
                mApn.mType.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleDataStateChanged_connectFailed() throws Exception {
        // handle DATA_STATE_CONNECT_FAILED
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_NOTIFY_DATA_STATE_CHANGED;
        msg.arg1 = mApn.mType.getType();
        msg.arg2 = EDataState.DATA_STATE_CONNECT_FAILED.getState();
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionFailed(mApn.mType.getType());
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_ignoreNullState() throws Exception {
        // ignore if PreciseDataConnectionState is null
        assertEquals(TelephonyManager.DATA_UNKNOWN, mApn.mPreciseDcState);
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = null;
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        assertEquals(TelephonyManager.DATA_UNKNOWN, mApn.mPreciseDcState);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_ignoreUnknownNetworktype()
            throws Exception {
        // ignore if connected with unknown network type
        mApn.mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_CONNECTED,
                TelephonyManager.NETWORK_TYPE_UNKNOWN, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        assertNotEquals("TestApn", mApn.mApnString);
        assertNotEquals(ApnSetting.PROTOCOL_IPV6, mApn.mApnProtocol);
        assertNotEquals(TelephonyManager.NETWORK_TYPE_IWLAN, mApn.mNetworkType);
        assertNotEquals(TelephonyManager.DATA_CONNECTED, mApn.mPreciseDcState);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_handoverFail() throws Exception {
        mApn.addListener(mMockApnListener);

        // fail to handover
        mApn.mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
        mApn.mPreciseDcState = TelephonyManager.DATA_HANDOVER_IN_PROGRESS;
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_CONNECTED,
                TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.OPERATOR_BARRED);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener).onHandoverStateChanged(Apn.HANDOVER_FAILURE,
                TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.OPERATOR_BARRED);
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mApn.mNetworkType);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_handoverSucceed() throws Exception {
        mApn.addListener(mMockApnListener);

        // success to handover
        mApn.mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
        mApn.mPreciseDcState = TelephonyManager.DATA_HANDOVER_IN_PROGRESS;
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_CONNECTED,
                TelephonyManager.NETWORK_TYPE_IWLAN, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener).onHandoverStateChanged(Apn.HANDOVER_SUCCESS,
                TelephonyManager.NETWORK_TYPE_IWLAN, DataFailCause.NONE);
        verify(mMockISystem).notifyDataConnectionIpcanChanged(
                mApn.mType.getType(), Apn.IPCAN_CATEGORY_WLAN);
        verify(mMockApnListener).onIpcanCategoryChanged(
                mApn.mType.getType(), Apn.IPCAN_CATEGORY_WLAN);
        assertEquals(Apn.IPCAN_CATEGORY_WLAN, mApn.mIpcanCategory);
        assertEquals(TelephonyManager.NETWORK_TYPE_IWLAN, mApn.mNetworkType);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_initialConnect() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApn, mMockIDcNetWatcher);
        when(mMockIDcNetWatcher.isDataNetworkRoaming()).thenReturn(true);

        // initially connected
        mApn.mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_CONNECTED,
                TelephonyManager.NETWORK_TYPE_IWLAN, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        assertEquals("TestApn", mApn.mApnString);
        assertEquals(ApnSetting.PROTOCOL_IPV6, mApn.mApnProtocol);
        assertEquals(TelephonyManager.NETWORK_TYPE_IWLAN, mApn.mNetworkType);
        assertEquals(TelephonyManager.DATA_CONNECTED, mApn.mPreciseDcState);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_handoverInProgress() throws Exception {
        mApn.addListener(mMockApnListener);

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_HANDOVER_IN_PROGRESS,
                TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener).onHandoverStateChanged(
                Apn.HANDOVER_START, TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.NONE);
        assertEquals(TelephonyManager.DATA_HANDOVER_IN_PROGRESS, mApn.mPreciseDcState);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_handoverFromUnknownNetworktype()
            throws Exception {
        mApn.addListener(mMockApnListener);

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_HANDOVER_IN_PROGRESS,
                TelephonyManager.NETWORK_TYPE_UNKNOWN, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener).onHandoverStateChanged(
                Apn.HANDOVER_START, TelephonyManager.NETWORK_TYPE_UNKNOWN, DataFailCause.NONE);
        assertEquals(TelephonyManager.DATA_HANDOVER_IN_PROGRESS, mApn.mPreciseDcState);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_notifyConnectionStateChange() {
        mApn.addListener(mMockApnListener);

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_CONNECTING,
                TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener).onPreciseDataConnectionStateChanged(
                EApnType.IMS.getType(), TelephonyManager.DATA_CONNECTING);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_doNotNotifySameConnectionState() {
        mApn.addListener(mMockApnListener);
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_CONNECTING,
                TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.NONE);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener, never()).onPreciseDataConnectionStateChanged(
                EApnType.IMS.getType(), TelephonyManager.DATA_CONNECTING);
    }

    @Test
    public void testHandlePreciseDataConnectionStateChanged_disconnected() throws Exception {
        mApn.registerHandler(Apn.EVENT_DATA_CONNECTION_FAILED, mMockMsgProc);
        mApn.mNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
        mApn.mPreciseDcState = TelephonyManager.DATA_CONNECTING;

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
        msg.obj = getPreciseDataConnectionState(TelephonyManager.DATA_DISCONNECTED,
                TelephonyManager.NETWORK_TYPE_LTE, DataFailCause.OPERATOR_BARRED);
        mApn.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockMsgProc).procMsg(any(Message.class));
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mApn.mNetworkType);
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApn.mPreciseDcState);
    }

    @Test
    public void testImsNetworkCallback_create() throws Exception {
        Apn.ImsNetworkCallback callback1 = new Apn.ImsNetworkCallback(mApn.mType.getType(), mApn);
        assertNotNull(callback1);
        assertEquals(mApn.mType.getType(), callback1.getType());
        assertEquals(Apn.ImsNetworkCallback.EVENT_ALL, callback1.getEvents());

        Apn.ImsNetworkCallback callback2 = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);
        assertNotNull(callback2);
        assertEquals(mApn.mType.getType(), callback2.getType());
        assertEquals(Apn.ImsNetworkCallback.EVENT_LOST, callback2.getEvents());

        callback1.cleanUp();
        callback2.cleanUp();

        assertEquals(0, callback1.getEvents());
        assertEquals(0, callback2.getEvents());
        callback1 = null;
        callback2 = null;
    }

    @Test
    public void testImsNetworkCallback_onAvailable_withChangedNetwork() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_AVAILABLE, mMockMsgProc);
        mApn.registerHandler(Apn.EVENT_NETWORK_LOST, mMockMsgProc);
        LinkProperties linkProperties = new LinkProperties();
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(linkProperties);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);

        // first onAvailable is called and handle EVENT_NETWORK_AVAILABLE
        callback.onAvailable(mMockNetwork);

        // onAvailable is called with changed Network again
        Network mockNewNetwork = mock(Network.class);
        callback.onAvailable(mockNewNetwork);
        mTestableLooper.processAllMessages();

        // verify whether handle EVENT_NETWORK_LOST and EVENT_NETWORK_AVAILABLE
        verify(mMockMsgProc, times(3)).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onAvailable_ignore() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_AVAILABLE, mMockMsgProc);
        LinkProperties linkProperties = new LinkProperties();
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(linkProperties);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);

        // ignore if EVENT_AVAILABLE is not set
        callback.onAvailable(mMockNetwork);
        assertEquals(linkProperties, callback.mCachedLinkProperties);
        assertEquals(mMockNetwork, callback.getCachedNetwork());

        // check whether send EVENT_NETWORK_AVAILABLE
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onAvailable(mMockNetwork);
        mTestableLooper.processAllMessages();

        assertFalse(callback.mIsPendingOnAvailable);
        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onAvailablePending() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_AVAILABLE, mMockMsgProc);
        LinkProperties linkProperties = null;
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(linkProperties);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_ALL, mApn);

        // do not sent EVENT_NETWORK_AVAILABLE when LinkProperties is not updated yet
        callback.onAvailable(mMockNetwork);
        mTestableLooper.processAllMessages();

        assertTrue(callback.mIsPendingOnAvailable);
        verify(mMockMsgProc, never()).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onLosing() throws Exception {
        int maxMsToLive = 500;
        mApn.registerHandler(Apn.EVENT_NETWORK_LOSING, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);

        // ignore if EVENT_LOSING is not set
        callback.onLosing(mMockNetwork, maxMsToLive);

        // check whether send EVENT_NETWORK_LOSING
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onLosing(mMockNetwork, maxMsToLive);
        mTestableLooper.processAllMessages();

        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onLost() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_LOST, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_AVAILABLE, mApn);
        callback.mCachedLinkProperties = new LinkProperties();
        callback.mNetwork = mMockNetwork;
        assertNotNull(callback.mCachedLinkProperties);
        assertNotNull(callback.mNetwork);

        // ignore if EVENT_LOST is not set
        callback.onLost(mMockNetwork);
        assertEquals(null, callback.mCachedLinkProperties);
        assertEquals(null, callback.getCachedNetwork());

        // check whether send EVENT_NETWORK_LOST
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onLost(mMockNetwork);
        mTestableLooper.processAllMessages();

        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onUnavailable() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_UNAVAILABLE, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);
        callback.mCachedLinkProperties = new LinkProperties();
        assertNotNull(callback.mCachedLinkProperties);

        // ignore if EVENT_UNAVAILABLE is not set
        callback.onUnavailable();
        assertEquals(null, callback.mCachedLinkProperties);

        // check whether send EVENT_NETWORK_UNAVAILABLE
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onUnavailable();
        mTestableLooper.processAllMessages();

        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onCapabilitiesChanged() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_CAPABILITIES_CHANGED, mMockMsgProc);
        NetworkCapabilities networkCapabilities = new NetworkCapabilities();
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);

        // ignore if EVENT_CAPABILITIES_CHANGED is not set
        callback.onCapabilitiesChanged(mMockNetwork, networkCapabilities);

        // check whether send EVENT_NETWORK_CAPABILITIES_CHANGED
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onCapabilitiesChanged(mMockNetwork, networkCapabilities);
        mTestableLooper.processAllMessages();

        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onCapabilitiesChangedAfterPending() throws Exception {
        mApn.registerHandler(Apn.EVENT_NETWORK_AVAILABLE, mMockMsgProc);
        mApn.registerHandler(Apn.EVENT_NETWORK_CAPABILITIES_CHANGED, mMockMsgProc);
        NetworkCapabilities networkCapabilities = new NetworkCapabilities();
        LinkProperties linkProperties = new LinkProperties();
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(linkProperties);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);

        // check whether send EVENT_NETWORK_AVAILABLE and EVENT_NETWORK_CAPABILITIES_CHANGED
        callback.mIsPendingOnAvailable = true;
        callback.cacheLinkProperties(mMockNetwork);
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onCapabilitiesChanged(mMockNetwork, networkCapabilities);
        mTestableLooper.processAllMessages();

        verify(mMockMsgProc, times(2)).procMsg(any(Message.class));
        assertFalse(callback.mIsPendingOnAvailable);
    }

    @Test
    public void testImsNetworkCallback_onLinkPropertiesChangedWithFirstIp() throws Exception {
        mApn.registerHandler(Apn.EVENT_IP_CHANGED, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOCAL_IP_CHANGED, mApn);
        callback.mCachedLinkProperties = new LinkProperties();
        LinkProperties newLinkProperties = new LinkProperties();
        newLinkProperties.addLinkAddress(new LinkAddress("11.11.11.11/8"));
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(newLinkProperties);

        callback.onLinkPropertiesChanged(mMockNetwork, newLinkProperties);
        assertEquals(newLinkProperties, callback.mCachedLinkProperties);

        // check whether send EVENT_IP_CHANGED
        mTestableLooper.processAllMessages();
        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onLinkPropertiesChangedWithIpChange() throws Exception {
        mApn.registerHandler(Apn.EVENT_IP_CHANGED, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOCAL_IP_CHANGED, mApn);

        callback.mCachedLinkProperties = new LinkProperties();
        LinkProperties newLinkProperties = new LinkProperties();
        callback.mCachedLinkProperties.addLinkAddress(new LinkAddress("11.11.11.11/8"));
        newLinkProperties.addLinkAddress(new LinkAddress("22.22.22.22/8"));
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(newLinkProperties);

        callback.onLinkPropertiesChanged(mMockNetwork, newLinkProperties);
        assertEquals(newLinkProperties, callback.mCachedLinkProperties);

        // check whether send EVENT_IP_CHANGED
        mTestableLooper.processAllMessages();
        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onLinkPropertiesChangedWithFirstPcscf() throws Exception {
        mApn.registerHandler(Apn.EVENT_PCSCF_CHANGED, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_NET_PCSCF_CHANGED, mApn);

        callback.mCachedLinkProperties = new LinkProperties();
        LinkProperties newLinkProperties = new LinkProperties();
        newLinkProperties.addPcscfServer(InetAddresses.parseNumericAddress("11.11.11.11"));
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(newLinkProperties);
        callback.onLinkPropertiesChanged(mMockNetwork, newLinkProperties);
        assertEquals(newLinkProperties, callback.mCachedLinkProperties);

        // check whether send EVENT_PCSCF_CHANGED
        mTestableLooper.processAllMessages();
        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onLinkPropertiesChangedWithPcscfChange() throws Exception {
        mApn.registerHandler(Apn.EVENT_PCSCF_CHANGED, mMockMsgProc);
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_NET_PCSCF_CHANGED, mApn);

        callback.mCachedLinkProperties = new LinkProperties();
        LinkProperties newLinkProperties = new LinkProperties();
        callback.mCachedLinkProperties.addPcscfServer(
                InetAddresses.parseNumericAddress("11.11.11.11"));
        newLinkProperties.addPcscfServer(InetAddresses.parseNumericAddress("22.22.22.22"));
        when(mConnectivityManagerProxy.getLinkProperties(any(Network.class)))
                .thenReturn(newLinkProperties);
        callback.onLinkPropertiesChanged(mMockNetwork, newLinkProperties);
        assertEquals(newLinkProperties, callback.mCachedLinkProperties);

        // check whether send EVENT_PCSCF_CHANGED
        mTestableLooper.processAllMessages();
        verify(mMockMsgProc).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallback_onLinkPropertiesChangedWithoutChange() throws Exception {
        mApn.registerHandler(Apn.EVENT_IP_CHANGED, mMockMsgProc);
        mApn.registerHandler(Apn.EVENT_PCSCF_CHANGED, mMockMsgProc);
        LinkProperties linkProperties = new LinkProperties();
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOST, mApn);

        // ignore if EVENT_LOCAL_IP_CHANGED and EVENT_NET_PCSCF_CHANGED is not set
        callback.onLinkPropertiesChanged(mMockNetwork, linkProperties);

        // check whether not send EVENT_IP_CHANGED or EVENT_PCSCF_CHANGED
        callback.setEvents(Apn.ImsNetworkCallback.EVENT_ALL);
        callback.onLinkPropertiesChanged(mMockNetwork, linkProperties);
        mTestableLooper.processAllMessages();
        verify(mMockMsgProc, never()).procMsg(any(Message.class));
    }

    @Test
    public void testImsNetworkCallbackIsIpChanged() throws Exception {
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_LOCAL_IP_CHANGED, mApn);
        callback.mCachedLinkProperties = new LinkProperties();
        LinkProperties newLinkProperties = new LinkProperties();
        LinkAddress ip4LinkAddress = new LinkAddress("11.11.11.11/8");
        LinkAddress ip6LinkAddress = new LinkAddress("2001:4860:4860::8888/64");
        LinkAddress ip6Linklocal = new LinkAddress("fe80::1/64");

        // both mCachedLinkProperties and newLinkProperties is empty
        assertFalse(callback.isIpChanged(newLinkProperties));

        // mCachedLinkProperties is not empty but newLinkProperties is empty
        callback.mCachedLinkProperties.addLinkAddress(ip4LinkAddress);
        assertFalse(callback.isIpChanged(newLinkProperties));

        // same number of addresses but address is different
        newLinkProperties.addLinkAddress(ip6LinkAddress);
        newLinkProperties.addLinkAddress(ip6Linklocal);
        assertTrue(callback.isIpChanged(newLinkProperties));

        // number of addresses is different
        newLinkProperties.addLinkAddress(ip4LinkAddress);
        newLinkProperties.addLinkAddress(ip6Linklocal);
        assertTrue(callback.isIpChanged(newLinkProperties));
    }

    @Test
    public void testImsNetworkCallbackIsPcscfChanged() throws Exception {
        Apn.ImsNetworkCallback callback = new Apn.ImsNetworkCallback(
                mApn.mType.getType(), Apn.ImsNetworkCallback.EVENT_NET_PCSCF_CHANGED, mApn);
        callback.mCachedLinkProperties = new LinkProperties();
        LinkProperties newLinkProperties = new LinkProperties();
        InetAddress ip4Pcscf = InetAddresses.parseNumericAddress("22.22.22.22");
        InetAddress ip6Pcscf = InetAddresses.parseNumericAddress("2001:db8::abcd:1234");
        InetAddress linklocal = InetAddresses.parseNumericAddress("fe80::1");

        // mCachedLinkProperties is not empty but newLinkProperties is empty
        callback.mCachedLinkProperties.addPcscfServer(ip4Pcscf);
        assertFalse(callback.isPcscfChanged(newLinkProperties));

        // number of PCSCF addresses is different
        newLinkProperties.addPcscfServer(ip4Pcscf);
        newLinkProperties.addPcscfServer(ip6Pcscf);
        newLinkProperties.addPcscfServer(linklocal);
        assertTrue(callback.isPcscfChanged(newLinkProperties));

        // Apn Type is not IMS
        Apn.ImsNetworkCallback callbackXcap = new Apn.ImsNetworkCallback(
                EApnType.XCAP.getType(), Apn.ImsNetworkCallback.EVENT_NET_PCSCF_CHANGED, mApn);
        assertFalse(callbackXcap.isPcscfChanged(newLinkProperties));
    }

    private static class FakeApn extends Apn {
        FakeApn(Context context, int slotId) {
            super(context, slotId, EApnType.IMS);
        }
    }

    private PreciseDataConnectionState getPreciseDataConnectionState(
            int state, int networkType, int causeCode) {
        mPreciseDataConnectionState = new PreciseDataConnectionState.Builder()
                .setState(state)
                .setNetworkType(networkType)
                .setApnSetting(new ApnSetting.Builder()
                        .setApnTypeBitmask(ApnSetting.TYPE_IMS)
                        .setApnName("TestApn")
                        .setEntryName("Test")
                        .setProtocol(ApnSetting.PROTOCOL_IPV6)
                        .setRoamingProtocol(ApnSetting.PROTOCOL_IPV6)
                        .build())
                .setFailCause(causeCode)
                .build();

        assertNotNull(mPreciseDataConnectionState);

        return mPreciseDataConnectionState;
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
