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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.content.Context;
import android.os.Looper;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcCallManagerTest extends ImsStackTest {
    @Mock private IBaseContext mMockBaseContext;
    @Mock private CallStateListener mMockCallStateListener;
    @Mock private MtcCall mMockMtcCall;
    @Mock private Context mMockContext;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    private long mNativeCallId = 1;
    private int mSlotId0 = 0;

    private MessageExecutor mExecutor;
    private MtcCallManager mTestMtcCallManager;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        AppContext.init(mMockContext);
        mTestMtcCallManager = new MtcCallManager(mMockBaseContext);

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, mSlotId0);
        doReturn(mMockCarrierConfig).when(mMockConfigInterface).getCarrierConfig();
        doReturn(mSlotId0).when(mMockBaseContext).getSlotId();
        mExecutor = new MessageExecutor(Looper.myLooper());
        doReturn(mExecutor).when(mMockBaseContext).getExecutor();
        doReturn(mNativeCallId).when(mMockMtcCall).getNativeCallId();
        doReturn(IUMtcCall.CALLTYPE_VOIP).when(mMockMtcCall).getCallType();

        ImsStateStore.getCallState(mSlotId0).setState(MtcStateUtils.STATE_INACTIVE);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, mSlotId0);
        mTestMtcCallManager = null;
        mExecutor = null;
        super.tearDown();
        AppContext.deinit();
    }

    @Test
    public void testDispose() {
        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        processAllMessages();

        mTestMtcCallManager.dispose();

        assertNull(mTestMtcCallManager.getCallTracker());
        assertNull(mTestMtcCallManager.getCall(mNativeCallId));
        assertNull(mTestMtcCallManager.getPendingCall(mNativeCallId));
    }

    @Test
    public void testInit() {
        IServiceStateTracker sst = Mockito.mock(IServiceStateTracker.class);
        doReturn(mMockContext).when(mMockBaseContext).getContext();
        doReturn(Looper.myLooper()).when(mMockContext).getMainLooper();
        doReturn(sst).when(mMockBaseContext).getServiceStateTracker();
        doReturn(false).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOLTE_BOOL);
        doReturn(true).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOWIFI_BOOL);

        mTestMtcCallManager.init();

        assertNotNull(mTestMtcCallManager.getECallStateTracker());
    }

    @Test
    public void testClear() {
        IServiceStateTracker sst = Mockito.mock(IServiceStateTracker.class);
        doReturn(mMockContext).when(mMockBaseContext).getContext();
        doReturn(Looper.myLooper()).when(mMockContext).getMainLooper();
        doReturn(sst).when(mMockBaseContext).getServiceStateTracker();
        doReturn(false).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOLTE_BOOL);
        doReturn(true).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOWIFI_BOOL);

        mTestMtcCallManager.init();
        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        processAllMessages();

        assertNotNull(mTestMtcCallManager.getECallStateTracker());
        assertNotNull(mTestMtcCallManager.getCall(mNativeCallId));
        assertNotNull(mTestMtcCallManager.getPendingCall(mNativeCallId));

        mTestMtcCallManager.clear();

        assertNull(mTestMtcCallManager.getECallStateTracker());
        assertNull(mTestMtcCallManager.getCall(mNativeCallId));
        assertNull(mTestMtcCallManager.getPendingCall(mNativeCallId));
    }

    @Test
    public void testRemoveListener() {
        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallCreated(eq(mMockMtcCall));

        mTestMtcCallManager.removeListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        processAllMessages();

        verifyNoMoreInteractions(mMockCallStateListener);
    }

    @Test
    public void testHasCallHasEstablishedCall() {
        assertFalse(mTestMtcCallManager.hasCall());
        assertFalse(mTestMtcCallManager.hasEstablishedCall());

        mTestMtcCallManager.attachCall(mMockMtcCall);

        assertTrue(mTestMtcCallManager.hasCall());
        assertFalse(mTestMtcCallManager.hasEstablishedCall());

        mTestMtcCallManager.getCallTracker().updateCallState(
                null, CallTracker.CALL_EVENT_ESTABLISHED, null);

        assertTrue(mTestMtcCallManager.hasCall());
        assertFalse(mTestMtcCallManager.hasEstablishedCall());

        mTestMtcCallManager.getCallTracker().updateCallState(
                mTestMtcCallManager, CallTracker.CALL_EVENT_ESTABLISHED, null);

        assertTrue(mTestMtcCallManager.hasCall());
        assertFalse(mTestMtcCallManager.hasEstablishedCall());

        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ESTABLISHED, null);

        assertTrue(mTestMtcCallManager.hasCall());
        assertTrue(mTestMtcCallManager.hasEstablishedCall());
    }

    @Test
    public void testAttachCall() {
        mTestMtcCallManager.attachCall(null);

        assertNull(mTestMtcCallManager.getCall(mNativeCallId));

        mTestMtcCallManager.attachCall(mMockMtcCall);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallCreated(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        processAllMessages();

        assertNotNull(mTestMtcCallManager.getCall(mNativeCallId));
        verify(mMockCallStateListener, times(1)).onCallCreated(eq(mMockMtcCall));
    }

    @Test
    public void testAttachPreIncomingCall() {
        mTestMtcCallManager.attachPreIncomingCall(null);
        assertNull(mTestMtcCallManager.getPendingCall(mNativeCallId));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        processAllMessages();

        assertNotNull(mTestMtcCallManager.getPendingCall(mNativeCallId));
    }

    @Test
    public void testCloseAllCalls() {
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);

        mTestMtcCallManager.closeAllCalls(true);
        processAllMessages();

        verify(mMockMtcCall, times(2)).terminate(0, true);
        verify(mMockMtcCall, times(2)).close();
    }

    @Test
    public void testTerminateAllCalls() {
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);

        mTestMtcCallManager.terminateAllCalls();
        processAllMessages();

        verify(mMockMtcCall, times(2)).terminate(0, true);
        verify(mMockMtcCall, times(0)).close();
    }

    @Test
    public void testGetVacantCallIndex() {
        doReturn(1).when(mMockMtcCall).getCallIndex();
        mTestMtcCallManager.attachCall(mMockMtcCall);

        assertEquals(2, mTestMtcCallManager.getVacantCallIndex());
    }

    @Test
    public void testMtcCallTrackerGetActiveCalls() {
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ESTABLISHED, null);

        assertEquals(1, mTestMtcCallManager.getCallTracker().getActiveCalls());
    }

    @Test
    public void testMtcCallTrackerCallCreate() {
        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_CREATE, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallCreated(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallEstablishing() {
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ESTABLISHING, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallEstablishing(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ESTABLISHING, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallEstablishing(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallRinging() {
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_RINGING, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallRinging(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_RINGING, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallRinging(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_INACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallAccept() {
        mTestMtcCallManager.attachCall(mMockMtcCall);

        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ACCEPT, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallAccepted(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ACCEPT, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallAccepted(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallEstablished() {
        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_ESTABLISHED, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallEstablished(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallUpdated() {
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_UPDATED, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallUpdated(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_UPDATED, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallUpdated(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallTerminating() {
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_TERMINATING, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallTerminating(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_TERMINATING, null);
        processAllMessages();

        verify(mMockCallStateListener, times(1)).onCallTerminating(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_INACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallTerminated() {
        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_TERMINATED, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallTerminated(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_TERMINATED, null);
        processAllMessages();

        assertNotNull(mTestMtcCallManager.getCall(mNativeCallId));

        verify(mMockCallStateListener, times(1)).onCallTerminated(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallTerminatedPendingCall() {
        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_TERMINATED, null);
        processAllMessages();

        assertNotNull(mTestMtcCallManager.getPendingCall(mNativeCallId));

        verify(mMockCallStateListener, times(1)).onCallTerminated(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallDestroy() {
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_DESTROY, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallDestroyed(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_DESTROY, null);

        verify(mMockCallStateListener, times(0)).onCallDestroyed(eq(mMockMtcCall));

        mTestMtcCallManager.attachCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_DESTROY, null);
        processAllMessages();

        assertNull(mTestMtcCallManager.getCall(mNativeCallId));

        verify(mMockCallStateListener, times(1)).onCallDestroyed(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_INACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallDestroyPendingCall() {
        doReturn(false).when(mMockMtcCall).isOnPreIncoming();

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_DESTROY, null);
        processAllMessages();

        assertNull(mTestMtcCallManager.getPendingCall(mNativeCallId));

        verify(mMockCallStateListener, times(1)).onCallDestroyed(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_INACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }

    @Test
    public void testMtcCallTrackerCallIncomingReceived() {
        doReturn(false).when(mMockMtcCall).isOnPreIncoming();
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_INCOMING_RECEIVED, null);
        processAllMessages();

        verify(mMockCallStateListener, times(0)).onCallCreated(eq(mMockMtcCall));

        mTestMtcCallManager.addListener(mMockCallStateListener);
        mTestMtcCallManager.attachPreIncomingCall(mMockMtcCall);
        mTestMtcCallManager.getCallTracker().updateCallState(
                mMockMtcCall, CallTracker.CALL_EVENT_INCOMING_RECEIVED, null);
        processAllMessages();

        assertNull(mTestMtcCallManager.getPendingCall(mNativeCallId));
        assertNotNull(mTestMtcCallManager.getCall(mNativeCallId));

        verify(mMockCallStateListener, times(1)).onCallCreated(eq(mMockMtcCall));
        assertEquals(MtcStateUtils.STATE_ACTIVE, ImsStateStore.getCallState(mSlotId0).getState());
    }
}
