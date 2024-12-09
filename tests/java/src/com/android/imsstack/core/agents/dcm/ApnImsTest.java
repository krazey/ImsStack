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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
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
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ImsTrafficInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.enabler.aos.IAosInfo;
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
public class ApnImsTest {
    ApnIms mApnIms;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private IApn.Listener mMockApnListener;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcSettings mMockIDcSettings;
    @Mock private IAosInfo mMockIAosInfo;
    @Mock private ISystem mMockISystem;
    @Mock private ImsTrafficInterface mMockImsTrafficInterface;
    @Mock private SubsInfoInterface mMockSubsInfoInterface;
    @Mock private RuntimeException mMockRuntimeException;

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

        mApnIms = new ApnIms(mContextFixture.getTestDouble(), SLOT0);
        mTestableLooper = TestableLooper.get(ApnImsTest.this);
        mTestableLooper.processAllMessages();
        replaceInstance(Apn.class, "mSystem", mApnIms, mMockISystem);
        replaceInstance(Apn.class, "mDcSettings", mApnIms, mMockIDcSettings);
    }

    @After
    public void tearDown() throws Exception {
        if (mApnIms != null) {
            mApnIms.cleanup();
            mApnIms = null;
        }

        TestableLooper.remove(ApnImsTest.this);
        mTestableLooper = null;
        mConnectivityManagerProxy = null;
        mContextFixture = null;
        DeviceConfig.setSimCount(1, 1);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testConnect() throws Exception {
        assertTrue(mApnIms.connect());
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_CONNECTING, mApnIms.getDataState());
        verify(mConnectivityManagerProxy).requestNetwork(any(NetworkRequest.class),
                any(ConnectivityManager.NetworkCallback.class), any(Handler.class));

        // return true without request to connect because request is already done
        assertTrue(mApnIms.connect());
    }

    @Test
    public void testConnect_imsOff() throws Exception {
        AgentFactory.getInstance()
                .setAgent(SubsInfoInterface.class, mMockSubsInfoInterface, SLOT0);

        // If IMS is not enabled, do not handle connect request and return false
        when(mMockSubsInfoInterface.isImsEnabled()).thenReturn(false);
        assertFalse(mApnIms.connect());
        verify(mConnectivityManagerProxy, never())
                .requestNetwork(any(NetworkRequest.class),
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));

        AgentFactory.getInstance().setAgent(SubsInfoInterface.class, null, SLOT0);
    }

    @Test
    public void testDisconnect() throws Exception {
        replaceInstance(Apn.class, "mNetworkCallback", mApnIms, mMockNetworkCallback);

        // do not handle request to disconnect because apn has never been requested to connect
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        assertFalse(mApnIms.disconnect());

        // handle request to disconnect if request to connect is done
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnIms.mDataState = TelephonyManager.DATA_CONNECTED;

        assertTrue(mApnIms.disconnect());
        verify(mConnectivityManagerProxy).unregisterNetworkCallback(mMockNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnIms.getDataState());
        mTestableLooper.processAllMessages();
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    @Test
    public void testGetApn() throws Exception {
        // when the mApnString is null
        assertEquals(EApnType.IMS.getString(), mApnIms.getApn());

        // when the mApnString is not null
        String newApnString = "NEWAPN";
        mApnIms.mApnString = newApnString;
        assertEquals(newApnString, mApnIms.getApn());
    }

    @Test
    public void testHandleCarrierConfigChanged() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        DeviceConfig.setSimCount(2, 2);

        mApnIms.registerSystemDefaultNetworkCallback();

        // do not handle the event for other slot
        mApnIms.handleCarrierConfigChanged(SLOT0 + 1, 1);
        // only handle the event for my slot
        mApnIms.handleCarrierConfigChanged(SLOT0, 1);

        verify(mConnectivityManagerProxy)
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
        verify(mConnectivityManagerProxy, times(2))
                .registerSystemDefaultNetworkCallback(
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));
    }

    @Test
    public void doNotNotifyCrossSimStatusWhenNotChanged() throws Exception {
        mApnIms.addListener(mMockApnListener);
        mApnIms.mIsCellularDefaultNetwork = false;
        mApnIms.mIsConnectedOverCrossSim = false;

        mApnIms.updateCrossSimStatus(TelephonyManager.NETWORK_TYPE_IWLAN);

        verify(mMockApnListener, never()).onCrossSimStatusChanged(anyBoolean());
        mApnIms.removeListener(mMockApnListener);
    }

    @Test
    public void notifyCrossSimStatusWhenChanged() throws Exception {
        mApnIms.addListener(mMockApnListener);
        mApnIms.mIsCellularDefaultNetwork = true;
        mApnIms.mIsConnectedOverCrossSim = true;

        mApnIms.updateCrossSimStatus(TelephonyManager.NETWORK_TYPE_LTE);

        verify(mMockApnListener).onCrossSimStatusChanged(false);
        mApnIms.removeListener(mMockApnListener);
    }

    @Test
    public void testRegisterDefaultNetworkCallback_SingleSim() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);

        mApnIms.registerSystemDefaultNetworkCallback();
        verify(mConnectivityManagerProxy, never())
                .registerSystemDefaultNetworkCallback(
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));
    }

    @Test
    public void testRegisterDefaultNetworkCallback_crossSimDisabled() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(false);
        DeviceConfig.setSimCount(2, 2);

        mApnIms.registerSystemDefaultNetworkCallback();
        verify(mConnectivityManagerProxy, never())
                .registerSystemDefaultNetworkCallback(
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));
    }

    @Test
    public void testRegisterDefaultNetworkCallback_alreadyRegistered() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        DeviceConfig.setSimCount(2, 2);

        // do not invoke registerSystemDefaultNetworkCallback if it has registered networkCallback
        // once
        mApnIms.registerSystemDefaultNetworkCallback();
        mApnIms.registerSystemDefaultNetworkCallback();
        verify(mConnectivityManagerProxy)
                .registerSystemDefaultNetworkCallback(
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));
    }

    @Test
    public void testRegisterDefaultNetworkCallback_runtimeException() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        DeviceConfig.setSimCount(2, 2);
        doThrow(mMockRuntimeException)
                .when(mConnectivityManagerProxy)
                .registerSystemDefaultNetworkCallback(
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));

        mApnIms.registerSystemDefaultNetworkCallback();
        verify(mMockRuntimeException).getMessage();
    }

    @Test
    public void testUnregisterSystemDefaultNetworkCallback() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        DeviceConfig.setSimCount(2, 2);

        // do not invoke unregisterNetworkCallback() if callback has not been registered
        mApnIms.unregisterSystemDefaultNetworkCallback();
        verify(mConnectivityManagerProxy, never())
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));

        mApnIms.registerSystemDefaultNetworkCallback();
        verify(mConnectivityManagerProxy)
                .registerSystemDefaultNetworkCallback(
                        any(ConnectivityManager.NetworkCallback.class), any(Handler.class));

        mApnIms.unregisterSystemDefaultNetworkCallback();
        verify(mConnectivityManagerProxy)
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testUnRegisterDefaultNetworkCallback_runtimeException() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        DeviceConfig.setSimCount(2, 2);
        mApnIms.registerSystemDefaultNetworkCallback();
        doThrow(mMockRuntimeException).when(mConnectivityManagerProxy)
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));

        mApnIms.unregisterSystemDefaultNetworkCallback();
        verify(mMockRuntimeException).getMessage();
    }

    @Test
    public void testHandleIpcanCategory() throws Exception {
        AgentFactory.getInstance().setAgent(ImsTrafficInterface.class, mMockImsTrafficInterface);
        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;

        assertTrue(mApnIms.handleIpcanCategory(TelephonyManager.NETWORK_TYPE_IWLAN));

        assertEquals(Apn.IPCAN_CATEGORY_WLAN, mApnIms.mIpcanCategory);
        verify(mMockISystem).notifyDataConnectionIpcanChanged(
                mApnIms.mType.getType(), Apn.IPCAN_CATEGORY_WLAN);
        verify(mMockImsTrafficInterface).setWlan(true, SLOT0);

        AgentFactory.getInstance().setAgent(ImsTrafficInterface.class, null);
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
        verify(mMockISystem).notifyDataConnectionStateChanged(
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
        verify(mMockISystem).notifyDataConnectionStateChanged(
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

        verify(mMockISystem).notifyDataConnectionStateChanged(
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

        verify(mMockISystem, never()).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_IP_CHANGED.getState());
    }

    @Test
    public void testHandlePcscfChanged() throws Exception {
        mApnIms.sendEmptyMessage(Apn.EVENT_PCSCF_CHANGED);
        mTestableLooper.processAllMessages();
        verify(mMockISystem).notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), EDataState.DATA_STATE_PCSCF_CHANGED.getState());
    }

    @Test
    public void testHandleDataConnectionFailed() throws Exception {
        int failureCause = 33;
        when(mMockIDcSettings.isPermanentFailure(EApnType.IMS, failureCause))
                .thenReturn(false)
                .thenReturn(true);

        // if apn has been requested before, notify data connection state change
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        // not permanent failure
        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = failureCause;
        mApnIms.sendMessage(msg1);

        // permanent failure
        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg2.obj = failureCause;
        mApnIms.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mMockISystem).notifyDataConnectionFailed(EApnType.IMS.getType());
    }

    @Test
    public void testHandleDataConnectionFailed_invalidCase() throws Exception {
        int failureCause = 33;
        when(mMockIDcSettings.isPermanentFailure(EApnType.IMS, failureCause))
                .thenReturn(true);

        // if apn is not requested, ignore event
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_IDLE);
        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg1.obj = failureCause;
        mApnIms.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        // if msg.obj is null, ignore event
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_DATA_CONNECTION_FAILED;
        msg2.obj = null;
        mApnIms.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mMockISystem, never()).notifyDataConnectionFailed(EApnType.IMS.getType());
    }

    @Test
    public void doNotHandleInvalidMsgForDefaultNetworkStatusChange() throws Exception {
        mApnIms.addListener(mMockApnListener);
        mApnIms.mNetworkType = TelephonyManager.NETWORK_TYPE_IWLAN;

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED;
        msg.obj = null;
        mApnIms.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener, never()).onCrossSimStatusChanged(anyBoolean());
        mApnIms.removeListener(mMockApnListener);
    }

    @Test
    public void handleMsgForDefaultNetworkStatusChange() throws Exception {
        mApnIms.addListener(mMockApnListener);
        mApnIms.mNetworkType = TelephonyManager.NETWORK_TYPE_IWLAN;

        Message msg = Message.obtain();
        msg.what = Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED;
        msg.obj = true;
        mApnIms.sendMessage(msg);
        mTestableLooper.processAllMessages();

        verify(mMockApnListener).onCrossSimStatusChanged(true);
        mApnIms.removeListener(mMockApnListener);
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
