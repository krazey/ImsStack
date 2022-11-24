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
import android.os.Message;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.enabler.aos.IAosInfo;
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
public class ApnImsTest {
    private static final int SLOT_0 = 0;
    static ContextFixture sContext;
    ApnIms mApnIms;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcSettings mMockIDcSettings;
    @Mock private IAosInfo mMockIAosInfo;
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
        mApnIms = new ApnIms(AppContext.getInstance(), SLOT_0);
        mConnectivityManager = AppContext.getInstance().getSystemService(ConnectivityManager.class);

        mTestableLooper = TestableLooper.get(ApnImsTest.this);
        mTestableLooper.processAllMessages();

        replaceInstance(Apn.class, "mSystem", mApnIms, mMockISystem);
    }

    @After
    public void tearDown() throws Exception {
        if (mApnIms != null) {
            mApnIms.cleanup();
            mApnIms = null;
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
        // do not handle request to connect because ApnIms is not enabled
        assertFalse(mApnIms.connect());

        // handle request to connect if ApnIms is enabled
        mApnIms.employApn();
        assertTrue(mApnIms.connect());
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_CONNECTING, mApnIms.getDataState());
        verify(mConnectivityManager, times(1)).requestNetwork(
                any(NetworkRequest.class), any(ConnectivityManager.NetworkCallback.class));

        // return true without request to connect because request is already done
        assertTrue(mApnIms.connect());
    }

    @Test
    public void testDisconnect() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnIms, mMockNetworkCallback);

        // do not handle request to disconnect because apn has never been requested to connect
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        assertFalse(mApnIms.disconnect());

        // handle request to disconnect if request to connect is done
        mApnIms.employApn();
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        assertTrue(mApnIms.disconnect());
        verify(mConnectivityManager, times(1)).unregisterNetworkCallback(mMockNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnIms.getDataState());
        mTestableLooper.processAllMessages();
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testGetApn() throws Exception {
        assertEquals(EApnType.IMS.getString(), mApnIms.getApn());
    }

    @Test
    public void testNotifyHandoverInfoChanged() throws Exception {
        int failureCause = 33;
        replaceInstance(ApnIms.class, "mAosInfo", mApnIms, mMockIAosInfo);
        mApnIms.notifyHandoverInfoChanged(
                IApn.HANDOVER_FAILURE, TelephonyManager.NETWORK_TYPE_IWLAN, failureCause);

        verify(mMockIAosInfo, times(1)).notifyIpcanHandoverFailure(
                IApn.IPCAN_CATEGORY_MOBILE, failureCause);
    }

    @Test
    public void testHandleNetworkAvailable() throws Exception {
        // if apn is not requested, ignore event
        mApnIms.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());

        // if apn has been requested before, notify data connection state change
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnIms.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        assertEquals(TelephonyManager.DATA_CONNECTED, mApnIms.getDataState());
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleNetworkLost() throws Exception {
        // if data state is not DATA_CONNECTED, ignore event
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnIms.getDataState());
        mApnIms.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        // if data state is DATA_CONNECTED, notify data connection state change
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnIms.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnIms.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnIms.getDataState());
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testHandleIpChanged_differentIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnIms, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(EApnType.IMS.getType())).thenReturn("1.2.3.4");
        when(mMockIDcApn.getLocalAddress(EApnType.IMS.getType(), 0)).thenReturn("0.0.0.0");

        // if data state is not connected, ignore event
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnIms.getDataState());
        mApnIms.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        // if data state is connected and receive different IP address, notify changed IP address
        mApnIms.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnIms.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandleIpChanged_sameIp() throws Exception {
        replaceInstance(Apn.class, "mDcApn", mApnIms, mMockIDcApn);
        when(mMockIDcApn.getCachedLocalAddress(EApnType.IMS.getType())).thenReturn("0.0.0.0");
        when(mMockIDcApn.getLocalAddress(EApnType.IMS.getType(), 0)).thenReturn("0.0.0.0");

        // if data state is connected but receive same IP address, ignore event
        mApnIms.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnIms.sendEmptyMessage(Apn.EVENT_IP_CHANGED);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(0)).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandlePcscfChanged() throws Exception {
        mApnIms.sendEmptyMessage(Apn.EVENT_PCSCF_CHANGED);
        mTestableLooper.processAllMessages();
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_PCSCF_CHANGED.getState());
    }

    @Test
    public void testHandleDataConnectionFailed() throws Exception {
        int failureCause = 33;
        replaceInstance(Apn.class, "mDcSettings", mApnIms, mMockIDcSettings);
        when(mMockIDcSettings.isPermanentFailure(EApnType.IMS, failureCause))
                .thenReturn(false)
                .thenReturn(true);

        // if apn is not requested, ignore event
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        mApnIms.sendEmptyMessage(Apn.EVENT_DATA_CONNECTION_FAILED);
        mTestableLooper.processAllMessages();

        // if apn has been requested before, notify data connection state change
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnIms.setDataState(TelephonyManager.DATA_CONNECTING);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = null;
        mApnIms.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg2.obj = failureCause;
        mApnIms.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        Message msg3 = Message.obtain();
        msg3.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg3.obj = failureCause;
        mApnIms.sendMessage(msg3);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, times(1)).notifyDataConnectionFailed(EApnType.IMS.getType());
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
