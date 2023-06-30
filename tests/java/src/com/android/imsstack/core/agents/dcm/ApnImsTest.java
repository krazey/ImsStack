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
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkRequest;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.util.AppContext;

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
    private static final int SLOT_0 = 0;
    ApnIms mApnIms;

    @Mock private Apn.ImsNetworkCallback mMockNetworkCallback;
    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private IDcSettings mMockIDcSettings;
    @Mock private IAosInfo mMockIAosInfo;
    @Mock private IAosRegistration mMockIAosReg;
    @Mock private ISystem mMockISystem;

    private Context mContext;
    private TestableLooper mTestableLooper;
    private ConnectivityManager mConnectivityManager;
    private TelephonyManager mTelephonyManager;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContext = new ContextFixture().getTestDouble();
        AppContext.init(mContext);

        // create the instance to test
        mApnIms = new ApnIms(AppContext.getInstance(), SLOT_0);
        mConnectivityManager = AppContext.getInstance().getSystemService(ConnectivityManager.class);
        mTelephonyManager = AppContext.getInstance().getSystemService(TelephonyManager.class);

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
        mConnectivityManager = null;
        mTelephonyManager = null;

        AppContext.deinit();
        mContext = null;
    }

    @Test
    public void testConnect() throws Exception {
        assertTrue(mApnIms.connect());
        assertEquals(EApnReqState.APN_REQUEST_DONE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_CONNECTING, mApnIms.getDataState());
        verify(mConnectivityManager).requestNetwork(
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
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        assertTrue(mApnIms.disconnect());
        verify(mConnectivityManager).unregisterNetworkCallback(mMockNetworkCallback);
        assertEquals(EApnReqState.APN_REQUEST_IDLE, mApnIms.getApnReqState());
        assertEquals(TelephonyManager.DATA_DISCONNECTED, mApnIms.getDataState());
        mTestableLooper.processAllMessages();
        verify(mMockISystem).notifyDataConnectionStateChanged(
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

        verify(mMockIAosInfo).notifyIpcanHandoverFailure(
                IApn.IPCAN_CATEGORY_MOBILE, failureCause);
    }

    @Test
    public void testHandleCarrierConfigChanged() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(2);

        mApnIms.registerDefaultNetworkCallback();

        // do not handle the event for other slot
        mApnIms.handleCarrierConfigChanged(SLOT_0 + 1, 1);
        // only handle the event for my slot
        mApnIms.handleCarrierConfigChanged(SLOT_0, 1);

        verify(mConnectivityManager)
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
        verify(mConnectivityManager, times(2))
                .registerDefaultNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testUpdateCrossSimStatus() throws Exception {
        replaceInstance(ApnIms.class, "mAosInfo", mApnIms, mMockIAosInfo);
        mApnIms.mNetworkType = TelephonyManager.NETWORK_TYPE_IWLAN;

        // do not notify CrossSim status to AosInfo because default network is not available
        mApnIms.updateCrossSimStatus(TelephonyManager.NETWORK_TYPE_IWLAN);
        verify(mMockIAosInfo, never()).notifyCrossSimStatus(anyBoolean());

        // notify CrossSim status as true to AosInfo when default network is available
        Message msg = Message.obtain();
        msg.what = Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED;
        msg.obj = true;
        mApnIms.sendMessage(msg);
        mTestableLooper.processAllMessages();
        verify(mMockIAosInfo).notifyCrossSimStatus(true);

        // notify CrossSim status as false to AosInfo because it does not connected over IWLAN
        mApnIms.updateCrossSimStatus(TelephonyManager.NETWORK_TYPE_LTE);
        verify(mMockIAosInfo).notifyCrossSimStatus(false);
    }

    @Test
    public void testRegisterDefaultNetworkCallback_SingleSim() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(1);

        mApnIms.registerDefaultNetworkCallback();
        verify(mConnectivityManager, never())
                .registerDefaultNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testRegisterDefaultNetworkCallback_CrossSimDisabled() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(false);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(2);

        mApnIms.registerDefaultNetworkCallback();
        verify(mConnectivityManager, never())
                .registerDefaultNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testRegisterDefaultNetworkCallback_AlreadyRegistered() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(2);

        // do not invoke registerDefaultNetworkCallback if it has registered networkCallback once
        mApnIms.registerDefaultNetworkCallback();
        mApnIms.registerDefaultNetworkCallback();
        verify(mConnectivityManager)
                .registerDefaultNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testUnregisterDefaultNetworkCallback() throws Exception {
        when(mMockIDcSettings.isCrossSimEnabledByPlatform()).thenReturn(true);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(2);

        // do not invoke unregisterNetworkCallback() if callback has not been registered
        mApnIms.unregisterDefaultNetworkCallback();
        verify(mConnectivityManager, never())
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));

        mApnIms.registerDefaultNetworkCallback();
        verify(mConnectivityManager)
                .registerDefaultNetworkCallback(any(ConnectivityManager.NetworkCallback.class));

        mApnIms.unregisterDefaultNetworkCallback();
        verify(mConnectivityManager)
                .unregisterNetworkCallback(any(ConnectivityManager.NetworkCallback.class));
    }

    @Test
    public void testEvaluateImsNetworkCapability() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApnIms, mMockIDcNetWatcher);
        replaceInstance(Apn.class, "mAosReg", mApnIms, mMockIAosReg);
        when(mMockIDcNetWatcher.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN)
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE)
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE)
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        mApnIms.mIsMmtelRequired = true;
        mApnIms.mImsPdnRequestWithoutMmtel = true;
        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        // do not change network capability when network is not registered
        mApnIms.evaluateImsNetworkCapability();

        // do not change network capability when PDN has not been requested before
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_IDLE);
        mApnIms.evaluateImsNetworkCapability();
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);

        // do not change network capability when network is connected through the IWLAN
        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_WLAN;
        mApnIms.evaluateImsNetworkCapability();
        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;

        // verify whether it request PDN capability change
        mApnIms.setApnReqState(EApnReqState.APN_REQUEST_DONE);
        mApnIms.evaluateImsNetworkCapability();
        verify(mMockIAosReg).controlRegistration(IAosRegistration.RequestType.STOP,
                IAosRegistration.Pcscf.CURRENT, IAosRegistration.Cause.PDN_CAPABILITY_CHANGED);
    }

    @Test
    public void testIsMmtelCapabilityRequired() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApnIms, mMockIDcNetWatcher);

        // KEY_REQUEST_IMS_PDN_WITHOUT_MMTEL_BOOL is true
        mApnIms.mImsPdnRequestWithoutMmtel = true;
        assertFalse(mApnIms.isMmtelCapabilityRequired());

        // Network is in scenario of KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
        mApnIms.mImsPdnRequestWithoutMmtel = false;
        when(mMockIDcNetWatcher.isRoaming())
                .thenReturn(true)
                .thenReturn(false);

        mApnIms.mNoVopsRequired = mApnIms.ROAMING_NETWORK;
        assertFalse(mApnIms.isMmtelCapabilityRequired());

        mApnIms.mNoVopsRequired = mApnIms.HOME_NETWORK;
        assertFalse(mApnIms.isMmtelCapabilityRequired());

        // Network is not in scenario of KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
        when(mMockIDcNetWatcher.isRoaming())
                .thenReturn(false)
                .thenReturn(true);

        mApnIms.mNoVopsRequired = mApnIms.ROAMING_NETWORK;
        assertTrue(mApnIms.isMmtelCapabilityRequired());

        mApnIms.mNoVopsRequired = mApnIms.HOME_NETWORK;
        assertTrue(mApnIms.isMmtelCapabilityRequired());

        // Exception - DcNetWather is null
        replaceInstance(Apn.class, "mDcNetWatcher", mApnIms, null);
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);
        mApnIms.mImsPdnRequestWithoutMmtel = false;
        mApnIms.mNoVopsRequired = mApnIms.ROAMING_NETWORK;
        assertTrue(mApnIms.isMmtelCapabilityRequired());
    }

    @Test
    public void testHandleIpcanCategory() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApnIms, mMockIDcNetWatcher);

        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;
        assertTrue(mApnIms.handleIpcanCategory(TelephonyManager.NETWORK_TYPE_IWLAN));
        assertEquals(Apn.IPCAN_CATEGORY_WLAN, mApnIms.mIpcanCategory);
        verify(mMockISystem, times(1)).notifyDataConnectionIpcanChanged(
                mApnIms.mType.getType(), Apn.IPCAN_CATEGORY_WLAN);
        verify(mMockIDcNetWatcher, times(1)).getNetworkType();
    }

    @Test
    public void testUpdateCarrierConfig() throws Exception {
        int[] configNoVopsRequired = {CarrierConfigManager.Ims.NETWORK_TYPE_ROAMING};
        when(mMockIDcSettings.isImsPdnRequestWithoutMmtel()).thenReturn(false);
        when(mMockIDcSettings.getImsPdnEnabledInNoVopsSupport()).thenReturn(configNoVopsRequired);
        mApnIms.mImsPdnRequestWithoutMmtel = true;
        mApnIms.mNoVopsRequired = mApnIms.HOME_NETWORK;

        assertTrue(mApnIms.updateCarrierConfig());
        assertEquals(false, mApnIms.mImsPdnRequestWithoutMmtel);
        assertEquals(mApnIms.ROAMING_NETWORK, mApnIms.mNoVopsRequired);

        replaceInstance(Apn.class, "mDcSettings", mApnIms, null);
        assertFalse(mApnIms.updateCarrierConfig());
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
    public void testHandleIpChanged_DifferentIp() throws Exception {
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
    public void testHandleIpChanged_SameIp() throws Exception {
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

        verify(mMockISystem).notifyDataConnectionFailed(EApnType.IMS.getType());
    }

    @Test
    public void testHandleDefaultNetworkStatusChanged() throws Exception {
        replaceInstance(ApnIms.class, "mAosInfo", mApnIms, mMockIAosInfo);
        mApnIms.mNetworkType = TelephonyManager.NETWORK_TYPE_IWLAN;

        // do not handle invalid msg.obj
        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED;
        msg1.obj = null;
        mApnIms.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED;
        msg2.obj = true;
        mApnIms.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mMockIAosInfo).notifyCrossSimStatus(true);
    }

    @Test
    public void testHandleRoamingStateChanged() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApnIms, mMockIDcNetWatcher);
        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;

        // do not handle invalid msg.obj
        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_ROAMING_STATE_CHANGED;
        msg1.obj = null;
        mApnIms.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_ROAMING_STATE_CHANGED;
        msg2.obj = true;
        mApnIms.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher, times(1)).getNetworkType();
    }

    @Test
    public void testHandleVopsSupportChanged() throws Exception {
        replaceInstance(Apn.class, "mDcNetWatcher", mApnIms, mMockIDcNetWatcher);
        mApnIms.mIpcanCategory = Apn.IPCAN_CATEGORY_MOBILE;

        // do not handle invalid msg.obj
        Message msg1 = Message.obtain();
        msg1.what = Apn.EVENT_VOPS_SUPPORT_CHANGED;
        msg1.obj = null;
        mApnIms.sendMessage(msg1);
        mTestableLooper.processAllMessages();

        Message msg2 = Message.obtain();
        msg2.what = Apn.EVENT_VOPS_SUPPORT_CHANGED;
        msg2.obj = false;
        mApnIms.sendMessage(msg2);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher, times(1)).getNetworkType();
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
