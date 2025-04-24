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

import static android.telephony.DataFailCause.ERROR_UNSPECIFIED;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.ConnectivityManager;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.system.ISystem;

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
public class ApnEmergencyTest {
    private static final int SLOT_0 = 0;
    static ContextFixture sContext;
    ApnEmergency mApnEmergency;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private IDcSettings mMockIDcSettings;
    @Mock private IDcUtils mMockIDcUtils;
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

        DcFactory.setDcAgent(IDcNetWatcher.class, mMockIDcNetWatcher, SLOT_0);

        // create the instance to test
        mApnEmergency = new ApnEmergency(AppContext.getInstance(), SLOT_0);
        mConnectivityManager = AppContext.getInstance().getSystemService(ConnectivityManager.class);

        mTestableLooper = TestableLooper.get(ApnEmergencyTest.this);
        mTestableLooper.processAllMessages();

        replaceInstance(Apn.class, "mSystem", mApnEmergency, mMockISystem);
        replaceInstance(Apn.class, "mDcSettings", mApnEmergency, mMockIDcSettings);
    }

    @After
    public void tearDown() throws Exception {
        if (mApnEmergency != null) {
            mApnEmergency.cleanup();
            mApnEmergency = null;
        }
        mTestableLooper = null;

        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT_0);
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    public void testConnect() throws Exception {
        assertTrue(mApnEmergency.connect());
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApnEmergency.getApnReqState());
        assertEquals(TelephonyManager.DATA_CONNECTING, mApnEmergency.getDataState());
        verify(mConnectivityManager).requestNetwork(
                any(NetworkRequest.class), any(ConnectivityManager.NetworkCallback.class),
                any(Handler.class));

        // return true without request to connect because request is already done
        assertTrue(mApnEmergency.connect());
    }

    @Test
    public void testDisconnect() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);

        // do not handle request to disconnect because apn has never been requested to connect
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnEmergency.getApnReqState());
        assertFalse(mApnEmergency.disconnect());

        // handle request to disconnect if request to connect is done
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.mPreciseDcState = TelephonyManager.DATA_CONNECTED;

        assertTrue(mApnEmergency.disconnect());
        verify(mConnectivityManager).unregisterNetworkCallback(mMockNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnEmergency.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnEmergency.getDataState());
        mTestableLooper.processAllMessages();
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testGetApn() throws Exception {
        // when the mApnString is null
        assertEquals(EApnType.EMERGENCY.getString(), mApnEmergency.getApn());

        // when the mApnString is not null
        String newApnString = "NEWAPN";
        mApnEmergency.mApnString = newApnString;
        assertEquals(newApnString, mApnEmergency.getApn());
    }

    @Test
    public void testHandleNetworkAvailable() throws Exception {
        replaceInstance(ApnEmergency.class, "mDcUtils", mApnEmergency, mMockIDcUtils);

        // if apn is not requested, ignore event
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnEmergency.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnEmergency.getDataState());
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mApnEmergency.mNetworkType);

        // if apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        verify(mMockIDcUtils).updateAllCellInfoForcinglyOnLimitedServiceState();
        assertEquals(TelephonyManager.DATA_CONNECTED, mApnEmergency.getDataState());
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleNetworkLost() throws Exception {
        // if apn is not requested, ignore event
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnEmergency.getApnReqState());
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        // if apn has been requested before, notify data connection state change
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnEmergency.getDataState());
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testHandleIpChanged_differentIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnEmergency, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(EApnType.EMERGENCY.getType())).thenReturn("1.2.3.4");
        when(mMockIDcApn.getLocalAddress(EApnType.EMERGENCY.getType(), 0)).thenReturn("0.0.0.0");

        // if data state is not connected, ignore event
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnEmergency.getDataState());
        mApnEmergency.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        // if data state is connected and receive different IP address, notify changed IP address
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleIpChanged_sameIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnEmergency, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(EApnType.EMERGENCY.getType())).thenReturn("0.0.0.0");
        when(mMockIDcApn.getLocalAddress(EApnType.EMERGENCY.getType(), 0)).thenReturn("0.0.0.0");

        // if data state is connected but receive same IP address, ignore event
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, never()).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleDataConnectionFailed() throws Exception {
        // only when the apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = 33;
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testHandleDataConnectionFailed_crossStackRedialCause() throws Exception {
        int failureCause = 33;
        when(mMockIDcSettings.isCrossStackRedialCause(EApnType.EMERGENCY, failureCause))
                .thenReturn(true);
        // only when the apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = failureCause;
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionFailed(EApnType.EMERGENCY.getType());
    }

    @Test
    public void testHandleDataConnectionFailed_crossStackRedialCauseWithEmmCause()
            throws Exception {
        int failureCause = 5; // EMM cause IMEI not accepted.
        when(mMockIDcNetWatcher.getNetworkRegistrationRejectCause()).thenReturn(failureCause);
        when(mMockIDcSettings.isCrossStackRedialCause(EApnType.EMERGENCY, failureCause))
                .thenReturn(true);
        // only when the apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = ERROR_UNSPECIFIED;
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionFailed(EApnType.EMERGENCY.getType());
    }

    @Test
    public void testHandleDataConnectionFailed_invalidCase() throws Exception {
        // if apn is not requested, ignore event
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_IDLE);
        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = 33;
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        // if msg.obj is null, ignore event
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg2.obj = null;
        mApnEmergency.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, never()).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
