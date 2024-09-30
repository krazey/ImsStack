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

package com.android.imsstack.imsservice.mmtel.internal;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyObject;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Log;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;
import com.android.imsstack.imsservice.mmtel.internal.MergeProxy;
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
public class MergeProxyTest extends ImsStackTest {
    private static final String TAG = "MergeProxyTest";
    private MessageExecutor mMessageExecutor;
    private CallReasonInfo mFailInfo = null;
    private MtcCall.Listener mMtcCallListenerProxy = null;
    private MtcConference.Listener mMtcConferencelistenerProxy = null;
    private TestMergeProxy mMergeProxy = null;
    private boolean mIsHoldCalled = false;

    //Mocked Classes
    @Mock private CallInfo mMockCallInfo;
    @Mock private ConferenceProxy mMockConfProxy;
    @Mock private ImsCallContext mMockCallContext;
    @Mock private MediaInfo mMockMediaInfo;
    @Mock private MtcCall mMockBgCall;
    @Mock private MtcCall mMockFgCall;
    @Mock private MtcCall mMockMtcCall;
    @Mock private MtcCall.Listener mMockMtcCallListenerProxy;
    @Mock private MtcConference mMockMtcConference;
    @Mock private MtcConference.Listener mMockMtcConferencelistenerProxy;
    @Mock private SuppInfo mMockSuppInfo;
    @Mock private UsersInfo mMockUsersInfo;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;

    public static final int STATE_IDLE = 0;
    public static final int STATE_HOLDING = 1;
    public static final int STATE_SWAP_HOLDING = 2;
    public static final int STATE_UNHOLDING = 3;
    public static final int STATE_MERGE_WAITING = 4;
    public static final int STATE_MERGED = 5;
    public static final int STATE_CONFERENCE_EXTENDING = 6;
    public static final int STATE_CONFERENCE_EXTENDED = 7;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        mMessageExecutor = new MessageExecutor(Looper.myLooper());
        when(mMockCallContext.getExecutor()).thenReturn(mMessageExecutor);

        mMergeProxy = new TestMergeProxy(mMockCallContext, mMockFgCall, mMockBgCall,
                mMessageExecutor);
        mMtcCallListenerProxy = mMergeProxy.getMtcCallListener();
        mMtcConferencelistenerProxy = mMergeProxy.getMtcConferenceListener();
        mFailInfo = new CallReasonInfo();
        mockCarrierConfig();
    }

    @After
    public void tearDown()throws Exception {
        super.tearDown();
        int slotId = 0;
        mMergeProxy = null;
        mFailInfo = null;
        mMtcCallListenerProxy = null;
        mMtcConferencelistenerProxy = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, slotId);
    }

    @Test
    public void testMtcCallListenerCallbacks() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);

        /* callbacks */
        mMtcCallListenerProxy.onCallHoldReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onCallHoldReceived(mMockFgCall, mMockCallInfo,
                mMockMediaInfo, mMockSuppInfo);

        mMtcCallListenerProxy.onCallResumeReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onCallResumeReceived(mMockFgCall, mMockCallInfo,
                mMockMediaInfo, mMockSuppInfo);

        mMtcCallListenerProxy.onCallAutoUpdated(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onCallAutoUpdated(mMockFgCall, mMockCallInfo,
                mMockMediaInfo, mMockSuppInfo);

        mMtcCallListenerProxy.onCallUpdateReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onCallUpdateReceived(mMockFgCall, mMockCallInfo,
                mMockMediaInfo, mMockSuppInfo);

        mMtcCallListenerProxy.onCallInfoUpdated(mMockFgCall, 0, "", 0, false);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onCallInfoUpdated(mMockFgCall, 0, "", 0, false);

        mMtcCallListenerProxy.onAudioSessionOpened(mMockFgCall);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onAudioSessionOpened(mMockFgCall);

        mMtcCallListenerProxy.onCallQualityChanged(mMockFgCall, null);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onCallQualityChanged(mMockFgCall, null);

        mMtcCallListenerProxy.onAudioSessionClosed(mMockFgCall);
        processAllMessages();
        verify(mMockMtcCallListenerProxy).onAudioSessionClosed(mMockFgCall);
    }

    @Test
    public void testMtcConferenceListenerCallbacks() {
        //If not ConferenceCall it will return
        mMtcConferencelistenerProxy.onCallMerged(mMockMtcConference, mMockCallInfo, mMockMediaInfo,
                 mMockSuppInfo, mMockUsersInfo);
        assertEquals(mMockConfProxy.getState(), STATE_IDLE);

        //If not ConferenceCall it will return
        mMtcConferencelistenerProxy.onCallMergeFailed(mMockMtcConference, mFailInfo);
        verify(mMockMtcConferencelistenerProxy, never()).onCallMergeFailed(mMockMtcConference,
                mFailInfo);

        //If not ConferenceCall it will return
        mMtcConferencelistenerProxy.onCallConferenceStateUpdated(mMockMtcConference,
                mMockUsersInfo);
        verify(mMockConfProxy, never()).getConferenceParticipants();

        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        when(mMockMtcConference.isSameCall(mMockMtcConference)).thenReturn(true);
        when(mMockConfProxy.getConferenceCall()).thenReturn(mMockMtcCall);
        MtcConference mtcConferenceFg = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mtcConferenceFg);

        mMtcConferencelistenerProxy.onCallMergeFailed(mMockMtcConference, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mtcConferenceFg, mFailInfo);

        mMergeProxy.setForegroundCallRecoveryForTest(true);
        when(mMockFgCall.isOnHold()).thenReturn(true);
        mMtcConferencelistenerProxy.onCallMergeFailed(mMockMtcConference, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy, never()).onCallMergeFailed(mMockMtcConference,
                mFailInfo);

        //NotifySessionMergeStarted and NotifySessionMerged()
        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_CALL_MERGEABLE_ON_CONFERENCE_ON_HOLD_BOOL))
                .thenReturn(false);
        mMtcConferencelistenerProxy.onCallMerged(mMockMtcConference, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo, mMockUsersInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeStarted(mtcConferenceFg,
                mMockMtcConference, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        verify(mMockMtcConferencelistenerProxy).onCallMerged(mtcConferenceFg, mMockCallInfo,
                mMockMediaInfo, mMockSuppInfo, mMockUsersInfo);

        //setConferenceParticipants(usersInfo);
        mMtcConferencelistenerProxy.onCallConferenceStateUpdated(mMockMtcConference,
                mMockUsersInfo);
        assertEquals(mMockUsersInfo, mMergeProxy.getConferenceParticipants());

        //setBackgroundCallRecovery(true);
        mMockMtcCall = mMockBgCall;
        when(mMockConfProxy.getConferenceCall()).thenReturn(mMockMtcCall);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        mMtcConferencelistenerProxy.onCallMergeFailed(mMockMtcConference, mFailInfo);
        assertTrue(mMergeProxy.isBackgroundCallRecoveryRequired());
    }

    @Test
    public void testConferenceExtension() {
        MtcCall mtcCall = Mockito.mock(MtcCall.class);

        assertTrue(mMergeProxy.isConferenceForCallMerge());
        assertFalse(mMergeProxy.isConferenceExtensionRequestor(mtcCall));
        assertTrue(mMergeProxy.isConferenceExtensionRequestor(mMockFgCall));

        mtcCall = null;
    }

    @Test
    public void testMergeProxyCleanup() {
        //to call terminateConferenceSession()
        mMergeProxy.setStateForTest(STATE_MERGE_WAITING);
        mMergeProxy.setConferenceCallForTest(mMockMtcCall);
        mMergeProxy.abort();
        mMergeProxy.dispose();
        verify(mMockMtcCall).terminate(CallReasonInfo.CODE_USER_TERMINATED);
        verify(mMockMtcCall).close();
    }

    @Test
    public void testStartInternal() {
        int slotId = 0;
        mMergeProxy.setInitialConferenceExtensionForTest(true);
        assertTrue(mMergeProxy.startInternal(true));
        verify(mMockMtcCall).setListener(mMtcCallListenerProxy);
        verify(mMockFgCall).setListener(mMtcCallListenerProxy);
        verify(mMockBgCall).setListener(mMtcCallListenerProxy);
        assertEquals(mMergeProxy.getState(), STATE_HOLDING);
        assertTrue(mIsHoldCalled);
        mIsHoldCalled = false;

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_CALL_MERGEABLE_ON_CONFERENCE_ON_HOLD_BOOL))
                .thenReturn(false)
                .thenReturn(true);
        clearInvocations(mMockFgCall);
        clearInvocations(mMockBgCall);
        mMergeProxy.setInitialConferenceExtensionForTest(false);
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        assertTrue(mMergeProxy.startInternal(false));
        verify(mMockFgCall).setListener(mMtcCallListenerProxy);
        verify(mMockBgCall).setListener(mMtcCallListenerProxy);
        assertEquals(mMergeProxy.getState(), STATE_SWAP_HOLDING);
        assertTrue(mIsHoldCalled);

        mIsHoldCalled = false;
        assertTrue(mMergeProxy.startInternal(false));
        verify(mMockFgCall, times(2)).setListener(mMtcCallListenerProxy);
        verify(mMockBgCall, times(2)).setListener(mMtcCallListenerProxy);
        assertEquals(mMergeProxy.getState(), STATE_MERGE_WAITING);
        assertFalse(mIsHoldCalled);

        mMockMtcCall = null;
        assertFalse(mMergeProxy.startInternal(true));
    }

    private void mockCarrierConfig() {
        int slotId = 0;
        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, slotId);
    }

    @Test
    public void testOnCallTerminatedBackgroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        mMergeProxy.setStateForTest(STATE_HOLDING);
        mMtcCallListenerProxy.onCallTerminated(mMockBgCall, mFailInfo);
        assertTrue(mMergeProxy.isForegroundCallRecoveryRequired());

        MtcCallUtils mtcCallUtils = Mockito.mock(MtcCallUtils.class);
        when(mMockFgCall.isInCall()).thenReturn(true);
        mMergeProxy.setStateForTest(STATE_MERGE_WAITING);
        mMergeProxy.setInitialConferenceExtensionForTest(true);
        mMtcCallListenerProxy.onCallTerminated(mMockBgCall, mFailInfo);
        assertTrue(mMergeProxy.isForegroundCallRecoveryRequired());
        verify(mtcCallUtils).isCallTerminatedByJoiningConference(CallReasonInfo.CODE_UNSPECIFIED);
    }

    @Test
    public void testOnCallTerminatedForegroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);

        mMergeProxy.setStateForTest(STATE_MERGE_WAITING);
        mMtcCallListenerProxy.onCallTerminated(mMockFgCall, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy, times(2)).onCallMergeFailed(anyObject(),
                anyObject());

        mMergeProxy.setStateForTest(STATE_HOLDING);
        clearInvocations(mMockMtcConferencelistenerProxy);
        mMtcCallListenerProxy.onCallTerminated(mMockFgCall, mFailInfo);
        assertFalse(mMergeProxy.isForegroundCallRecoveryRequired());
    }

    @Test
    public void testOnCallTerminatedConferenceCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        mMergeProxy.setInitialConferenceExtensionForTest(false);
        mMergeProxy.setStateForTest(STATE_MERGE_WAITING);
        mMtcCallListenerProxy.onCallTerminated(mMockMtcCall, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy, times(2)).onCallMergeFailed(anyObject(),
                anyObject());
    }

    @Test
    public void testOnCallHeldBackgroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        mMergeProxy.setBackgroundCallRecoveryForTest(true);
        when(mMockFgCall.isTerminated()).thenReturn(true);
        //notifySessionMergeFailed()
        mMtcCallListenerProxy.onCallHeld(mMockBgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy, times(2)).onCallMergeFailed(anyObject(),
                anyObject());
        assertFalse(mMergeProxy.isBackgroundCallRecoveryRequired());
    }

    @Test
    public void testOnCallHeldForegroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        mMergeProxy.setStateForTest(STATE_HOLDING);
        MtcConference mtcConferenceBg = Mockito.mock(MtcConference.class);
        MtcConference mtcConferenceFg = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mtcConferenceFg);
        when(mMockBgCall.getConferenceInterface()).thenReturn(mtcConferenceBg);

        // executeMerge()
        mMtcCallListenerProxy.onCallHeld(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallProxyMerge(mMockMtcConference,
                mtcConferenceFg, mtcConferenceBg);
        assertEquals(mMergeProxy.getState(), STATE_MERGE_WAITING);

        //executeUnhold(call) for hold state
        // To control mCallRecoveryRequired variable
        mMergeProxy.setForegroundCallRecoveryForTest(true);
        mMergeProxy.setStateForTest(STATE_HOLDING);
        mMtcCallListenerProxy.onCallHeld(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        assertEquals(mMergeProxy.getState(), STATE_HOLDING);

        //executeUnhold(call) for swap hold state
        mMergeProxy.setForegroundCallRecoveryForTest(true);
        mMergeProxy.setStateForTest(STATE_SWAP_HOLDING);
        mMtcCallListenerProxy.onCallHeld(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        assertEquals(mMergeProxy.getState(), STATE_SWAP_HOLDING);

        // executeUnhold(mBackgroundCall)
        mMergeProxy.setStateForTest(STATE_IDLE);
        mMtcCallListenerProxy.onCallHeld(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        assertEquals(mMergeProxy.getState(), STATE_UNHOLDING);
    }

    @Test
    public void testOnCallHoldFailedBackgroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        mMergeProxy.setBackgroundCallRecoveryForTest(true);
        when(mMockFgCall.isTerminated()).thenReturn(true);
        when(mMockBgCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        mMtcCallListenerProxy.onCallHoldFailed(mMockBgCall, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mMockMtcConference, mFailInfo);
        assertFalse(mMergeProxy.isBackgroundCallRecoveryRequired());
    }

    @Test
    public void testOnCallHoldFailedForegroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        mMergeProxy.setForegroundCallRecoveryForTest(true);
        mMtcCallListenerProxy.onCallHoldFailed(mMockFgCall, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mMockMtcConference, mFailInfo);
        clearInvocations(mMockMtcConferencelistenerProxy);

        //added to get mMergeFailedReason value as not null
        mMergeProxy.setStateForTest(STATE_UNHOLDING);
        mMtcCallListenerProxy.onCallResumeFailed(mMockBgCall, mFailInfo);

        mMtcCallListenerProxy.onCallHoldFailed(mMockFgCall, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mMockMtcConference, mFailInfo);
    }

    @Test
    public void testOnCallResumeFailed() {
        mMergeProxy.setForegroundCallRecoveryForTest(true);
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);

        //notifySessionMergeFailed() for background case
        when(mMockBgCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        mMtcCallListenerProxy.onCallResumeFailed(mMockBgCall, mFailInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mMockMtcConference, mFailInfo);

        //executeUnhold
        mMergeProxy.setStateForTest(STATE_UNHOLDING);
        mMtcCallListenerProxy.onCallResumeFailed(mMockBgCall, mFailInfo);
        assertTrue(mMergeProxy.isForegroundCallRecoveryRequired());

        clearInvocations(mMockMtcConferencelistenerProxy);
        mMergeProxy.setForegroundCallRecoveryForTest(true);
        //notifySessionMergeFailed for foreground case
        mMergeProxy.setStateForTest(STATE_IDLE);
        mMtcCallListenerProxy.onCallResumeFailed(mMockFgCall, mFailInfo);
        assertFalse(mMergeProxy.isForegroundCallRecoveryRequired());
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mMockMtcConference, mFailInfo);
    }

    @Test
    public void testOnCallResumedBackgroundCall() {
        MtcConference mtcConferenceBg = Mockito.mock(MtcConference.class);
        MtcConference mtcConferenceFg = Mockito.mock(MtcConference.class);
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mtcConferenceFg);
        when(mMockBgCall.getConferenceInterface()).thenReturn(mtcConferenceBg);
        mMergeProxy.setStateForTest(STATE_UNHOLDING);
        //executeMerge()
        mMtcCallListenerProxy.onCallResumed(mMockBgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallProxyMerge(mMockMtcConference,
                mtcConferenceFg, mtcConferenceBg);
        assertEquals(mMergeProxy.getState(), STATE_MERGE_WAITING);
    }

    @Test
    public void testOnCallResumedForegroundCall() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);

        //added to get mMergeFailedReason value as not null
        mMergeProxy.setStateForTest(STATE_UNHOLDING);
        mMtcCallListenerProxy.onCallResumeFailed(mMockBgCall, mFailInfo);

        when(mMockFgCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        mMergeProxy.setStateForTest(STATE_IDLE);
        mMergeProxy.setForegroundCallRecoveryForTest(true);
        //notifySessionMergeFailed()
        mMtcCallListenerProxy.onCallResumed(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        assertFalse(mMergeProxy.isBackgroundCallRecoveryRequired());
        processAllMessages();
        verify(mMockMtcConferencelistenerProxy).onCallMergeFailed(mMockMtcConference, mFailInfo);
    }

    @Test
    public void testNotifySessionHoldReceivedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);

        doThrow(mockRuntimeException).when(mMockMtcCallListenerProxy)
            .onCallHoldReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        mMtcCallListenerProxy.onCallHoldReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        verify(mMockConfProxy).loge("notifySessionHoldReceived", mockRuntimeException);
    }

    @Test
    public void testNotifySessionResumeReceivedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);

        doThrow(mockRuntimeException).when(mMockMtcCallListenerProxy)
            .onCallResumeReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        mMtcCallListenerProxy.onCallResumeReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        verify(mMockConfProxy).loge("notifySessionResumeReceived", mockRuntimeException);
    }

    @Test
    public void testNotifySessionAutoUpdatedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);

        doThrow(mockRuntimeException).when(mMockMtcCallListenerProxy)
            .onCallAutoUpdated(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        mMtcCallListenerProxy.onCallAutoUpdated(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        verify(mMockConfProxy).loge("notifySessionAutoUpdated", mockRuntimeException);
    }

    @Test
    public void testNotifySessionUpdateReceivedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);

        doThrow(mockRuntimeException).when(mMockMtcCallListenerProxy)
            .onCallUpdateReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo, mMockSuppInfo);
        mMtcCallListenerProxy.onCallUpdateReceived(mMockFgCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        verify(mMockConfProxy).loge("notifySessionUpdateReceived", mockRuntimeException);
    }

    @Test
    public void testNotifySessionInfoUpdatedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);

        doThrow(mockRuntimeException).when(mMockMtcCallListenerProxy)
            .onCallInfoUpdated(mMockFgCall, 0, "", 0, false);
        mMtcCallListenerProxy.onCallInfoUpdated(mMockFgCall, 0, "", 0, false);
        verify(mMockConfProxy).loge("notifySessionInfoUpdated", mockRuntimeException);
    }

    @Test
    public void testNotifySessionMergedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);

        when(mMockMtcConference.isSameCall(mMockMtcConference)).thenReturn(true);
        when(mMockConfProxy.getConferenceCall()).thenReturn(mMockMtcCall);
        MtcConference mtcConferenceFg = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mtcConferenceFg);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);

        doThrow(mockRuntimeException).when(mMockMtcConferencelistenerProxy)
                .onCallMerged(mtcConferenceFg, mMockCallInfo, mMockMediaInfo, mMockSuppInfo,
                    mMockUsersInfo);
        mMtcConferencelistenerProxy.onCallMerged(mMockMtcConference, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo, mMockUsersInfo);
        verify(mMockConfProxy).loge("notifySessionMerged", mockRuntimeException);
    }

    @Test
    public void testNotifySessionMergeFailedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);
        when(mMockMtcConference.isSameCall(mMockMtcConference)).thenReturn(true);
        when(mMockConfProxy.getConferenceCall()).thenReturn(mMockMtcCall);
        MtcConference mtcConferenceFg = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mtcConferenceFg);

        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(mMockMtcConferencelistenerProxy)
            .onCallMergeFailed(mtcConferenceFg, mFailInfo);
        mMtcConferencelistenerProxy.onCallMergeFailed(mMockMtcConference, mFailInfo);
        verify(mMockConfProxy).loge("notifySessionMergeFailed", mockRuntimeException);
    }

    @Test
    public void testNotifySessionMergeStartedException() {
        mMergeProxy.addListener(mMockMtcCallListenerProxy, mMockMtcConferencelistenerProxy);

        when(mMockMtcConference.isSameCall(mMockMtcConference)).thenReturn(true);
        when(mMockConfProxy.getConferenceCall()).thenReturn(mMockMtcCall);
        MtcConference mtcConferenceFg = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mMockMtcConference);
        when(mMockFgCall.getConferenceInterface()).thenReturn(mtcConferenceFg);

        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(mMockMtcConferencelistenerProxy)
                .onCallMergeStarted(mtcConferenceFg, mMockMtcConference, mMockCallInfo,
                    mMockMediaInfo, mMockSuppInfo);
        mMtcConferencelistenerProxy.onCallMerged(mMockMtcConference, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo, mMockUsersInfo);
        verify(mMockConfProxy).loge("notifySessionMergeStarted", mockRuntimeException);
    }

    private class TestMergeProxy extends MergeProxy {
        TestMergeProxy(ICallContext callContext, MtcCall fgCall, MtcCall bgCall,
                MessageExecutor exec) {
                super(callContext, fgCall, bgCall);
        }

        public MtcCall getConferenceCall() {
            Log.d(TAG, "getConferenceCall");
            return mMockMtcCall;
        }

        protected void executeHold(final MtcCall call) {
            Log.d(TAG, "executeHold");
            mIsHoldCalled = true;
        }

        public void setInitialConferenceExtensionForTest(boolean initialConfExt) {
            setInitialConferenceExtension(initialConfExt);
        }

        public void setStateForTest(int state) {
            setState(state);
        }

        public void setBackgroundCallRecoveryForTest(boolean flag) {
            setBackgroundCallRecovery(flag);
        }

        public void setForegroundCallRecoveryForTest(boolean flag) {
            setForegroundCallRecovery(flag);
        }

        public void setConferenceCallForTest(MtcCall call) {
            setConferenceCall(call);
        }
    }
}
