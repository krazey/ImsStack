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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.util.Log;

import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceExtensionProxy;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ConferenceExtensionProxyTest {
    private static final String LOG_TAG = "ConferenceExtensionProxyTest";
    private TestConferenceProxy mConfExtProxy;
    private ImsCallContext mMockCallContext;
    private MtcCall mMockMtcCall;
    private CallInfo mMockCallInfo;
    private MediaInfo mMockMediaInfo;
    private SuppInfo mMockSuppInfo;
    private CallReasonInfo mMockCallReasonInfo;
    private MtcCall.Listener mMtcCallListenerProxy;
    private MtcConference.Listener mMtcConfListenerProxy;
    private TestCallListener mCallListener;
    private TestConferenceListener mConfListener;

    private String[] mParticipants = {"1234", "5678"};
    private boolean mIscallBackCalled = false;
    private boolean mIsHoldCalled = false;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mMockMtcCall = Mockito.mock(MtcCall.class);
        mMockCallInfo = Mockito.mock(CallInfo.class);
        mMockMediaInfo = Mockito.mock(MediaInfo.class);
        mMockSuppInfo = Mockito.mock(SuppInfo.class);
        mMockCallReasonInfo = new CallReasonInfo();
        mMockCallContext = Mockito.mock(ImsCallContext.class);

        mConfExtProxy = new TestConferenceProxy(mMockCallContext, mMockMtcCall,
                mParticipants);
        mMtcCallListenerProxy = mConfExtProxy.getMtcCallListener();
        mMtcConfListenerProxy = mConfExtProxy.getMtcConferenceListener();
        mCallListener = new TestCallListener();
        mConfListener = new TestConferenceListener();
    }

    @After
    public void tearDown() throws Exception {
        mConfExtProxy.setStateForTest(ConferenceProxy.STATE_IDLE);
        mConfExtProxy.dispose();
        mConfExtProxy = null;
        mIscallBackCalled = false;
    }

    @Test
    public void testAbort() {
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mConfExtProxy.setConferenceCallForTest(mockMtcCall);
        mConfExtProxy.abort();
        verify(mockMtcCall).close();
    }

    @Test
    public void testIsConferenceExtensionRequestor() {
        assertTrue(mConfExtProxy.isConferenceExtensionRequestor(mMockMtcCall));
        mConfExtProxy = new TestConferenceProxy(mMockCallContext, null,
                mParticipants);
        assertFalse(mConfExtProxy.isConferenceExtensionRequestor(mMockMtcCall));
    }

    @Test
    public void testStartInternal() {
        mConfExtProxy.startInternal(true);
        assertTrue(mIsHoldCalled);

        mConfExtProxy.addListener(mCallListener, mConfListener);
        when(mMockMtcCall.getCallId()).thenReturn("1");
        mConfExtProxy.startInternal(false);

        //Expected to call onCallProxyExtendToConference
        sleep(500);
        assertTrue(mIscallBackCalled);
    }

    @Test
    public void testOnCallTerminated() {
        assertNotNull(mConfExtProxy);
        assertNotNull(mMtcCallListenerProxy);
        mConfExtProxy.addListener(mCallListener, mConfListener);

        //Verify onCallTerminated
        when(mMockMtcCall.getCallId()).thenReturn("1");
        mMtcCallListenerProxy.onCallTerminated(mMockMtcCall, mMockCallReasonInfo);
        sleep(500);
        assertTrue(mIscallBackCalled);
        mIscallBackCalled = false;

        //Expected callback onCallTerminated
        mMtcCallListenerProxy.onCallTerminated(mMockMtcCall, mMockCallReasonInfo);
        sleep(500);
        assertTrue(mIscallBackCalled);

        mIscallBackCalled = false;
        mConfExtProxy.setStateForTest(ConferenceProxy.STATE_HOLDING);
        mMtcCallListenerProxy.onCallTerminated(mMockMtcCall, mMockCallReasonInfo);

        //Expected callback onCallConferenceExtendFailed
        sleep(500);
        assertTrue(mIscallBackCalled);
    }

    @Test
    public void testOnCallHeld() {
        assertNotNull(mConfExtProxy);
        assertNotNull(mMtcCallListenerProxy);
        mConfExtProxy.addListener(mCallListener, mConfListener);

        //Verify onCallHeld
        when(mMockMtcCall.getCallId()).thenReturn("1");
        mConfExtProxy.setStateForTest(ConferenceProxy.STATE_HOLDING);
        mMtcCallListenerProxy.onCallHeld(mMockMtcCall, mMockCallInfo, mMockMediaInfo,
                mMockSuppInfo);
        sleep(500);
        //Expected to call onCallProxyExtendToConference
        assertTrue(mIscallBackCalled);
    }

    @Test
    public void testOnCallHoldFailed() {
        when(mMockMtcCall.getCallId()).thenReturn("1");
        mConfExtProxy.addListener(mCallListener, mConfListener);

        mMtcCallListenerProxy.onCallHoldFailed(mMockMtcCall, mMockCallReasonInfo);
        //Expected to call onCallConferenceExtendFailed
        sleep(500);
        assertTrue(mIscallBackCalled);
    }

    @Test
    public void testOnCallConferenceExtended() {
        when(mMockMtcCall.getCallId()).thenReturn("1");
        mConfExtProxy.addListener(mCallListener, mConfListener);

        MtcConference mtcConference = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mtcConference);
        when(mtcConference.isSameCall(mtcConference)).thenReturn(true);
        mMtcConfListenerProxy.onCallConferenceExtended(mtcConference, 123, mMockCallInfo,
                mMockMediaInfo, mMockSuppInfo);

        //Expected to call onCallProxyExtendToConference
        sleep(500);
        assertTrue(mIscallBackCalled);
    }

    @Test
    public void testOnCallConferenceExtendFailed() {
        when(mMockMtcCall.getCallId()).thenReturn("1");
        mConfExtProxy.addListener(mCallListener, mConfListener);

        MtcConference mtcConference = Mockito.mock(MtcConference.class);
        when(mMockMtcCall.getConferenceInterface()).thenReturn(mtcConference);
        when(mtcConference.isSameCall(mtcConference)).thenReturn(true);
        mMtcConfListenerProxy.onCallConferenceExtendFailed(mtcConference, mMockCallReasonInfo);

        //Expected to call onCallConferenceExtendFailed
        sleep(500);
        assertTrue(mIscallBackCalled);
    }

    private class TestCallListener extends MtcCall.Listener {
        public void onCallTerminated(MtcCall call, CallReasonInfo callReasonInfo) {
            Log.d(LOG_TAG, "onCallTerminated");
            mIscallBackCalled = true;
        }
    }

    private class TestConferenceListener extends MtcConference.Listener {
        public void onCallConferenceExtendFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            Log.d(LOG_TAG, "onCallConferenceExtendFailed");
            mIscallBackCalled = true;
        }

        public void onCallProxyExtendToConference(MtcConference confCall,
                MtcConference hostCall, String[] participants) {
            Log.d(LOG_TAG, "onCallProxyExtendToConference");
            mIscallBackCalled = true;
        }

        public void onCallConferenceExtended(MtcConference call, long confCallId,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            Log.d(LOG_TAG, "onCallConferenceExtended");
            mIscallBackCalled = true;
        }
    }

    private class TestConferenceProxy extends ConferenceExtensionProxy {
        TestConferenceProxy(ICallContext callContext, MtcCall fgCall, String[] participants) {
            super(callContext, fgCall, participants);
        }

        public MtcCall getConferenceCall() {
            Log.d(LOG_TAG, "getConferenceCall");
            return mMockMtcCall;
        }

        protected boolean startInternal(boolean holdRequired) {
            return super.startInternal(holdRequired);
        }

        protected void executeHold(final MtcCall call) {
            Log.d(LOG_TAG, "executeHold");
            mIsHoldCalled = true;
        }

        public void setStateForTest(int state) {
            setState(state);
        }

        public void setConferenceCallForTest(MtcCall call) {
            setConferenceCall(call);
        }
    }

    private void sleep(long ms) {
        try {
            Thread.sleep(ms);
        } catch (Exception e) {
            Log.d(LOG_TAG, "InterruptedException");
        }
    }
}
