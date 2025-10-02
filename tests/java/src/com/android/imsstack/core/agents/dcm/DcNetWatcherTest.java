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

import static android.provider.Settings.Global.AIRPLANE_MODE_ON;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.core.agents.dcmif.IDcNetWatcher.REGISTRATION_REJECT_CAUSE_NONE;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.CellIdentity;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.DataSpecificRegistrationInfo;
import android.telephony.LteVopsSupportInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.ContentProviderProxy.SettingsProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.ImsPhoneStateListener;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.agents.internal.PhoneStateNotifier;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Collections;
import java.util.List;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class DcNetWatcherTest extends ImsStackTest {
    private TestAppContext mTestAppContext;
    private DcNetWatcher mDcNetWatcher;

    @Mock SettingsProxy mSettingsProxy;
    @Mock DcSettings mMockDcSetting;
    @Mock ISystem mMockSystem;
    @Mock PhoneStateInterface mMockPhoneStateInterface;
    @Mock PhoneStateNotifier mMockPhoneStateNotifier;
    @Mock TelephonyInterface mMockTelephonyInterface;
    @Mock ConfigInterface mMockConfigInterface;
    @Mock SystemInterface mMockSystemInterface;
    @Mock NativeStateInterface mMockNativeStateInterface;
    @Mock IDcNetWatcher.Listener mNetWatherListener;

    @Before
    public void setup() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        when(mTestAppContext.getContentProviderProxy().getGlobalSettings())
                .thenReturn(mSettingsProxy);

        when(mMockSystemInterface.getSystem(SLOT0)).thenReturn(mMockSystem);
        replaceInstance(SystemInterface.class, "sSystemInterface", null, mMockSystemInterface);

        AgentFactory.getInstance().setAgent(
                PhoneStateInterface.class, mMockPhoneStateInterface, SLOT0);
        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mMockNativeStateInterface, SLOT0);
        AgentFactory.getInstance().setAgent(
                TelephonyInterface.class, mMockTelephonyInterface, SLOT0);
        AgentFactory.getInstance().setAgent(
                ConfigInterface.class, mMockConfigInterface, SLOT0);

        when(mMockPhoneStateInterface.createNotifier(any(), any(Looper.class)))
                .thenReturn(mMockPhoneStateNotifier);
        when(mMockDcSetting.getImsSupportedAccessNetworks())
                .thenReturn(new int[] {AccessNetworkConstants.AccessNetworkType.EUTRAN,
                        AccessNetworkConstants.AccessNetworkType.IWLAN,
                        AccessNetworkConstants.AccessNetworkType.UTRAN,
                        AccessNetworkConstants.AccessNetworkType.GERAN});
        DcFactory.setDcAgent(IDcSettings.class, mMockDcSetting, SLOT0);

        mDcNetWatcher = new DcNetWatcher(SLOT0);
        mDcNetWatcher.init(mContext);
        mDcNetWatcher.addListener(mNetWatherListener);
    }

    @After
    public void tearDown() throws Exception {
        mDcNetWatcher.removeListener(mNetWatherListener);
        mDcNetWatcher.cleanup();

        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, null, SLOT0);
        DcFactory.setDcAgent(IDcSettings.class, null, SLOT0);

        mSettingsProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;

        super.tearDown();
    }

    @Test
    public void testInit() {
        verify(mMockDcSetting).getImsSupportedAccessNetworks();
        verify(mMockSystemInterface).getSystem(SLOT0);
        verify(mMockNativeStateInterface).addListener(any(NativeStateInterface.Listener.class));
        verify(mTestAppContext.getBroadcastReceiverProxy()).registerReceiver(any(), any());
        verify(mMockPhoneStateInterface).createNotifier(any(), any(Looper.class));
        verify(mMockPhoneStateNotifier).setEvents(ImsPhoneStateListener.LISTEN_CALL_STATE
                | ImsPhoneStateListener.LISTEN_SERVICE_STATE
                | ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE);
        verify(mMockPhoneStateInterface).addNotifier(mMockPhoneStateNotifier);
        verify(mMockConfigInterface).addListener(any(ConfigInterface.Listener.class));
    }

    @Test
    public void testCleanup() {
        mDcNetWatcher.cleanup();

        verify(mMockConfigInterface).removeListener(any(ConfigInterface.Listener.class));
        verify(mMockPhoneStateInterface).removeNotifier(mMockPhoneStateNotifier);
        verify(mMockPhoneStateNotifier).setListener(null);
        verify(mTestAppContext.getBroadcastReceiverProxy()).unregisterReceiver(any());
        verify(mMockNativeStateInterface).removeListener(any(NativeStateInterface.Listener.class));
    }

    @Test
    public void testUpdateTelephonyNetworkType() {
        mDcNetWatcher.updateTelephonyNetworkType(TelephonyManager.NETWORK_TYPE_LTE);

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mDcNetWatcher.mTelephonyNetworkType);
    }

    @Test
    public void testUpdateTelephonyVoiceNetworkType() {
        mDcNetWatcher.updateTelephonyVoiceNetworkType(TelephonyManager.NETWORK_TYPE_LTE);

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mDcNetWatcher.mTelephonyVoiceNetworkType);
    }

    @Test
    public void testNotifyDataConnectionState() throws Exception {
        EApnType apnType = EApnType.IMS;
        EDataState dataState = EDataState.DATA_STATE_DISCONNECTED;

        mDcNetWatcher.notifyDataConnectionState(apnType, dataState);

        verify(mNetWatherListener).onDataConnectionStateChanged(apnType, dataState);
    }

    @Test
    public void testNotifyPdnConnectionFailed() throws Exception {
        EApnType apnType = EApnType.IMS;
        int cause = 33;

        mDcNetWatcher.notifyPdnConnectionFailed(apnType, cause);

        verify(mNetWatherListener).onPdnConnectionFailed(apnType, cause);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_ngran() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.NGRAN);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_NR});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_NGRAN), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_eutran() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.EUTRAN);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_LTE});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_EUTRAN), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_ehrpd() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.CDMA2000);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_EHRPD});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_EHRPD), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_utran() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.UTRAN);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_HSPA});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_UTRAN), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_geran() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.GERAN);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_EDGE});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_GEREAN), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_evdo() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.CDMA2000);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_EVDO_A});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_EVDO), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_1xrtt() throws Exception {
        mDcNetWatcher.mImsSupportedAccessNetworks =
                List.of(AccessNetworkConstants.AccessNetworkType.CDMA2000);

        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {TelephonyManager.NETWORK_TYPE_1xRTT});

        assertEquals(Integer.valueOf(IDcNetWatcher.AN_1XRTT), accessNetworkType);
    }

    @Test
    public void testGetAvailableInternalAccessNetworkType_unknown() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getAvailableInternalAccessNetworkType",
                new Class[] {int.class}, new Object[] {0});

        assertEquals(Integer.valueOf(0), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_ngran() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_NR});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_NGRAN), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_eutran() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_LTE});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_EUTRAN), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_ehrpd() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_EHRPD});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_EHRPD), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_utran() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_UMTS});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_UTRAN), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_geran() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_GPRS});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_GEREAN), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_evdo() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_EVDO_0});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_EVDO), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_1xrtt() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class},
                new Object[] {TelephonyManager.NETWORK_TYPE_CDMA});
        assertEquals(Integer.valueOf(IDcNetWatcher.AN_1XRTT), accessNetworkType);
    }

    @Test
    public void testGetInternalAccessNetworkType_unknown() {
        Integer accessNetworkType = (Integer) invokeMethod(mDcNetWatcher,
                "getInternalAccessNetworkType", new Class[] {int.class}, new Object[] {0});
        assertEquals(Integer.valueOf(0), accessNetworkType);
    }

    @Test
    public void testOnServiceStateChanged_handleVoiceServiceStateChanged() {
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_IN_SERVICE);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_VOICE_SERVICE_STATE,
                ServiceState.STATE_IN_SERVICE, 0);
        assertEquals(ServiceState.STATE_IN_SERVICE, mDcNetWatcher.getVoiceServiceState());
    }

    @Test
    public void testOnServiceStateChanged_handleNetworkOperatorChanged() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_IN_SERVICE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN)).thenReturn(null);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);
        when(mServiceState.getOperatorNumeric()).thenReturn("45000");

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onNetworkOperatorChanged("45000");
        assertEquals("45000", mDcNetWatcher.getNetworkOperator());
    }

    @Test
    public void testOnServiceStateChanged_handleDataServiceStateChangedWithWwan() throws Exception {
        NetworkRegistrationInfo iwlanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                TelephonyManager.NETWORK_TYPE_IWLAN, false);
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_IN_SERVICE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN)).thenReturn(iwlanInfo);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onDataServiceStateChanged(ServiceState.STATE_IN_SERVICE);
        verify(mMockSystem).notifyServiceStateChanged(ServiceState.STATE_IN_SERVICE);
        assertEquals(ServiceState.STATE_IN_SERVICE, mDcNetWatcher.getDataServiceState());
        assertEquals(ServiceState.STATE_IN_SERVICE, mDcNetWatcher.getCellularDataServiceState());
    }

    @Test
    public void testOnServiceStateChanged_handleDataServiceStateChangedWithIwlan()
            throws Exception {
        NetworkRegistrationInfo iwlanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_IWLAN, false);
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_IN_SERVICE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN)).thenReturn(iwlanInfo);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onDataServiceStateChanged(ServiceState.STATE_IN_SERVICE);
        verify(mMockSystem).notifyServiceStateChanged(ServiceState.STATE_IN_SERVICE);
        assertEquals(ServiceState.STATE_IN_SERVICE, mDcNetWatcher.getDataServiceState());
        assertEquals(ServiceState.STATE_OUT_OF_SERVICE,
                mDcNetWatcher.getCellularDataServiceState());
    }

    @Test
    public void testOnServiceStateChanged_handleDataServiceStateChangedWithEmergency()
            throws Exception {
        NetworkRegistrationInfo iwlanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                TelephonyManager.NETWORK_TYPE_IWLAN, false);
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                TelephonyManager.NETWORK_TYPE_LTE, true);
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_EMERGENCY_ONLY);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN)).thenReturn(iwlanInfo);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onDataServiceStateChanged(ServiceState.STATE_EMERGENCY_ONLY);
        verify(mMockSystem).notifyServiceStateChanged(ServiceState.STATE_EMERGENCY_ONLY);
        assertEquals(ServiceState.STATE_EMERGENCY_ONLY, mDcNetWatcher.getDataServiceState());
        assertEquals(ServiceState.STATE_EMERGENCY_ONLY,
                mDcNetWatcher.getCellularDataServiceState());
    }

    @Test
    public void testOnServiceStateChanged_handleDataServiceStateChangedWithPowerOff()
            throws Exception {
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_POWER_OFF);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WLAN)).thenReturn(null);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(null);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onDataServiceStateChanged(ServiceState.STATE_POWER_OFF);
        verify(mMockSystem).notifyServiceStateChanged(ServiceState.STATE_POWER_OFF);
        assertEquals(ServiceState.STATE_POWER_OFF, mDcNetWatcher.getDataServiceState());
        assertEquals(ServiceState.STATE_POWER_OFF, mDcNetWatcher.getCellularDataServiceState());
    }

    @Test
    public void testOnServiceStateChanged_handleNetworkTypeChangeWithLte() throws Exception {
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onDataNetworkTypeChanged();
        verify(mMockSystem).notifyNetworkTypeChanged(IDcNetWatcher.AN_EUTRAN);
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mDcNetWatcher.getNetworkType());
        assertEquals(AccessNetworkConstants.AccessNetworkType.EUTRAN,
                mDcNetWatcher.getAccessNetworkType());
    }

    @Test
    public void testOnServiceStateChanged_handleVoiceNetworkTypeChangeWithLte() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_CS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onVoiceNetworkTypeChanged();
        verify(mMockSystem).notifyVoiceNetworkTypeChanged(IDcNetWatcher.AN_EUTRAN);
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mDcNetWatcher.getVoiceNetworkType());
    }

    @Test
    public void testOnServiceStateChanged_handleRoamingStateChangedWhenRoaming() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_CS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);
        when(mServiceState.getRoaming()).thenReturn(true);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onRoamingStateChanged(true);
        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_ROAMING_STATE,
                ImsEventDef.IMS_ROAMING_STATE_ON, ImsEventDef.IMS_ROAMING_STATE_ON);
        assertTrue(mDcNetWatcher.isRoaming());
        assertTrue(mDcNetWatcher.isVoiceRoaming());
        assertTrue(mDcNetWatcher.isDataNetworkRoaming());
    }

    @Test
    public void testOnServiceStateChanged_handleImsNetworkFeatureWithLte() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        DataSpecificRegistrationInfo dsrInfo =
                new DataSpecificRegistrationInfo(2, false, true, true, null);
        replaceInstance(DataSpecificRegistrationInfo.class, "mLteAttachResultType", dsrInfo,
                DataSpecificRegistrationInfo.LTE_ATTACH_TYPE_EPS_ONLY);
        replaceInstance(NetworkRegistrationInfo.class, "mDataSpecificInfo", wwanInfo, dsrInfo);
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_LTE_INFO,
                ImsEventDef.IMS_LTE_INFO_EPS_ONLY_ATTACHED,
                ImsEventDef.IMS_LTE_INFO_EXTRA_NONE);
    }

    @Test
    public void testOnServiceStateChanged_handleImsNetworkFeature_VopsChanged() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        LteVopsSupportInfo vsi = new LteVopsSupportInfo(LteVopsSupportInfo.LTE_STATUS_NOT_SUPPORTED,
                LteVopsSupportInfo.LTE_STATUS_SUPPORTED);
        DataSpecificRegistrationInfo dsrInfo =
                new DataSpecificRegistrationInfo(2, false, true, true, vsi);
        replaceInstance(NetworkRegistrationInfo.class, "mDataSpecificInfo", wwanInfo, dsrInfo);
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED);
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);
        when(mServiceState.getDuplexMode()).thenReturn(ServiceState.DUPLEX_MODE_FDD);
        when(mServiceState.getOperatorNumeric()).thenReturn("123456");
        when(mMockDcSetting.isVopsIgnored()).thenReturn(false);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE,
                ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED, 0);
        verify(mNetWatherListener).onVopsStateChanged(ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED,
                "123456");
        assertTrue(mDcNetWatcher.isEmergencyServiceSupported());
        assertFalse(mDcNetWatcher.isVopsSupported());
    }

    @Test
    public void testOnServiceStateChanged_handleImsNetworkFeature_VopsTrueIfNetWorkNotRegistered()
            throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        LteVopsSupportInfo vsi = new LteVopsSupportInfo(LteVopsSupportInfo.LTE_STATUS_NOT_SUPPORTED,
                LteVopsSupportInfo.LTE_STATUS_SUPPORTED);
        DataSpecificRegistrationInfo dsrInfo =
                new DataSpecificRegistrationInfo(2, false, true, true, vsi);
        replaceInstance(NetworkRegistrationInfo.class, "mDataSpecificInfo", wwanInfo, dsrInfo);
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED);
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);
        when(mServiceState.getDuplexMode()).thenReturn(ServiceState.DUPLEX_MODE_FDD);
        when(mServiceState.getOperatorNumeric()).thenReturn("123456");
        when(mMockDcSetting.isVopsIgnored()).thenReturn(false);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mMockSystem, never())
                .notifyEvent(eq(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE), anyInt(), eq(0));
        verify(mNetWatherListener, never()).onVopsStateChanged(anyInt(), anyString());
        assertTrue(mDcNetWatcher.isVopsSupported());
    }

    @Test
    public void testOnServiceStateChanged_handleImsNetworkFeature_VopsTrueOnNon4G5G()
            throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_UMTS, false);
        DataSpecificRegistrationInfo dsrInfo =
                new DataSpecificRegistrationInfo(2, false, true, true, null);
        replaceInstance(NetworkRegistrationInfo.class, "mDataSpecificInfo", wwanInfo, dsrInfo);
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED);
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mMockSystem, never())
                .notifyEvent(eq(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE), anyInt(), eq(0));
        verify(mNetWatherListener, never()).onVopsStateChanged(anyInt(), anyString());
        assertTrue(mDcNetWatcher.isVopsSupported());
    }

    @Test
    public void testOnServiceStateChanged_handleImsNetworkFeature_PlmnChanged() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        LteVopsSupportInfo vsi = new LteVopsSupportInfo(LteVopsSupportInfo.LTE_STATUS_NOT_SUPPORTED,
                LteVopsSupportInfo.LTE_STATUS_SUPPORTED);
        DataSpecificRegistrationInfo dsrInfo =
                new DataSpecificRegistrationInfo(2, false, true, true, vsi);
        replaceInstance(NetworkRegistrationInfo.class, "mDataSpecificInfo", wwanInfo, dsrInfo);
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED);
        replaceInstance(DcNetWatcher.class, "mImsVopsPlmn", mDcNetWatcher, "111111");
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);
        when(mServiceState.getDuplexMode()).thenReturn(ServiceState.DUPLEX_MODE_FDD);
        when(mServiceState.getOperatorNumeric()).thenReturn("222222");
        when(mMockDcSetting.isVopsIgnored()).thenReturn(false);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});
        verify(mNetWatherListener).onVopsStateChanged(ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED,
                "222222");
        assertFalse(mDcNetWatcher.isVopsSupported());
    }

    @Test
    public void testOnServiceStateChanged_handleVoiceRoamingTypeChanged() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_CS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        assertEquals(ServiceState.ROAMING_TYPE_UNKNOWN, mDcNetWatcher.getVoiceRoamingType());
    }

    @Test
    public void testOnServiceStateChanged_handleDataRoamingTypeChanged() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        assertEquals(ServiceState.ROAMING_TYPE_UNKNOWN, mDcNetWatcher.getDataRoamingType());
    }

    @Test
    public void testOnServiceStateChanged_handleNetworkRegistrationStateChanged() throws Exception {
        NetworkRegistrationInfo wwanInfo = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY,
                TelephonyManager.NETWORK_TYPE_LTE, false);
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo);

        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});

        verify(mNetWatherListener).onNetworkRegistrationStateChanged(
                NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY);

        assertTrue(mDcNetWatcher.isEmergencyOnly());
    }

    @Test
    public void testOnServiceStateChanged_handleNetworkRegistrationStateAndRejectCauseChanged()
            throws Exception {
        int rejectCause = 3;
        NetworkRegistrationInfo wwanInfo1 = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_DENIED,
                TelephonyManager.NETWORK_TYPE_LTE, false, rejectCause);
        NetworkRegistrationInfo wwanInfo2 = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY,
                TelephonyManager.NETWORK_TYPE_LTE, false, REGISTRATION_REJECT_CAUSE_NONE);
        NetworkRegistrationInfo wwanInfo3 = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, false, REGISTRATION_REJECT_CAUSE_NONE);

        // Initially, no reject cause.
        assertEquals(REGISTRATION_REJECT_CAUSE_NONE,
                mDcNetWatcher.getNetworkRegistrationRejectCause());

        // REGISTRATION_STATE_DENIED, update reject cause.
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo1);
        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});
        assertEquals(rejectCause, mDcNetWatcher.getNetworkRegistrationRejectCause());

        // REGISTRATION_STATE_EMERGENCY, keep previous reject cause.
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo2);
        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});
        assertEquals(rejectCause, mDcNetWatcher.getNetworkRegistrationRejectCause());

        // REGISTRATION_STATE_HOME, reset reject cause.
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo3);
        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});
        assertEquals(REGISTRATION_REJECT_CAUSE_NONE,
                mDcNetWatcher.getNetworkRegistrationRejectCause());
    }

    @Test
    public void testOnServiceStateChanged_clearNetworkRegistrationRejectCause()
            throws Exception {
        int rejectCause = 3;
        NetworkRegistrationInfo wwanInfo1 = createNetworkRegistrationInfo(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN,
                NetworkRegistrationInfo.REGISTRATION_STATE_DENIED,
                TelephonyManager.NETWORK_TYPE_LTE, false, rejectCause);

        // Update reject cause.
        when(mServiceState.getNetworkRegistrationInfo(NetworkRegistrationInfo.DOMAIN_PS,
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).thenReturn(wwanInfo1);
        invokeMethod(mDcNetWatcher.mPhoneStateListener, "onServiceStateChanged",
                new Class[] {ServiceState.class}, new Object[] {mServiceState});
        assertEquals(rejectCause, mDcNetWatcher.getNetworkRegistrationRejectCause());

        // Clear reject cause.
        mDcNetWatcher.clearNetworkRegistrationRejectCause();

        assertEquals(REGISTRATION_REJECT_CAUSE_NONE,
                mDcNetWatcher.getNetworkRegistrationRejectCause());
    }

    @Test
    public void testDcNetWatcherHandler_handleAirplaneModeChangedWhenOn() throws Exception {
        when(mSettingsProxy.getInt(AIRPLANE_MODE_ON, -1)).thenReturn(1);

        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED).putExtra("state", true);
        Message.obtain(mDcNetWatcher.mDcNetWatcherHandler, DcNetWatcher.EVENT_AIRPLANE_MODE_CHANGED,
                intent).sendToTarget();
        processAllMessages();

        verify(mMockSystem).notifyAirplaneModeChanged(1);
        verify(mNetWatherListener).onAirplaneModeChanged(true);
        assertTrue(mDcNetWatcher.isAirplaneMode());
    }

    @Test
    public void testDcNetWatcherHandler_handleAirplaneModeChangedWhenStateIsDifferentFromSetting()
            throws Exception {
        when(mSettingsProxy.getInt(AIRPLANE_MODE_ON, -1)).thenReturn(-1);

        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED).putExtra("state", true);
        Message.obtain(mDcNetWatcher.mDcNetWatcherHandler, DcNetWatcher.EVENT_AIRPLANE_MODE_CHANGED,
                intent).sendToTarget();
        processAllMessages();

        verify(mMockSystem, never()).notifyAirplaneModeChanged(anyInt());
        verify(mNetWatherListener, never()).onAirplaneModeChanged(anyBoolean());
        assertFalse(mDcNetWatcher.isAirplaneMode());
    }

    @Test
    public void testOnCarrierConfigChanged() throws Exception {
        ArgumentCaptor<ConfigInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(ConfigInterface.Listener.class);
        verify(mMockConfigInterface).addListener(listenerCaptor.capture());

        ConfigInterface.Listener listener = listenerCaptor.getValue();
        listener.onCarrierConfigChanged(SLOT0, SUB_ID_1);

        // Called once in init() and again in onCarrierConfigChanged().
        verify(mMockDcSetting, times(2)).getImsSupportedAccessNetworks();
    }

    @Test
    public void testOnNativeServiceReady() throws Exception {
        replaceInstance(DcNetWatcher.class, "mAirplaneMode", mDcNetWatcher, true);
        replaceInstance(DcNetWatcher.class, "mDataRoaming", mDcNetWatcher, false);
        replaceInstance(DcNetWatcher.class, "mVoiceRoaming", mDcNetWatcher, true);
        replaceInstance(DcNetWatcher.class, "mVoiceServiceState", mDcNetWatcher,
                ServiceState.STATE_IN_SERVICE);

        ArgumentCaptor<NativeStateInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mMockNativeStateInterface).addListener(listenerCaptor.capture());

        NativeStateInterface.Listener listener = listenerCaptor.getValue();
        listener.onNativeServiceReady();

        verify(mNetWatherListener).onAirplaneModeChanged(true);
        verify(mMockSystem).notifyAirplaneModeChanged(1);
        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_ROAMING_STATE,
                ImsEventDef.IMS_ROAMING_STATE_OFF, ImsEventDef.IMS_ROAMING_STATE_ON);
        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_VOICE_SERVICE_STATE,
                ServiceState.STATE_IN_SERVICE, 0);
        verify(mMockSystem).notifyEvent(ImsEventDef.IMS_EVENT_LTE_INFO,
                ImsEventDef.IMS_LTE_INFO_UNKNOWN,
                ImsEventDef.IMS_LTE_INFO_EXTRA_NONE);
    }

    @Test
    public void testOnReceive() {
        ArgumentCaptor<BroadcastReceiver> captor = ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mTestAppContext.getBroadcastReceiverProxy())
                .registerReceiver(captor.capture(), any(IntentFilter.class));
        BroadcastReceiver receiver = captor.getValue();
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        receiver.onReceive(mTestAppContext.getContext(), intent);

        assertTrue(mDcNetWatcher.mDcNetWatcherHandler.hasMessages(
                DcNetWatcher.EVENT_AIRPLANE_MODE_CHANGED));
    }

    /*
     * The following test functions are intended to test functions that are not currently in use or
     * don't have actual implementations.
     */
    @Test
    public void testGetMocnPlmnInfo() {
        assertEquals(0, mDcNetWatcher.getMocnPlmnInfo());
        // TODO : no actual implementations so far
    }

    @Test
    public void testIsEmergencyOnly() throws Exception {
        replaceInstance(DcNetWatcher.class, "mNetworkRegistrationState",
                mDcNetWatcher, NetworkRegistrationInfo.REGISTRATION_STATE_HOME);
        assertFalse(mDcNetWatcher.isEmergencyOnly());

        replaceInstance(DcNetWatcher.class, "mNetworkRegistrationState",
                mDcNetWatcher, NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY);
        assertTrue(mDcNetWatcher.isEmergencyOnly());
    }

    @Test
    public void testIs3G() throws Exception {
        replaceInstance(DcNetWatcher.class, "mNetworkType", mDcNetWatcher,
                TelephonyManager.NETWORK_TYPE_UMTS);

        assertTrue(mDcNetWatcher.is3G());
    }

    @Test
    public void testIs4G() throws Exception {
        replaceInstance(DcNetWatcher.class, "mNetworkType", mDcNetWatcher,
                TelephonyManager.NETWORK_TYPE_LTE);

        assertTrue(mDcNetWatcher.is4G());
    }

    @Test
    public void testIs5G() throws Exception {
        replaceInstance(DcNetWatcher.class, "mNetworkType", mDcNetWatcher,
                TelephonyManager.NETWORK_TYPE_NR);

        assertTrue(mDcNetWatcher.is5G());
    }

    @Test
    public void testIsVopsSupported_VopsSupported() throws Exception {
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED);
        assertTrue(mDcNetWatcher.isVopsSupported());
    }

    @Test
    public void testIsVopsSupported_VopsInvalid() throws Exception {
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_INVALID);
        assertTrue(mDcNetWatcher.isVopsSupported());
    }

    @Test
    public void testIsVopsSupported_VopsNotSupported() throws Exception {
        replaceInstance(DcNetWatcher.class, "mImsVopsState", mDcNetWatcher,
                ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED);
        assertFalse(mDcNetWatcher.isVopsSupported());
    }

    private NetworkRegistrationInfo createNetworkRegistrationInfo(int transportType,
            int registrationState, int networkType, boolean isEmergencyOnly) {
        return createNetworkRegistrationInfo(transportType, registrationState, networkType,
                isEmergencyOnly, REGISTRATION_REJECT_CAUSE_NONE);
    }

    private NetworkRegistrationInfo createNetworkRegistrationInfo(int transportType,
            int registrationState, int networkType, boolean isEmergencyOnly, int rejectCause) {
        return new NetworkRegistrationInfo.Builder()
                .setTransportType(transportType)
                .setRegistrationState(registrationState)
                .setAccessNetworkTechnology(networkType)
                .setCellIdentity(createCellIdentity(networkType))
                .setEmergencyOnly(isEmergencyOnly)
                .setRejectCause(rejectCause)
                .build();
    }

    private CellIdentity createCellIdentity(int networkType) {
        return switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE -> new CellIdentityLte(0x1111111, 13, 0x2222, 0,
                    new int[]{}, 0, "001", "01",
                    "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_NR -> new CellIdentityNr(20, 0x333333, 2,
                    new int[]{}, "001", "01", 0x555555555L,
                    "Test-SIM", "Test", Collections.emptyList());
            case TelephonyManager.NETWORK_TYPE_UMTS, TelephonyManager.NETWORK_TYPE_HSDPA,
                    TelephonyManager.NETWORK_TYPE_HSUPA, TelephonyManager.NETWORK_TYPE_HSPA,
                    TelephonyManager.NETWORK_TYPE_HSPAP ->
                    new CellIdentityWcdma(0x6666, 0x7777777, 3, 0, "001", "01", "Test-SIM",
                            "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_GPRS, TelephonyManager.NETWORK_TYPE_EDGE ->
                    new CellIdentityGsm(0x8888, 0x9999, 0, 1, "001", "01", "Test-SIM", "Test",
                            Collections.emptyList());
            default -> null;
        };
    }
}
