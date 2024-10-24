/*
 * Copyright (C) 2024 The Android Open Source Project
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

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.enabler.aos.IAosInfo.EmcCallbackModeState;
import com.android.imsstack.enabler.aos.IAosInfo.EmcCallbackModeType;
import com.android.imsstack.enabler.aos.service.AosService;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.time.Duration;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AosEmergencyCallbackModeTrackerTest {
    @Mock private AosService mMockAosService;
    @Mock private SimInterface mSimInterface;

    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private AosEmergencyCallbackModeTracker mAosEmergencyTracker;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        when(mSimInterface.getSubId()).thenReturn(SUB_ID_1);
        AgentFactory.getInstance().setAgent(SimInterface.class, mSimInterface, SLOT0);
        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);

        AosFactory.getInstance().replaceService(SLOT0, mMockAosService);
        mAosEmergencyTracker = new AosEmergencyCallbackModeTracker(SLOT0);

        mAosEmergencyTracker.init();
    }

    @After
    public void tearDown() throws Exception {
        if (mAosEmergencyTracker != null) {
            mAosEmergencyTracker.cleanup();
            mAosEmergencyTracker = null;
        }

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        mSimInterface = null;
        AosFactory.getInstance().replaceService(SLOT0, null);
        mTelephonyManagerProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mTestableLooper = null;
    }

    @Test
    public void testOnCallbackModeStarted() {
        TelephonyCallback.EmergencyCallbackModeListener emergencyCbmListener =
                captureEmergencyCbmListener();

        emergencyCbmListener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMillis(300000L), SUB_ID_1);

        verify(mMockAosService).notifyEmcCallbackModeChanged(
                    eq(EmcCallbackModeType.CALL), eq(EmcCallbackModeState.START), eq(300L));
    }

    @Test
    public void testOnCallbackModeRestarted() {
        TelephonyCallback.EmergencyCallbackModeListener emergencyCbmListener =
                captureEmergencyCbmListener();

        emergencyCbmListener.onCallbackModeRestarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_SMS,
                Duration.ofMillis(200000L), SUB_ID_1);

        verify(mMockAosService).notifyEmcCallbackModeChanged(
                    eq(EmcCallbackModeType.SMS), eq(EmcCallbackModeState.START), eq(200L));
    }

    @Test
    public void testOnCallbackModeStopped() {
        TelephonyCallback.EmergencyCallbackModeListener emergencyCbmListener =
                captureEmergencyCbmListener();

        emergencyCbmListener.onCallbackModeStopped(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                TelephonyManager.STOP_REASON_EMERGENCY_SMS_SENT, SUB_ID_1);

        verify(mMockAosService).notifyEmcCallbackModeChanged(
                    eq(EmcCallbackModeType.CALL), eq(EmcCallbackModeState.STOP_BY_EMC), eq(0L));
    }

    @Test
    public void testOnCallbackModeStopped_byUserAction() {
        TelephonyCallback.EmergencyCallbackModeListener emergencyCbmListener =
                captureEmergencyCbmListener();

        emergencyCbmListener.onCallbackModeStopped(TelephonyManager.EMERGENCY_CALLBACK_MODE_SMS,
                TelephonyManager.STOP_REASON_USER_ACTION, SUB_ID_1);

        verify(mMockAosService).notifyEmcCallbackModeChanged(
                    eq(EmcCallbackModeType.SMS), eq(EmcCallbackModeState.STOP), eq(0L));
    }

    @Test
    public void testOnCallbackModeStarted_notMatchedSubId() {
        TelephonyCallback.EmergencyCallbackModeListener emergencyCbmListener =
                captureEmergencyCbmListener();

        emergencyCbmListener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMillis(300000L), SUB_ID_2);

        verify(mMockAosService, never()).notifyEmcCallbackModeChanged(
                    eq(EmcCallbackModeType.CALL), eq(EmcCallbackModeState.START), eq(300L));
    }

    private TelephonyCallback.EmergencyCallbackModeListener captureEmergencyCbmListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy).registerTelephonyCallback(any(), captor.capture());
        return (TelephonyCallback.EmergencyCallbackModeListener) captor.getValue();
    }
}
