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
import static org.mockito.Matchers.anyInt;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.ConnectivityManager;
import android.net.NetworkRequest;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ApnEmergencyTest {
    private static final int SLOT_0 = 0;
    static ContextFixture sContext;
    ApnEmergency mApnEmergency;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private DcApn mMockDcApn;
    @Mock private DcUtils mMockDcUtils;
    @Mock private ISystem mMockISystem;

    private TestableLooper mTestableLooper;
    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private ConnectivityManager mConnectivityManager;

    @BeforeClass
    public static void initial() {
        sContext = new ContextFixture();
        try {
            AppContext.init(sContext.getTestDouble());
        } catch (java.lang.IllegalThreadStateException e) {
            // AppContext thread is already started by others.
        }
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        // create the instance to test
        mApnEmergency = new ApnEmergency(AppContext.get(), SLOT_0);
        mTelephonyManager = AppContext.get().getSystemService(TelephonyManager.class);
        mSubscriptionManager = AppContext.get().getSystemService(SubscriptionManager.class);
        mConnectivityManager = AppContext.get().getSystemService(ConnectivityManager.class);

        mTestableLooper = TestableLooper.get(ApnEmergencyTest.this);
        mTestableLooper.processAllMessages();

        replaceInstance(Apn.class, "mSystem", mApnEmergency, mMockISystem);
    }

    @After
    public void tearDown() throws Exception {
        if (mApnEmergency != null) {
            mApnEmergency.cleanup();
            mApnEmergency = null;
        }
        mTestableLooper = null;
    }

    @Test
    public void testConnect() throws Exception {
        // ApnEmergency is not enabled.
        assertFalse(mApnEmergency.connect());

        // enable ApnEmergency
        mApnEmergency.employApn();
        assertTrue(mApnEmergency.connect());
        verify(mConnectivityManager).requestNetwork(
                any(NetworkRequest.class), any(ConnectivityManager.NetworkCallback.class));
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_DONE);
        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_CONNECTING);

        // request is already done
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_DONE);
        assertTrue(mApnEmergency.connect());
    }

    @Test
    public void testDisconnect() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        when(mMockNetworkCallback.isNetworkConnected()).thenReturn(true);

        // apn has never been requested to connect
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_IDLE);
        assertFalse(mApnEmergency.disconnect());

        // disconnect apn after apn request is done
        mApnEmergency.employApn();
        mApnEmergency.setAPNReqState(EApnReqState.APN_REQUEST_DONE);

        assertTrue(mApnEmergency.disconnect());
        verify(mConnectivityManager).unregisterNetworkCallback(
                any(ConnectivityManager.NetworkCallback.class));
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_IDLE);
        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_DISCONNECTED);
        mTestableLooper.processAllMessages();
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testGetApn() throws Exception {
        assertEquals(mApnEmergency.getApn(), EApnType.EMERGENCY.getString());
    }

    @Test
    public void testHandleNetworkAvailable() throws Exception {
        replaceInstance(ApnEmergency.class, "mDcUtils", mApnEmergency, mMockDcUtils);
        when(mSubscriptionManager.getSubscriptionIds(anyInt())).thenReturn(null);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getDataNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_IWLAN);

        // receive EVENT_NETWORK_AVAILABLE when apn is not requested
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_IDLE);
        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_DISCONNECTED);
        assertEquals(mApnEmergency.mNetworkType, TelephonyManager.NETWORK_TYPE_UNKNOWN);

        // receive EVENT_NETWORK_AVAILABLE after that apn is not requested
        mApnEmergency.setAPNReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        verify(mMockDcUtils).updateAllCellInfoForcinglyOnLimitedServiceState();
        assertEquals(mApnEmergency.mNetworkType, TelephonyManager.NETWORK_TYPE_IWLAN);
        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_CONNECTED);
        verify(mMockISystem).notifyDataConnectionIpcanChanged(
                EApnType.EMERGENCY.getType(), IApn.IPCAN_CATEGORY_WLAN);
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleNetworkLost() throws Exception {
        // receive EVENT_NETWORK_LOST when apn is not requested
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_IDLE);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        // receive EVENT_NETWORK_AVAILABLE after that apn is not requested
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.setAPNReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_DISCONNECTED);
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testHandleIpChanged_differentIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnEmergency, mMockDcApn);
        when(mMockDcApn.getCachedLocalAddress(EApnType.EMERGENCY.getType())).thenReturn("1.2.3.4");
        when(mMockDcApn.getLocalAddress(EApnType.EMERGENCY.getType(), 0)).thenReturn("0.0.0.0");

        // receive EVENT_IP_CHANGED when data state is not connected
        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_DISCONNECTED);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        // receive EVENT_IP_CHANGED after that data state is connected with different IP address
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleIpChanged_sameIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnEmergency, mMockDcApn);
        when(mMockDcApn.getCachedLocalAddress(EApnType.EMERGENCY.getType())).thenReturn("0.0.0.0");
        when(mMockDcApn.getLocalAddress(EApnType.EMERGENCY.getType(), 0)).thenReturn("0.0.0.0");

        // receive EVENT_IP_CHANGED after that data state is connected with same IP address
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(0)).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleDataConnectionFailed() throws Exception {
        // receive EVENT_DATA_CONNECTION_FAILED when apn is not requested
        assertEquals(mApnEmergency.getAPNReqState(), EApnReqState.APN_REQUEST_IDLE);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_DATA_CONNECTION_FAILED);
        mTestableLooper.processAllMessages();

        // receive EVENT_DATA_CONNECTION_FAILED after that apn is not requested
        mApnEmergency.setAPNReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTING);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_DATA_CONNECTION_FAILED);
        mTestableLooper.processAllMessages();

        assertEquals(mApnEmergency.getDataState(), TelephonyManager.DATA_DISCONNECTED);
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
                                                final Object obj, final Object newValue)
            throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }

}
