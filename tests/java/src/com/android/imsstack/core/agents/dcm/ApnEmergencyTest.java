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
import static android.telephony.DataFailCause.SERVICE_OPTION_NOT_SUBSCRIBED;

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
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.system.ISystem;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ApnEmergencyTest {
    private static final int SLOT_0 = 0;
    ApnEmergency mApnEmergency;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private IDcSettings mMockIDcSettings;
    @Mock private IDcUtils mMockIDcUtils;
    @Mock private ISystem mMockISystem;
    @Mock private Call mMockCall;

    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private ConnectivityManagerProxy mConnectivityManagerProxy;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mTestAppContext = new TestAppContext(mContextFixture.getTestDouble());
        mTestAppContext.setUp();

        mConnectivityManagerProxy =
                mTestAppContext.getSystemServiceProxy(ConnectivityManagerProxy.class);
        DcFactory.setDcAgent(IDcNetWatcher.class, mMockIDcNetWatcher, SLOT_0);

        mApnEmergency = new ApnEmergency(mContextFixture.getTestDouble(), SLOT_0);
        mTestableLooper = TestableLooper.get(ApnEmergencyTest.this);
        mTestableLooper.processAllMessages();

        replaceInstance(Apn.class, "mSystem", mApnEmergency, mMockISystem);
        replaceInstance(Apn.class, "mDcSettings", mApnEmergency, mMockIDcSettings);
    }

    @After
    public void tearDown() throws Exception {
        TestableLooper.remove(ApnEmergencyTest.this);
        mTestableLooper = null;
        if (mApnEmergency != null) {
            mApnEmergency.cleanup();
            mApnEmergency = null;
        }

        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT_0);
        mConnectivityManagerProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mContextFixture = null;
    }

    @Test
    public void testConnect() throws Exception {
        assertTrue(mApnEmergency.connect());
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApnEmergency.getApnReqState());
        assertEquals(TelephonyManager.DATA_CONNECTING, mApnEmergency.getDataState());
        verify(mMockIDcNetWatcher).clearNetworkRegistrationRejectCause();
        verify(mConnectivityManagerProxy).requestNetwork(
                any(NetworkRequest.class), any(ConnectivityManager.NetworkCallback.class),
                any(Handler.class));

        // return true without request to connect because request is already done
        assertTrue(mApnEmergency.connect());
    }

    @Test
    public void testConnect_shouldCancelPendingDisconnect() {
        // GIVEN
        mApnEmergency.mIsWaitForTransportChange = true;

        // WHEN
        mApnEmergency.connect();

        // THEN
        assertFalse(mApnEmergency.mIsWaitForTransportChange);
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
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnEmergency.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnEmergency.getDataState());
        mTestableLooper.processAllMessages();
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testDisconnect_pendDuringTransportChange() throws Exception {
        // GIVEN
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.mIsWaitForTransportChange = true;

        // WHEN
        assertTrue(mApnEmergency.disconnect());

        // THEN
        verify(mConnectivityManagerProxy, never()).unregisterNetworkCallback(mMockNetworkCallback);
        assertTrue(mApnEmergency.hasMessages(Apn.EVENT_DELAYED_DISCONNECT));
    }

    @Test
    public void testDisconnect_shouldNotPendIfTransportDoesNotChange() throws Exception {
        // GIVEN
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.mIsWaitForTransportChange = false;

        // WHEN
        assertTrue(mApnEmergency.disconnect());

        // THEN
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        assertFalse(mApnEmergency.hasMessages(Apn.EVENT_DELAYED_DISCONNECT));
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
    public void testOnCallCreated_shouldSendCallCreatedEvent() {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(true);
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false)).thenReturn(true);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallCreated(mMockCall);

        // THEN
        assertTrue(mApnEmergency.hasMessages(Apn.EVENT_CALL_CREATED));
    }

    @Test
    public void testOnCallCreated_normalCall_shouldNotSendCallCreatedEvent() {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(false);

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallCreated(mMockCall);

        // THEN
        assertFalse(mApnEmergency.hasMessages(Apn.EVENT_CALL_CREATED));
    }

    @Test
    public void testOnCallCreated_disconnectedState_shouldNotSendCallCreatedEvent() {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(true);
        mApnEmergency.setDataState(TelephonyManager.DATA_DISCONNECTED);

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallCreated(mMockCall);

        // THEN
        assertFalse(mApnEmergency.hasMessages(Apn.EVENT_CALL_CREATED));
    }

    @Test
    public void testOnCallDestroyed_shouldSendCallDestroyedEvent() throws Exception {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(true);

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallDestroyed(mMockCall);

        // THEN
        assertTrue(mApnEmergency.hasMessages(Apn.EVENT_CALL_DESTROYED));
    }

    @Test
    public void testOnCallCreated_normalCall_shouldNotSendCallDestroyedEvent() {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(false);

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallDestroyed(mMockCall);

        // THEN
        assertFalse(mApnEmergency.hasMessages(Apn.EVENT_CALL_DESTROYED));
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
    public void testHandleNetworkAvailable_shouldCancelPendingDisconnect() throws Exception {
        // GIVEN
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.mIsWaitForTransportChange = true;

        // WHEN
        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        // THEN
        assertFalse(mApnEmergency.mIsWaitForTransportChange);
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
    public void testHandleNetworkLost_releaseNetworkIfEcbmNotSupported() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        when(mMockIDcSettings.isEmergencyCallbackModeSupported()).thenReturn(false);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
    }

    @Test
    public void testHandleNetworkLost_doNotReleaseNetworkIfEcbmSupported() throws Exception {
        when(mMockIDcSettings.isEmergencyCallbackModeSupported()).thenReturn(true);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        verify(mConnectivityManagerProxy, never()).unregisterNetworkCallback(
                any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testHandleNetworkLost_doNotReleaseNetworkIfEcbmSupportUnknown() throws Exception {
        replaceInstance(Apn.class, "mDcSettings", mApnEmergency, null);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        mApnEmergency.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        verify(mConnectivityManagerProxy, never()).unregisterNetworkCallback(
                any(ConnectivityManager.NetworkCallback.class));
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
    public void testHandleDataConnectionFailed_crossStackRedialCauseWithEsmCause()
            throws Exception {
        int esmFailureCause = SERVICE_OPTION_NOT_SUBSCRIBED;
        when(mMockIDcSettings.isCrossStackRedialCause(EApnType.EMERGENCY, esmFailureCause))
                .thenReturn(true);
        // only when the apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = esmFailureCause;
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionFailed(EApnType.EMERGENCY.getType());
    }

    @Test
    public void testHandleDataConnectionFailed_crossStackRedialCauseWithEmmCause()
            throws Exception {
        int emmFailureCause = 5; // EMM cause IMEI not accepted.
        when(mMockIDcNetWatcher.getNetworkRegistrationRejectCause()).thenReturn(emmFailureCause);
        when(mMockIDcSettings.isCrossStackRedialCause(EApnType.EMERGENCY, emmFailureCause))
                .thenReturn(true);
        // only when the apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = ERROR_UNSPECIFIED; // Set UNSPECIFIED since it's failed by EMM.
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionFailed(EApnType.EMERGENCY.getType());
    }

    @Test
    public void testHandleDataConnectionFailed_crossStackRedialCauseWithEmmCauseButEsmCauseSpecified()
            throws Exception {
        int emmFailureCause = 5; // EMM cause IMEI not accepted.
        when(mMockIDcNetWatcher.getNetworkRegistrationRejectCause()).thenReturn(emmFailureCause);
        when(mMockIDcSettings.isCrossStackRedialCause(EApnType.EMERGENCY, emmFailureCause))
                .thenReturn(true);
        // only when the apn has been requested before, notify data connection state change
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = SERVICE_OPTION_NOT_SUBSCRIBED; // Set NOT_SUBSCRIBED since it's failed by ESM.
        mApnEmergency.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.EMERGENCY.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
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

    @Test
    public void testHandleDelayedDisconnect() throws Exception {
        // GIVEN
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.mIsWaitForTransportChange = true;

        // WHEN
        mApnEmergency.sendEmptyMessage(Apn.EVENT_DELAYED_DISCONNECT);
        mTestableLooper.processAllMessages();

        // THEN
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        assertFalse(mApnEmergency.mIsWaitForTransportChange);
    }

    @Test
    public void testHandleCallCreated_shouldMarkTransportChange() {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(true);
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false)).thenReturn(false);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.mIpcanCategory = Apn.IPCAN_CATEGORY_WLAN;

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallCreated(mMockCall);
        mTestableLooper.processAllMessages();

        // THEN
        assertTrue(mApnEmergency.mIsWaitForTransportChange);
    }

    @Test
    public void testHandleCallCreated_connectedToSameAsSelected_shouldNotMarkTransportChange() {
        // GIVEN
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(true);
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false)).thenReturn(true);
        mApnEmergency.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnEmergency.mIpcanCategory = Apn.IPCAN_CATEGORY_WLAN;

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallCreated(mMockCall);
        mTestableLooper.processAllMessages();

        // THEN
        assertFalse(mApnEmergency.mIsWaitForTransportChange);
    }

    @Test
    public void testHandleCallCreated_invalidMessage_shouldNotMarkTransportChange() {
        // WHEN
        mApnEmergency.sendMessage(mApnEmergency.obtainMessage(Apn.EVENT_CALL_CREATED, null));
        mTestableLooper.processAllMessages();

        // THEN
        assertFalse(mApnEmergency.mIsWaitForTransportChange);
    }

    @Test
    public void testHandleCallDestroyed_shouldCancelPendingDisconnect() throws Exception {
        // GIVEN
        replaceInstance(Apn.class, "mNetworkCallback", mApnEmergency, mMockNetworkCallback);
        when(mMockCall.getCallExtraBoolean(Call.EXTRA_E_CALL, false)).thenReturn(true);
        mApnEmergency.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnEmergency.mIsWaitForTransportChange = true;
        mApnEmergency.sendEmptyMessageDelayed(
                Apn.EVENT_DELAYED_DISCONNECT, ApnEmergency.DISCONNECT_DELAY_TIME);

        // WHEN
        mApnEmergency.mMtcCallRegistryListener.onCallDestroyed(mMockCall);
        mTestableLooper.processAllMessages();

        // THEN
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        assertFalse(mApnEmergency.mIsWaitForTransportChange);
    }
    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
