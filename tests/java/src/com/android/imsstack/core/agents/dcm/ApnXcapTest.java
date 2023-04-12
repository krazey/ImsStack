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
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.ConnectivityManager;
import android.net.NetworkRequest;
import android.os.AsyncResult;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ApnXcapTest {
    private static final int SLOT_0 = 0;
    static ContextFixture sContext;
    ApnXcap mApnXcap;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private ISystem mMockISystem;

    private TestableLooper mTestableLooper;
    private ConnectivityManager mConnectivityManager;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        // create the instance to test
        mApnXcap = new ApnXcap(AppContext.getInstance(), SLOT_0);
        mConnectivityManager = AppContext.getInstance().getSystemService(ConnectivityManager.class);

        mTestableLooper = TestableLooper.get(ApnXcapTest.this);
        mTestableLooper.processAllMessages();

        replaceInstance(Apn.class, "mDcNetWatcher", mApnXcap, mMockIDcNetWatcher);
        replaceInstance(Apn.class, "mSystem", mApnXcap, mMockISystem);
    }

    @After
    public void tearDown() throws Exception {
        if (mApnXcap != null) {
            mApnXcap.cleanup();
            mApnXcap = null;
        }
        mTestableLooper = null;
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    public void testConnect() throws Exception {
        assertTrue(mApnXcap.connect());
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApnXcap.getApnReqState());
        assertEquals(TelephonyManager.DATA_CONNECTING, mApnXcap.getDataState());
        verify(mConnectivityManager, times(1)).requestNetwork(
                any(NetworkRequest.class), any(ConnectivityManager.NetworkCallback.class));

        // return true without request to connect because request is already done
        assertTrue(mApnXcap.connect());
    }

    @Test
    public void testDisconnect() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnXcap, mMockNetworkCallback);

        // do not handle request to disconnect because apn has never been requested to connect
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnXcap.getApnReqState());
        assertFalse(mApnXcap.disconnect());

        // handle request to disconnect if request to connect is done
        mApnXcap.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnXcap.sendEmptyMessageDelayed(Apn.EVENT_WAITING_IPV6_ADDRESS,
                mApnXcap.OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL);

        assertTrue(mApnXcap.disconnect());
        verify(mConnectivityManager, times(1)).unregisterNetworkCallback(mMockNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnXcap.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnXcap.getDataState());
        assertFalse(mApnXcap.hasMessages(Apn.EVENT_WAITING_IPV6_ADDRESS));

        mTestableLooper.processAllMessages();
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testHandleNetworkAvailable() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnXcap, mMockIDcApn);
        when(mMockIDcApn.getLocalAddress(EApnType.XCAP.getType(), 1)).thenReturn(null);

        // if apn is not requested, ignore event
        mApnXcap.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnXcap.getApnReqState());

        // wait for obtaining IPv6 address
        mApnXcap.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnXcap.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        // if it already has EVENT_WAITING_IPV6_ADDRESS, ignore event
        mApnXcap.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        // if apn has been requested before and no need to wait IPv6 address,
        // notify data connection state change
        if (mApnXcap.hasMessages(Apn.EVENT_WAITING_IPV6_ADDRESS)) {
            mApnXcap.removeMessages(Apn.EVENT_WAITING_IPV6_ADDRESS);
        }
        mApnXcap.mApnProtocol = ApnSetting.PROTOCOL_IP;
        mApnXcap.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        assertEquals(TelephonyManager.DATA_CONNECTED, mApnXcap.getDataState());
        verify(mMockIDcNetWatcher, times(1)).notifyResult(
                EApnType.XCAP, EDataState.DATA_STATE_CONNECTED);
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleNetworkLost() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnXcap, mMockNetworkCallback);
        mApnXcap.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        // if data state is already DATA_DISCONNECTED, ignore event
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnXcap.getDataState());
        mApnXcap.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        // if data state is not DATA_DISCONNECTED, notify data connection state change
        mApnXcap.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnXcap.sendEmptyMessageDelayed(Apn.EVENT_WAITING_IPV6_ADDRESS,
                mApnXcap.OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL);
        mApnXcap.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        assertFalse(mApnXcap.hasMessages(Apn.EVENT_WAITING_IPV6_ADDRESS));
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnXcap.getDataState());
        verify(mMockIDcNetWatcher, times(2)).notifyResult(
                EApnType.XCAP, EDataState.DATA_STATE_DISCONNECTED);
        verify(mMockISystem, times(2)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
        verify(mConnectivityManager, times(1)).unregisterNetworkCallback(mMockNetworkCallback);
    }

    @Test
    public void testHandleIpChanged_notConnected() throws Exception {
        // if data state is not connected and do not wait IPv6 address, ignore event
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnXcap.getDataState());
        mApnXcap.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        // if data state is not connected and wait IPv6 address, update data state
        mApnXcap.sendEmptyMessageDelayed(Apn.EVENT_WAITING_IPV6_ADDRESS,
                mApnXcap.OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL);
        mApnXcap.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApnXcap.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        assertFalse(mApnXcap.hasMessages(Apn.EVENT_WAITING_IPV6_ADDRESS));
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleIpChanged_differentIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnXcap, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(EApnType.XCAP.getType())).thenReturn("1.2.3.4");
        when(mMockIDcApn.getLocalAddress(EApnType.XCAP.getType(), 0)).thenReturn("0.0.0.0");

        // if data state is connected and receive different IP address, notify changed IP address
        mApnXcap.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnXcap.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher, times(1)).notifyResult(
                EApnType.XCAP, EDataState.DATA_STATE_IP_CHANGED);
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleIpChanged_sameIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnXcap, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(EApnType.XCAP.getType())).thenReturn("0.0.0.0");
        when(mMockIDcApn.getLocalAddress(EApnType.XCAP.getType(), 0)).thenReturn("0.0.0.0");

        // if data state is connected but receive same IP address, ignore event
        mApnXcap.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnXcap.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(0)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleWaitingIpv6Address() throws Exception {
        // if updated data state is DATA_DISCONNECTED, ignore event
        mApnXcap.mPreciseDcState = TelephonyManager.DATA_DISCONNECTED;
        mApnXcap.sendEmptyMessage(Apn.EVENT_WAITING_IPV6_ADDRESS);
        mTestableLooper.processAllMessages();

        // if updated data state is DATA_CONNECTED, notify data connection state change
        mApnXcap.mPreciseDcState = TelephonyManager.DATA_CONNECTED;
        mApnXcap.sendEmptyMessage(Apn.EVENT_WAITING_IPV6_ADDRESS);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.XCAP.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testAirplaneModeChanged() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnXcap, mMockNetworkCallback);
        mApnXcap.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_AIRPLANE_MODE_CHANGED;
        AsyncResult ar = new AsyncResult(null, true, null);
        msg.obj = ar;
        mApnXcap.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mConnectivityManager, times(1)).unregisterNetworkCallback(mMockNetworkCallback);
    }

    @Test
    public void testAirplaneModeChanged_invalid() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnXcap, mMockNetworkCallback);
        mApnXcap.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_AIRPLANE_MODE_CHANGED;
        msg1.obj = null;
        mApnXcap.sendMessage(msg1);

        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_AIRPLANE_MODE_CHANGED;
        AsyncResult ar = new AsyncResult(null, null, null);
        msg2.obj = ar;
        mApnXcap.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mConnectivityManager, times(0)).unregisterNetworkCallback(mMockNetworkCallback);
    }

    @Test
    public void testHandleDataConnectionFailed() throws Exception {
        int failureCause = 33;

        // if apn is not requested, ignore event
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnXcap.getApnReqState());
        mApnXcap.sendEmptyMessage(Apn.EVENT_DATA_CONNECTION_FAILED);
        mTestableLooper.processAllMessages();

        // if apn has been requested before, notify data connection state change
        mApnXcap.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnXcap.setDataState(TelephonyManager.DATA_CONNECTING);

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg.obj = failureCause;
        mApnXcap.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher, times(1)).notifyPdnConnectionFailed(EApnType.XCAP, failureCause);
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
