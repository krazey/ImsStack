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

package com.android.imsstack.enabler.ssc;

import static android.telephony.ServiceState.STATE_IN_SERVICE;
import static android.telephony.ServiceState.STATE_OUT_OF_SERVICE;
import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_CROSS_SIM;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_LTE;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.TelephonyNetworkSpecifier;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TimerInterface;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.CarrierConfig.ImsSs;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationType;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.enabler.ssc.SscServiceState.SscCrossSimDataStateListener;
import com.android.imsstack.enabler.ssc.SscServiceState.SscMobileDataStateListener;
import com.android.imsstack.enabler.ssc.SscServiceState.SscRegiStateListener;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class SscServiceStateTest {
    private TestAppContext mTestAppContext;
    private SscServiceState mSscServiceState;

    private final long mTimerId = 1L;
    private final int mBlockTimer = 1;
    private TestableLooper mLooper;

    private TelephonyManagerProxy mMockTelephonyManagerProxy;
    private ConnectivityManagerProxy mMockConnectivityManagerProxy;
    @Mock private TimerInterface mMockTimerInterface;
    @Mock private AosService mMockAosService;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private IDcNetWatcher mMockDcNetWatcher;
    @Mock private IUtInterface mMockUtInterface;
    @Mock private WifiInterface mMockWifiInterface;
    @Mock private SimInterface mMockSimInterface;
    @Mock private ConfigInterface mMockConfigInterface;

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.initMocks(this);

        SscConfig.setConfigInterface(SLOT0, mMockConfigInterface);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN, AccessNetworkType.IWLAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_TEMPORARY_BLOCK_TIMER_WITH_ANY_REASON_SEC_INT))
                .thenReturn(mBlockTimer);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT)).thenReturn(mBlockTimer);

        when(mMockTimerInterface.startTimer(anyLong(), any(TimerInterface.Listener.class)))
                .thenReturn(mTimerId);
        when(mMockWifiInterface.isWifiConnected()).thenReturn(false);
        when(mMockDcNetWatcher.isRoaming()).thenReturn(false);
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_IN_SERVICE);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mMockSimInterface.getSubId()).thenReturn(SUB_ID_1);

        Context context = new ContextFixture().getTestDouble();
        mTestAppContext = new TestAppContext(context);
        mTestAppContext.setUp();

        mMockTelephonyManagerProxy =
                mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        mMockConnectivityManagerProxy =
                mTestAppContext.getSystemServiceProxy(ConnectivityManagerProxy.class);

        AgentFactory.getInstance().setAgent(TimerInterface.class, mMockTimerInterface);
        AgentFactory.getInstance().setAgent(WifiInterface.class, mMockWifiInterface);
        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT0);
        AosFactory.getInstance().replaceService(SLOT0, mMockAosService);

        DcFactory.setDcAgent(IDcNetWatcher.class, mMockDcNetWatcher, SLOT0);
        UtFactory.getInstance().setUtInterfaceForSlot(SLOT0, mMockUtInterface);

        mLooper = new TestableLooper(Looper.getMainLooper());
    }

    @After
    public void tearDown() {
        if (mSscServiceState != null) {
            mSscServiceState.deInit();
        }

        if (mLooper != null) {
            mLooper.destroy();
            mLooper = null;
        }

        AgentFactory.getInstance().setAgent(TimerInterface.class, null);
        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        AosFactory.getInstance().replaceService(SLOT0, null);
        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT0);
        UtFactory.getInstance().setUtInterfaceForSlot(SLOT0, null);

        SscConfig.clear(SLOT0);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testInit_registerListeners() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN, AccessNetworkType.IWLAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);

        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();

        verify(mMockDcNetWatcher).addListener(any(IDcNetWatcher.Listener.class));
        verify(mMockSimInterface).addListener(mSscServiceState.mSimStateListener);
        verify(mMockConfigInterface).addListener(mSscServiceState.mCarrierConfigListener);

        verify(mMockWifiInterface).addListener(mSscServiceState.mWifiListener);
        verify(mMockAosService).addListener(mSscServiceState.mRegiStateListener);
        verify(mMockTelephonyManagerProxy).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(),
                mSscServiceState.mMobileDataStateListener);
        verify(mMockConnectivityManagerProxy)
                .registerSystemDefaultNetworkCallback(
                        mSscServiceState.mCrossSimDataStateListener, mSscServiceState.mHandler);
        assertTrue(((Handler) mSscServiceState.mHandler)
                .hasMessages(SscServiceState.EVENT_UT_CAPABILITY_CHANGED));

        processAllMessages();
        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testInit_notRegisterListenersAccordingToCarrierConfig() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(false);

        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        processAllMessages();

        verify(mMockUtInterface).onServiceStateChanged();
        verify(mMockDcNetWatcher).addListener(any(IDcNetWatcher.Listener.class));
        verify(mMockSimInterface).addListener(mSscServiceState.mSimStateListener);
        verify(mMockConfigInterface).addListener(mSscServiceState.mCarrierConfigListener);

        verify(mMockWifiInterface, never()).addListener(mSscServiceState.mWifiListener);
        verify(mMockAosService, never()).addListener(mSscServiceState.mRegiStateListener);
        verify(mMockTelephonyManagerProxy, never()).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(),
                mSscServiceState.mMobileDataStateListener);
        verify(mMockConnectivityManagerProxy, never())
                .registerSystemDefaultNetworkCallback(
                        mSscServiceState.mCrossSimDataStateListener, mSscServiceState.mHandler);
    }

    @Test
    public void testDeInit_unregisterListeners() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN, AccessNetworkType.IWLAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);

        ISscAuthAgent authAgent = SscAuthAgent.getInstance(SLOT0);
        authAgent.setIsCredentialInfoUpdated(true);
        authAgent.setLastSuccessfulGbaMode(SscConfig.GBA_ME);
        authAgent.setETag("etag");

        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        processAllMessages();
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtAvailability = true;

        mSscServiceState.deInit();
        processAllMessages();

        verify(mMockDcNetWatcher).removeListener(any(IDcNetWatcher.Listener.class));
        verify(mMockSimInterface).removeListener(mSscServiceState.mSimStateListener);
        verify(mMockConfigInterface).removeListener(mSscServiceState.mCarrierConfigListener);

        verify(mMockWifiInterface).removeListener(any(WifiInterface.Listener.class));
        verify(mMockAosService).removeListener(any(SscRegiStateListener.class));
        verify(mMockTelephonyManagerProxy)
                .unregisterTelephonyCallback(any(SscMobileDataStateListener.class));
        verify(mMockConnectivityManagerProxy)
                .unregisterNetworkCallback(any(SscCrossSimDataStateListener.class));

        // verifying deInit() calls resetAllUtStatus()
        verify(mMockUtInterface).onServiceStateChanged();
        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        assertEquals(false, authAgent.isCredentialInfoUpdated());
        assertEquals(SscConfig.GBA_NONE, authAgent.getLastSuccessfulGbaMode());
        assertEquals("", authAgent.getETag());
    }

    @Test
    public void testDeInit_notUnregisterListenersAccordingToCarrierConfig() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(false);

        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        processAllMessages();
        verify(mMockUtInterface).onServiceStateChanged();

        mSscServiceState.deInit();
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        verify(mMockDcNetWatcher).removeListener(any(IDcNetWatcher.Listener.class));
        verify(mMockSimInterface).removeListener(mSscServiceState.mSimStateListener);
        verify(mMockConfigInterface).removeListener(mSscServiceState.mCarrierConfigListener);

        verify(mMockWifiInterface, never()).removeListener(any(WifiInterface.Listener.class));
        verify(mMockAosService, never()).removeListener(any(SscRegiStateListener.class));
        verify(mMockTelephonyManagerProxy, never())
                .unregisterTelephonyCallback(any(SscMobileDataStateListener.class));
        verify(mMockConnectivityManagerProxy, never())
                .unregisterNetworkCallback(any(SscCrossSimDataStateListener.class));
    }

    @Test
    public void testDeInit_removesPendingMessages() {
        // Verifies that deInit() cancels pending operations scheduled by init().
        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());

        // Call init(), which posts a delayed operation to check UT capability.
        mSscServiceState.init();

        // Call deInit() before the delayed operation has a chance to be processed.
        // This should cancel the pending operation.
        mSscServiceState.deInit();

        // Advance the looper's time past the delay of the original operation.
        processAllMessages();

        // Verify that onServiceStateChanged was not called. This confirms that the pending
        // operation was successfully canceled by deInit(), and that deInit() itself did not
        // trigger a capability change notification because the state hadn't changed yet.
        verify(mMockUtInterface, never()).onServiceStateChanged();
    }

    @Test
    public void testIsUtAvailable() {
        createAndInitSscServiceState();

        mSscServiceState.mUtAvailability = false;
        assertEquals(false, mSscServiceState.isUtAvailable());

        mSscServiceState.mUtAvailability = true;
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testChangeCapabilities_utEnabled() {
        createAndInitSscServiceState();
        mSscServiceState.mIsUtFeatureEnabled = false;
        mSscServiceState.mUtAvailability = false;

        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_UT, REGISTRATION_TECH_LTE));
        disabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE));

        mSscServiceState.changeCapabilities(enabledCaps, disabledCaps);
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mIsUtFeatureEnabled);
    }

    @Test
    public void testChangeCapabilities_utDisabled() {
        createAndInitSscServiceState();
        mSscServiceState.mIsUtFeatureEnabled = true;
        mSscServiceState.mUtAvailability = true;

        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE));
        disabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_UT, REGISTRATION_TECH_LTE));

        mSscServiceState.changeCapabilities(enabledCaps, disabledCaps);
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mIsUtFeatureEnabled);
    }

    @Test
    public void testChangeCapabilities_noUtCapability() {
        createAndInitSscServiceState();

        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE));

        mSscServiceState.changeCapabilities(enabledCaps, disabledCaps);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mIsUtFeatureEnabled);
    }

    @Test
    public void testChangeCapabilities_crossSimEnabled() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = false;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = true;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_CROSS_SIM));
        disabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE));

        mSscServiceState.changeCapabilities(enabledCaps, disabledCaps);
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mIsCrossSimFeatureEnabled);
    }

    @Test
    public void testChangeCapabilities_crossSimDisabled() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = true;
        mSscServiceState.mUtAvailability = true;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE));
        disabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_CROSS_SIM));

        mSscServiceState.changeCapabilities(enabledCaps, disabledCaps);
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mIsCrossSimFeatureEnabled);
    }

    @Test
    public void testChangeCapabilities_noCrossSimCapability() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();

        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE));

        mSscServiceState.changeCapabilities(enabledCaps, disabledCaps);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mIsCrossSimFeatureEnabled);
    }

    @Test
    public void testSetErrorResponseCode_codeNoneBlock() {
        createAndInitSscServiceState();
        int[] emptyBlockErrorCodes = {};
        int errorCode = 403;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY))
                .thenReturn(emptyBlockErrorCodes);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY))
                .thenReturn(emptyBlockErrorCodes);
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setErrorResponseCode(errorCode);

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSetErrorResponseCode_codeTempBlock() {
        int[] tempBlockErrorCodes = {480};
        int errorCode = 480;
        when(mMockCarrierConfig.getIntArray(ImsSs.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY))
                .thenReturn(tempBlockErrorCodes);
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setErrorResponseCode(errorCode);

        verify(mMockTimerInterface)
                .startTimer(eq(mBlockTimer * 60 * 1000L), any(TimerInterface.Listener.class));
        assertEquals(false, mSscServiceState.isUtAvailable());
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testSetErrorResponseCode_codePermBlock() {
        int[] permBlockErrorCodes = {499};
        int errorCode = 403;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY))
                .thenReturn(permBlockErrorCodes);
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setErrorResponseCode(errorCode);

        assertEquals(false, mSscServiceState.isUtAvailable());
        verifyNoMoreInteractions(mMockTimerInterface);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testSetPdnConnectionFailed_temporaryCause() {
        int[] tempBlockSmCodes = {33};
        int smCause = 33;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY))
                .thenReturn(tempBlockSmCodes);
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionFailed(smCause);

        verify(mMockTimerInterface)
                .startTimer(eq(mBlockTimer * 60 * 1000L), any(TimerInterface.Listener.class));
        assertEquals(false, mSscServiceState.isUtAvailable());
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testSetPdnConnectionFailed_permanentCause() {
        int[] permBlockSmCodes = {33};
        int smCause = 33;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_SM_CAUSE_PERMANENT_BLOCK_INT_ARRAY))
                .thenReturn(permBlockSmCodes);
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionFailed(smCause);

        assertEquals(false, mSscServiceState.isUtAvailable());
        verifyNoMoreInteractions(mMockTimerInterface);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testSetDnsQueryFailed() {
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setDnsQueryFailed(true);

        verify(mMockTimerInterface)
                .startTimer(eq(mBlockTimer * 1000L), any(TimerInterface.Listener.class));
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE, mSscServiceState.mUtBlockReason);

        mSscServiceState.setDnsQueryFailed(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
    }

    @Test
    public void testSetGbaRequestFailed() {
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setGbaRequestFailed(true);

        verify(mMockTimerInterface)
                .startTimer(eq(mBlockTimer * 1000L), any(TimerInterface.Listener.class));
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_GBA_FAILURE, mSscServiceState.mUtBlockReason);

        mSscServiceState.setGbaRequestFailed(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
    }

    @Test
    public void testSetPdnConnectionTimeout() {
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionTimeout(true);

        verify(mMockTimerInterface)
                .startTimer(eq(mBlockTimer * 1000L), any(TimerInterface.Listener.class));
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT,
                mSscServiceState.mUtBlockReason);

        mSscServiceState.setPdnConnectionTimeout(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
    }

    @Test
    public void testSetSocketConnectionExpired() {
        createAndInitSscServiceState();
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setSocketConnectionExpired(true);

        verify(mMockTimerInterface)
                .startTimer(eq(mBlockTimer * 1000L), any(TimerInterface.Listener.class));
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT,
                mSscServiceState.mUtBlockReason);

        mSscServiceState.setSocketConnectionExpired(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
    }

    @Test
    public void testSscServiceStateHandler_utBlockTimerExpired() {
        int[] tempBlockErrorCodes = {480};
        int errorCode = 480;
        when(mMockCarrierConfig.getIntArray(ImsSs.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY))
                .thenReturn(tempBlockErrorCodes);
        createAndInitSscServiceState();
        mSscServiceState.setErrorResponseCode(errorCode);
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT
                | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_GBA_FAILURE
                | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
        mSscServiceState.mUtAvailability = false;

        ArgumentCaptor<TimerInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(TimerInterface.Listener.class);
        verify(mMockTimerInterface).startTimer(anyLong(), listenerCaptor.capture());

        TimerInterface.Listener listener = listenerCaptor.getValue();
        listener.onTimerExpired(mTimerId);
        processAllMessages();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_airplaneModeOn() {
        createAndInitSscServiceState();
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.isAirplaneMode()).thenReturn(true);

        mSscServiceState.mNetWatcherListener.onAirplaneModeChanged(true);
        processAllMessages();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_airplaneModeOff() {
        createAndInitSscServiceState();
        int permanentBlockReasons = SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtBlockReason = permanentBlockReasons;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.isAirplaneMode()).thenReturn(false);

        mSscServiceState.mNetWatcherListener.onAirplaneModeChanged(false);
        processAllMessages();

        assertEquals(permanentBlockReasons, mSscServiceState.mUtBlockReason);
        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_wifiStateChangedConnectedWhenUtOverWifiSupported() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.IWLAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mMockWifiInterface.isWifiConnected()).thenReturn(true);

        ((WifiInterface.Listener) mSscServiceState.mWifiListener).onWifiConnectionStateChanged();
        processAllMessages();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_wifiStateChangedToDisconnectedWhenUtOverWifiSupported() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.IWLAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mMockWifiInterface.isWifiConnected()).thenReturn(false);

        ((WifiInterface.Listener) mSscServiceState.mWifiListener).onWifiConnectionStateChanged();
        processAllMessages();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_wifiStateChangedToConnectedWhenUtOverWifiNotSupported() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mMockWifiInterface.isWifiConnected()).thenReturn(true);

        ((WifiInterface.Listener) mSscServiceState.mWifiListener).onWifiConnectionStateChanged();
        processAllMessages();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_serviceStateChangedToInService() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_IN_SERVICE);

        mSscServiceState.mNetWatcherListener.onDataServiceStateChanged(STATE_IN_SERVICE);
        processAllMessages(); // for EVENT_DATA_SERVICE_STATE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_serviceStateChangedToOutOfService() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        mSscServiceState.mNetWatcherListener.onDataServiceStateChanged(STATE_OUT_OF_SERVICE);
        processAllMessages(); // for EVENT_DATA_SERVICE_STATE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataNetworkTypeChangedToValidNetworkType() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.NGRAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_NR);

        mSscServiceState.mNetWatcherListener.onDataNetworkTypeChanged();
        processAllMessages(); // for EVENT_DATA_NETWORK_TYPE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataNetworkTypeChangedToInvalidNetworkType() {
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.UTRAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_NR);

        mSscServiceState.mNetWatcherListener.onDataNetworkTypeChanged();
        processAllMessages(); // for EVENT_DATA_NETWORK_TYPE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataRoamingStateChanged() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL)).thenReturn(false);
        when(mMockDcNetWatcher.isRoaming()).thenReturn(false);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;

        when(mMockDcNetWatcher.isRoaming()).thenReturn(true);
        mSscServiceState.mNetWatcherListener.onRoamingStateChanged(true);
        processAllMessages(); // for EVENT_DATA_ROAMING_STATE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscSimStateListener_onSimCardStateChangedWhenSimAbsent() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtAvailability = false;
        ISscAuthAgent authAgent = SscAuthAgent.getInstance(SLOT0);
        authAgent.setIsCredentialInfoUpdated(true);
        authAgent.setLastSuccessfulGbaMode(SscConfig.GBA_ME);
        authAgent.setETag("etag");
        when(mMockSimInterface.getSimCardState()).thenReturn(Sim.STATE_ABSENT);

        ((Sim.Listener) mSscServiceState.mSimStateListener).onSimCardStateChanged();
        processAllMessages();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        assertEquals(false, authAgent.isCredentialInfoUpdated());
        assertEquals(SscConfig.GBA_NONE, authAgent.getLastSuccessfulGbaMode());
        assertEquals("", authAgent.getETag());
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testSscSimStateListener_onSimCardStateChangedWhenSimLoaded() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(false);
        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        TelephonyManagerProxy tmpForSubId2 = org.mockito.Mockito.mock(TelephonyManagerProxy.class);
        when(mMockSimInterface.getSimCardState()).thenReturn(Sim.STATE_LOADED);
        when(mMockSimInterface.getSubId()).thenReturn(SUB_ID_2);
        when(mMockTelephonyManagerProxy.createForSubscriptionId(SUB_ID_2)).thenReturn(tmpForSubId2);

        ((Sim.Listener) mSscServiceState.mSimStateListener).onSimCardStateChanged();

        verify(mMockTelephonyManagerProxy)
                .unregisterTelephonyCallback(any(SscMobileDataStateListener.class));
        verify(tmpForSubId2).registerTelephonyCallback(AppContext.getInstance().getMainExecutor(),
                mSscServiceState.mMobileDataStateListener);
    }

    @Test
    public void testSscCarrierConfigListener_onCarrierConfigChanged() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN, AccessNetworkType.IWLAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);

        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(false);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL)).thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(false);

        mSscServiceState.mUtAvailability = true;
        ((ConfigInterface.Listener) mSscServiceState.mCarrierConfigListener)
                .onCarrierConfigChanged(SLOT0, SUB_ID_1);
        processAllMessages();

        // Listeners are registered only once during init(). Not in onCarrierConfigChanged()
        verify(mMockWifiInterface, never()).addListener(mSscServiceState.mWifiListener);
        verify(mMockAosService, times(1)).addListener(any(SscRegiStateListener.class));
        verify(mMockTelephonyManagerProxy, times(1)).registerTelephonyCallback(any(Executor.class),
                any(SscMobileDataStateListener.class));
        verify(mMockConnectivityManagerProxy, times(1))
                .registerSystemDefaultNetworkCallback(
                        any(SscCrossSimDataStateListener.class), any(Handler.class));

        verify(mMockWifiInterface).removeListener(any(WifiInterface.Listener.class));
        verify(mMockAosService).removeListener(any(SscRegiStateListener.class));
        verify(mMockTelephonyManagerProxy)
                .unregisterTelephonyCallback(any(SscMobileDataStateListener.class));
        verify(mMockConnectivityManagerProxy)
                .unregisterNetworkCallback(any(SscCrossSimDataStateListener.class));
        verify(mMockUtInterface).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testRegistrationListener_notifyRegisteredWhenNotRegistered() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;

        mSscServiceState.mRegiStateListener.notifyRegistered(
                RegistrationType.NORMAL, NetworkType.LTE, 0, null);
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mRegiStateListener.getImsRegistrationState());
    }

    @Test
    public void testRegistrationListener_notifyRegisteredWhenRegistered() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;

        mSscServiceState.mRegiStateListener.notifyRegistered(
                RegistrationType.NORMAL, NetworkType.LTE, 0, null);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mRegiStateListener.getImsRegistrationState());
    }

    @Test
    public void testRegistrationListener_notifyRegistering() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;

        mSscServiceState.mRegiStateListener.notifyRegistering(
                RegistrationType.NORMAL, NetworkType.LTE, 0, null);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testRegistrationListener_notifyDeregisteredWhenNotRegistered() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;

        mSscServiceState.mRegiStateListener.notifyDeregistered(
                RegistrationType.NORMAL, NetworkType.LTE, ReasonCode.UNSPECIFIED, null,
                android.telephony.DataFailCause.NONE);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mRegiStateListener.getImsRegistrationState());
    }

    @Test
    public void testRegistrationListener_notifyDeregisteredWhenRegistered() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;
        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;

        mSscServiceState.mRegiStateListener.notifyDeregistered(
                RegistrationType.NORMAL, NetworkType.LTE, ReasonCode.UNSPECIFIED, null,
                android.telephony.DataFailCause.NONE);
        processAllMessages();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mRegiStateListener.getImsRegistrationState());
    }

    @Test
    public void testRegistrationListener_notifyDeregistering() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();

        mSscServiceState.mRegiStateListener.notifyDeregistering(RegistrationType.NORMAL);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyTechnologyChangeFailed() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);

        createAndInitSscServiceState();
        mSscServiceState.mRegiStateListener.notifyTechnologyChangeFailed(
                RegistrationType.NORMAL, NetworkType.LTE, ReasonCode.UNSPECIFIED, null);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyAssociatedUriChanged() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();

        mSscServiceState.mRegiStateListener.notifyAssociatedUriChanged(null);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyCapabilitiesUpdateFailed() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();

        mSscServiceState.mRegiStateListener.notifyCapabilitiesUpdateFailed(
                RegistrationType.NORMAL, NetworkType.LTE, 0);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyImsFeatureChanged() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();

        mSscServiceState.mRegiStateListener.notifyImsFeatureChanged(
                RegistrationType.NORMAL, NetworkType.LTE, 0);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyRegistered_nonNormalRegistration() {
        // Verifies that registration notifications with a non-NORMAL type are ignored.
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = false;
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;

        // Call notifyRegistered with a non-NORMAL registration type (e.g., EMERGENCY)
        mSscServiceState.mRegiStateListener.notifyRegistered(
                RegistrationType.EMERGENCY, NetworkType.LTE, 0, null);
        processAllMessages();

        // Verify that no state change or notification occurs
        verify(mMockUtInterface, times(1)).onServiceStateChanged(); // Only from init
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mRegiStateListener.getImsRegistrationState());
    }

    @Test
    public void testRegistrationListener_notifyDeregistered_nonNormalRegistration() {
        // Verifies that deregistration notifications with a non-NORMAL type are ignored.
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);
        when(mMockAosService.getRegistrationState()).thenReturn(RegistrationState.REGISTERED);
        createAndInitSscServiceState();
        mSscServiceState.mUtAvailability = true;
        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;

        // Call notifyDeregistered with a non-NORMAL registration type (e.g., EMERGENCY)
        mSscServiceState.mRegiStateListener.notifyDeregistered(
                RegistrationType.EMERGENCY, NetworkType.LTE, ReasonCode.UNSPECIFIED, null,
                android.telephony.DataFailCause.NONE);
        processAllMessages();

        // Verify that no state change or notification occurs
        verify(mMockUtInterface, times(1)).onServiceStateChanged(); // Only from init
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mRegiStateListener.getImsRegistrationState());
    }

    @Test
    public void testMobileDataStateListener_onUserMobileDataStateChangedToOffFromOn() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(false);
        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        mSscServiceState.mUtAvailability = true;
        mSscServiceState.mMobileDataStateListener.mMobileDataState = true;

        mSscServiceState.mMobileDataStateListener.onUserMobileDataStateChanged(false);
        processAllMessages();

        verify(mMockUtInterface).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mMobileDataStateListener.getUserMobileDataState());
    }

    @Test
    public void testMobileDataStateListener_onUserMobileDataStateChangedToOffFromOff() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL))
                .thenReturn(false);
        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();
        mSscServiceState.mMobileDataStateListener.mMobileDataState = false;

        mSscServiceState.mMobileDataStateListener.onUserMobileDataStateChanged(false);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.mMobileDataStateListener.getUserMobileDataState());
    }

    @Test
    public void testCrossSimDataStateListener_onLostWhileCrossSimDataAvailable() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = true;
        mSscServiceState.mUtAvailability = true;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        Network network = Mockito.mock(Network.class);
        mSscServiceState.mCrossSimDataStateListener.onLost(network);
        processAllMessages(); // for EVENT_CROSS_SIM_DATA_STATE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mCrossSimDataStateListener.getCrossSimDataState());
    }

    @Test
    public void testCrossSimDataStateListener_onLostWhileCrossSimDataUnavailable() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = false;
        mSscServiceState.mUtAvailability = true;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        Network network = Mockito.mock(Network.class);
        mSscServiceState.mCrossSimDataStateListener.onLost(network);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable()); // mUtAvailability not changed
        assertEquals(false, mSscServiceState.mCrossSimDataStateListener.getCrossSimDataState());
    }

    @Test
    public void testCrossSimDataStateListener_onCapabilitiesChangedWithoutCellularTransportType() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = false;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        Network network = Mockito.mock(Network.class);
        NetworkCapabilities networkCapabilities = new NetworkCapabilities.Builder().build();
        mSscServiceState.mCrossSimDataStateListener
                .onCapabilitiesChanged(network, networkCapabilities);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mCrossSimDataStateListener.getCrossSimDataState());
    }

    @Test
    public void testCrossSimDataStateListener_onCapabilitiesChangedWithoutNetworkSpecifier() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = false;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        Network network = Mockito.mock(Network.class);
        NetworkCapabilities networkCapabilities = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR).build();
        mSscServiceState.mCrossSimDataStateListener
                .onCapabilitiesChanged(network, networkCapabilities);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mCrossSimDataStateListener.getCrossSimDataState());
    }

    @Test
    public void testCrossSimDataStateListener_onCapabilitiesChangedWithSamSubscriptionId() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = false;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        Network network = Mockito.mock(Network.class);
        NetworkCapabilities networkCapabilities = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(SUB_ID_1))
                .build();
        mSscServiceState.mCrossSimDataStateListener
                .onCapabilitiesChanged(network, networkCapabilities);
        processAllMessages();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mCrossSimDataStateListener.getCrossSimDataState());
    }

    @Test
    public void testCrossSimDataStateListener_onCapabilitiesChangedWhenCrossSimAvailable() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL)).thenReturn(true);
        createAndInitSscServiceState();
        mSscServiceState.mIsCrossSimFeatureEnabled = true;
        mSscServiceState.mCrossSimDataStateListener.mCrossSimDataAvailable = false;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.getDataServiceState()).thenReturn(STATE_OUT_OF_SERVICE);

        Network network = Mockito.mock(Network.class);
        NetworkCapabilities networkCapabilities = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(SUB_ID_2))
                .build();
        mSscServiceState.mCrossSimDataStateListener
                .onCapabilitiesChanged(network, networkCapabilities);
        processAllMessages(); // for EVENT_CROSS_SIM_DATA_STATE_CHANGED
        processAllMessages(); // for EVENT_UT_CAPABILITY_CHANGED

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mCrossSimDataStateListener.getCrossSimDataState());
    }

    private void sendMessage(int what) {
        ((Handler) mSscServiceState.mHandler)
                .handleMessage(Message.obtain(mSscServiceState.mHandler, what));
    }

    private void processAllMessages() {
        mLooper.moveTimeForward(1000);
        while (!mLooper.getLooper().getQueue().isIdle()) {
            mLooper.processAllMessages();
        }
    }

    private void createAndInitSscServiceState() {
        mSscServiceState = new SscServiceState(SLOT0, mLooper.getLooper());
        mSscServiceState.init();

        processAllMessages();
        verify(mMockUtInterface).onServiceStateChanged();
    }
}
