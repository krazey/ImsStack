/*
 * Copyright (C) 2026 The Android Open Source Project
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
package com.android.imsstack.core.agents;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.EmergencyStateInterface.EmergencyCallbackModeState;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.time.Duration;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class EmergencyStateAgentTest {
    @Mock private EmergencyStateInterface.EmergencyStateListener mEmergencyStateListener;
    @Mock private TelephonyManagerProxy mTelephonyManagerProxy;
    @Mock private SimInterface mSimInterface;

    private TestAppContext mTestAppContext;
    private TestableLooper mTestableLooper;
    private EmergencyStateAgent mEmergencyStateAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        AgentFactory.getInstance()
                .setAgent(SimInterface.class, mSimInterface, TestAppContext.SLOT0);

        mEmergencyStateAgent = new EmergencyStateAgent(TestAppContext.SLOT0);
        mEmergencyStateAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mEmergencyStateAgent != null) {
            mEmergencyStateAgent.cleanup();
            mEmergencyStateAgent = null;
        }

        AgentFactory.getInstance().setAgent(SimInterface.class, null, TestAppContext.SLOT0);
        mEmergencyStateListener = null;
        mTelephonyManagerProxy = null;
        mSimInterface = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mTestableLooper = null;
    }

    @Test
    @SmallTest
    public void testAddListener() {
        mEmergencyStateAgent.addListener(mEmergencyStateListener);

        TelephonyCallback.DomainSelectionEmergencyModeListener listener =
                getDomainSelectionEmergencyModeListener();

        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();

        verify(mEmergencyStateListener).onEmergencyModeChanged(
                eq(TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL), eq(true));

        listener.onDomainSelectionEmergencyModeExited(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();

        verify(mEmergencyStateListener).onEmergencyModeChanged(
                eq(TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL), eq(false));

        mEmergencyStateAgent.removeListener(mEmergencyStateListener);

        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();

        verifyNoMoreInteractions(mEmergencyStateListener);
    }

    @Test
    @SmallTest
    public void testEmergencyModeEnteredForDifferentSlotId() {
        mEmergencyStateAgent.addListener(mEmergencyStateListener);

        TelephonyCallback.DomainSelectionEmergencyModeListener listener =
                getDomainSelectionEmergencyModeListener();

        // Calling onDomainSelectionEmergencyModeEntered with a different slotId should be ignored.
        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0 + 1, TestAppContext.SUB_ID_1);
        processAllMessages();

        verify(mEmergencyStateListener, never()).onEmergencyModeChanged(anyInt(), anyBoolean());
        assertFalse(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL));
    }

    @Test
    @SmallTest
    public void testEmergencyModeExitedForDifferentSlotId() {
        mEmergencyStateAgent.addListener(mEmergencyStateListener);

        TelephonyCallback.DomainSelectionEmergencyModeListener listener =
                getDomainSelectionEmergencyModeListener();

        // First, enter emergency mode.
        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();
        assertTrue(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL));

        // Calling onDomainSelectionEmergencyModeExited with a different slotId should be ignored.
        listener.onDomainSelectionEmergencyModeExited(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0 + 1, TestAppContext.SUB_ID_1);
        processAllMessages();

        // The state should not change and listener for exit shouldn't be called.
        verify(mEmergencyStateListener, never()).onEmergencyModeChanged(anyInt(), eq(false));
        assertTrue(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL));
    }

    @Test
    @SmallTest
    public void testNotifyEmergencyModeChanged() {
        mEmergencyStateAgent.addListener(mEmergencyStateListener);

        TelephonyCallback.DomainSelectionEmergencyModeListener listener =
                getDomainSelectionEmergencyModeListener();

        // Enter emergency mode.
        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();
        verify(mEmergencyStateListener, times(1)).onEmergencyModeChanged(
                eq(TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL), eq(true));

        // Try to enter again - listener should not be called.
        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();
        verifyNoMoreInteractions(mEmergencyStateListener);

        // Exit emergency mode.
        listener.onDomainSelectionEmergencyModeExited(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();
        verify(mEmergencyStateListener, times(1)).onEmergencyModeChanged(
                eq(TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL), eq(false));

        // Try to exit again - listener should not be called.
        listener.onDomainSelectionEmergencyModeExited(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();
        verifyNoMoreInteractions(mEmergencyStateListener);
    }

    @Test
    @SmallTest
    public void testIsInEmergencyMode() {
        TelephonyCallback.DomainSelectionEmergencyModeListener listener =
                getDomainSelectionEmergencyModeListener();
        assertFalse(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL));

        listener.onDomainSelectionEmergencyModeEntered(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();

        assertTrue(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL));
        assertFalse(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_SMS));

        listener.onDomainSelectionEmergencyModeExited(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL,
                TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        processAllMessages();

        assertFalse(mEmergencyStateAgent.isInEmergencyMode(
                TelephonyManager.DOMAIN_SELECTION_EMERGENCY_TYPE_CALL));
    }

    @Test
    @SmallTest
    public void testSimStateChanged() {
        mEmergencyStateAgent.addListener(mEmergencyStateListener);
        ArgumentCaptor<Sim.Listener> captor = ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mSimInterface).addListener(captor.capture());
        Sim.Listener simListener = captor.getValue();
        TelephonyCallback.EmergencyCallbackModeListener listener =
                getEmergencyCallbackModeListener();

        // Initial subId is SUB_ID_1 for SLOT0
        listener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMinutes(5), TestAppContext.SUB_ID_1);
        processAllMessages();
        verify(mEmergencyStateListener).onEmergencyCallbackModeChanged(
                eq(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL),
                eq(EmergencyCallbackModeState.START), eq(300L));

        // Event for another subId should be ignored.
        listener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMinutes(5), TestAppContext.SUB_ID_2);
        processAllMessages();
        verifyNoMoreInteractions(mEmergencyStateListener);

        // Change subId
        when(mSimInterface.getSubId()).thenReturn(TestAppContext.SUB_ID_2);
        simListener.onSimStateChanged();
        processAllMessages();

        // Event for old subId should be ignored now.
        listener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMinutes(5), TestAppContext.SUB_ID_1);
        processAllMessages();
        verifyNoMoreInteractions(mEmergencyStateListener);

        // Event for new subId should be processed.
        listener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMinutes(5), TestAppContext.SUB_ID_2);
        processAllMessages();
        verify(mEmergencyStateListener, times(2)).onEmergencyCallbackModeChanged(
                eq(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL),
                eq(EmergencyCallbackModeState.START), eq(300L));
    }

    @Test
    @SmallTest
    public void testEmergencyCallbackModeChanged() {
        mEmergencyStateAgent.addListener(mEmergencyStateListener);

        TelephonyCallback.EmergencyCallbackModeListener listener =
                getEmergencyCallbackModeListener();

        listener.onCallbackModeStarted(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                Duration.ofMinutes(5), TestAppContext.SUB_ID_1);
        processAllMessages();

        verify(mEmergencyStateListener).onEmergencyCallbackModeChanged(
                eq(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL),
                eq(EmergencyCallbackModeState.START), eq(300L));

        listener.onCallbackModeStopped(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL,
                TelephonyManager.STOP_REASON_EMERGENCY_SMS_SENT, TestAppContext.SUB_ID_1);
        processAllMessages();

        verify(mEmergencyStateListener).onEmergencyCallbackModeChanged(
                eq(TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL),
                eq(EmergencyCallbackModeState.STOP_BY_EMERGENCY), eq(0L));
    }

    private TelephonyCallback.DomainSelectionEmergencyModeListener
            getDomainSelectionEmergencyModeListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(Executor.class), captor.capture());
        List<TelephonyCallback> callbacks = captor.getAllValues();
        for (TelephonyCallback callback : callbacks) {
            if (callback instanceof TelephonyCallback.DomainSelectionEmergencyModeListener) {
                return (TelephonyCallback.DomainSelectionEmergencyModeListener) callback;
            }
        }
        return null;
    }

    private TelephonyCallback.EmergencyCallbackModeListener getEmergencyCallbackModeListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, atLeastOnce()).registerTelephonyCallback(
                any(Executor.class), captor.capture());
        List<TelephonyCallback> callbacks = captor.getAllValues();
        for (TelephonyCallback callback : callbacks) {
            if (callback instanceof TelephonyCallback.EmergencyCallbackModeListener) {
                return (TelephonyCallback.EmergencyCallbackModeListener) callback;
            }
        }
        return null;
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
