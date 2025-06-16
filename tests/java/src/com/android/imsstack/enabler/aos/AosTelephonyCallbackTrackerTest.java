/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.aos;

import static android.telephony.SecurityAlgorithmUpdate.CONNECTION_EVENT_NAS_SIGNALLING_LTE;
import static android.telephony.SecurityAlgorithmUpdate.CONNECTION_EVENT_VOLTE_SIP;
import static android.telephony.SecurityAlgorithmUpdate.SECURITY_ALGORITHM_EEA0;
import static android.telephony.SecurityAlgorithmUpdate.SECURITY_ALGORITHM_EEA2;
import static android.telephony.TelephonyManager.CALL_STATE_IDLE;
import static android.telephony.TelephonyManager.CALL_STATE_OFFHOOK;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;

import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.SecurityAlgorithmUpdate;
import android.telephony.TelephonyCallback;
import android.telephony.emergency.EmergencyNumber;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.aos.service.AosService;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.ArgumentMatcher;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AosTelephonyCallbackTrackerTest {
    @Mock private AosService mMockAosService;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SimInterface mSimInterface;

    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private AosTelephonyCallbackTracker mAosTelephonyCallbackTracker;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        when(mSimInterface.getSubId()).thenReturn(SUB_ID_1);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);

        AgentFactory.getInstance().setAgent(SimInterface.class, mSimInterface, SLOT0);
        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);

        AosFactory.getInstance().replaceService(SLOT0, mMockAosService);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT0);
        mAosTelephonyCallbackTracker = new AosTelephonyCallbackTracker(SLOT0);

        mAosTelephonyCallbackTracker.init();
    }

    @After
    public void tearDown() throws Exception {
        if (mAosTelephonyCallbackTracker != null) {
            mAosTelephonyCallbackTracker.cleanup();
            mAosTelephonyCallbackTracker = null;
        }

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        AosFactory.getInstance().replaceService(SLOT0, null);
        mTelephonyManagerProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mTestableLooper = null;
    }

    @Test
    @SmallTest
    public void registerTelephonyCbsWhenOutgoingEmergencyCallIfConfigured() {
        TelephonyCallback.OutgoingEmergencyCallListener outgoingEmcCallListener =
                captureOutgoingEmergencyCallListener();

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsEmergency
                .KEY_SUPPORT_EREG_WHEN_EATTACH_WITH_VALID_SIM_BOOL)))
                .thenReturn(true);

        EmergencyNumber num911 = new EmergencyNumber("911", "us", "",
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE, new ArrayList<String>(),
                EmergencyNumber.EMERGENCY_NUMBER_SOURCE_DATABASE,
                EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN);

        outgoingEmcCallListener.onOutgoingEmergencyCall(num911, SUB_ID_1);
        processAllMessages();

        verify(mTelephonyManagerProxy, times(1)).registerTelephonyCallback(
                any(Executor.class), eq(mAosTelephonyCallbackTracker.mEmergencyCallListener));
        verify(mTelephonyManagerProxy, times(1)).registerTelephonyCallback(
                any(Executor.class), eq(mAosTelephonyCallbackTracker.mCallStateListener));
        verify(mTelephonyManagerProxy, times(1)).registerTelephonyCallback(
                any(Executor.class), eq(mAosTelephonyCallbackTracker.mSecurityAlgorithmsListener));
    }

    @Test
    @SmallTest
    public void doNotRegisterTelephonyCbWhenOutgoingEmergencyCallIfNotConfigured() {
        TelephonyCallback.OutgoingEmergencyCallListener outgoingEmcCallListener =
                captureOutgoingEmergencyCallListener();

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsEmergency
                .KEY_SUPPORT_EREG_WHEN_EATTACH_WITH_VALID_SIM_BOOL)))
                .thenReturn(false);

        EmergencyNumber num911 = new EmergencyNumber("911", "us", "",
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE, new ArrayList<String>(),
                EmergencyNumber.EMERGENCY_NUMBER_SOURCE_DATABASE,
                EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN);

        outgoingEmcCallListener.onOutgoingEmergencyCall(num911, SUB_ID_1);
        processAllMessages();

        verify(mTelephonyManagerProxy, never()).registerTelephonyCallback(
                any(Executor.class), eq(mAosTelephonyCallbackTracker.mCallStateListener));
        verify(mTelephonyManagerProxy, never()).registerTelephonyCallback(
                any(Executor.class), eq(mAosTelephonyCallbackTracker.mSecurityAlgorithmsListener));
    }

    @Test
    @SmallTest
    public void doNotNotifyWhenSecurityAlgorithmsChangedIfNotNasEvent() {
        mAosTelephonyCallbackTracker.registerForSecurityAlgorithms();

        TelephonyCallback.SecurityAlgorithmsListener secuAlgoListener =
                captureSecurityAlgorithmsListener();

        SecurityAlgorithmUpdate update = new SecurityAlgorithmUpdate(
                CONNECTION_EVENT_VOLTE_SIP, SECURITY_ALGORITHM_EEA2,
                SECURITY_ALGORITHM_EEA2, false);

        secuAlgoListener.onSecurityAlgorithmsChanged(update);
        processAllMessages();

        verify(mMockAosService, never()).notifyNasSecurityAlgorithmChanged(anyBoolean());
    }

    @Test
    @SmallTest
    public void doNotifyWhenSecurityAlgorithmsChangedIfNasEventAndNullAlgo() {
        mAosTelephonyCallbackTracker.registerForSecurityAlgorithms();

        TelephonyCallback.SecurityAlgorithmsListener secuAlgoListener =
                captureSecurityAlgorithmsListener();

        SecurityAlgorithmUpdate update = new SecurityAlgorithmUpdate(
                CONNECTION_EVENT_NAS_SIGNALLING_LTE, SECURITY_ALGORITHM_EEA0,
                SECURITY_ALGORITHM_EEA0, false);

        secuAlgoListener.onSecurityAlgorithmsChanged(update);
        processAllMessages();

        verify(mMockAosService, times(1)).notifyNasSecurityAlgorithmChanged(eq(true));
    }

    @Test
    @SmallTest
    public void doNotifyWhenCallStateChangedIfIdleState() {
        mAosTelephonyCallbackTracker.registerForCallState();

        TelephonyCallback.CallStateListener callStateListener = captureCallStateListener();

        callStateListener.onCallStateChanged(CALL_STATE_OFFHOOK);
        callStateListener.onCallStateChanged(CALL_STATE_IDLE);

        verify(mMockAosService, times(1)).notifyNasSecurityAlgorithmChanged(eq(false));
    }

    @Test
    @SmallTest
    public void registerSecuAlgoCbWhenSimStateChangedWithUpdatedSubId() {
        mAosTelephonyCallbackTracker.registerForSecurityAlgorithms();
        Sim.Listener simListener = captureSimListener();

        when(mSimInterface.isSimLoadCompleted()).thenReturn(true);
        when(mSimInterface.getSubId()).thenReturn(SUB_ID_2);

        simListener.onSimStateChanged();

        verify(mTelephonyManagerProxy, times(1)).registerTelephonyCallback(
                any(Executor.class), eq(mAosTelephonyCallbackTracker.mSecurityAlgorithmsListener));
    }

    private TelephonyCallback.CallStateListener captureCallStateListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        ArgumentMatcher<TelephonyCallback> isCallStateListener = callback ->
                callback instanceof TelephonyCallback.CallStateListener;
        verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(), argThat(isCallStateListener));
        Mockito.verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(), captor.capture());
        return (TelephonyCallback.CallStateListener) captor.getValue();
    }

    private TelephonyCallback.OutgoingEmergencyCallListener
            captureOutgoingEmergencyCallListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        ArgumentMatcher<TelephonyCallback> isECallListener = callback ->
                callback instanceof TelephonyCallback.OutgoingEmergencyCallListener;
        verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(), argThat(isECallListener));
        Mockito.verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(), captor.capture());
        return (TelephonyCallback.OutgoingEmergencyCallListener) captor.getValue();
    }

    private TelephonyCallback.SecurityAlgorithmsListener captureSecurityAlgorithmsListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        ArgumentMatcher<TelephonyCallback> isSecuAlgoListener = callback ->
                callback instanceof TelephonyCallback.SecurityAlgorithmsListener;
        verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(), argThat(isSecuAlgoListener));
        Mockito.verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(), captor.capture());
        return (TelephonyCallback.SecurityAlgorithmsListener) captor.getValue();
    }

    private Sim.Listener captureSimListener() {
        ArgumentCaptor<Sim.Listener> captor = ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mSimInterface).addListener(captor.capture());
        return (Sim.Listener) captor.getValue();
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
