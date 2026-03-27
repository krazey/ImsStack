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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Log;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ConferenceProxyTest extends ImsStackTest {
    private static final String LOG_TAG = "ConferenceProxyTest";
    ConferenceProxyWrapperClass mConfProxyWrapper = null;

    public static class ConferenceProxyWrapperClass extends ConferenceProxy {
        boolean mHoldListenerCalled = false;
        boolean mUnholdListenerCalled = false;
        boolean mCallTerminatedrCalled = false;

        public ConferenceProxyWrapperClass(ICallContext callContext) {
            super(callContext);
        }

        public void closeConferenceSessionWrapper() {
            closeConferenceSession();
        }

        public ICallContext getCallContextWrapper() {
            return getCallContext();
        }

        public void setConferenceParticipantsWrapper(UsersInfo usersInfo) {
            setConferenceParticipants(usersInfo);
        }

        public void setConferenceCallWrapper(MtcCall call) {
            setConferenceCall(call);
        }

        public void terminateConferenceSessionWrapper() {
            terminateConferenceSession();
        }

        public MtcCall createConferenceCallWrapper(MtcApp app) {
            return createConferenceCall(app);
        }

        public void setStateWrapper(int state) {
            setState(state);
        }

        public CallReasonInfo createUnspecifiedCallReasonInfoWrapper() {
            return ConferenceProxy.createUnspecifiedCallReasonInfo();
        }

        public void executeHoldWrapper(final MtcCall call) {
            executeHold(call);
        }

        public void executeUnholdWrapper(final MtcCall call) {
            executeUnhold(call);
        }

        public void notifySessionTerminatedWrapper(final MtcCall call) {
            CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_USER_TERMINATED,
                    CallReasonInfo.EXTRA_CODE_CALL_RETRY_NORMAL, "TERMINATED");

            notifySessionTerminated(call, callReasonInfo);
        }

        @Override
        public boolean isVideoDirectionInactiveOnVideoCallHold() {
            return true;
        }

        @Override
        public boolean isTextDirectionInactiveOnRttCallHold() {
            return true;
        }
    }

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        ImsCallContext mCallContext = Mockito.mock(ImsCallContext.class);

        mConfProxyWrapper = new ConferenceProxyWrapperClass(mCallContext);
        assertNotNull(mConfProxyWrapper);

        assertNotNull(mConfProxyWrapper.getCallContextWrapper());
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mConfProxyWrapper = null;
    }

    @Test
    public void testIsConferenceExtensionRequestor() {
        MtcCall call = Mockito.mock(MtcCall.class);
        assertFalse(mConfProxyWrapper.isConferenceExtensionRequestor(call));
    }

    @Test
    public void testIsConferenceForCallMerge() {
        assertFalse(mConfProxyWrapper.isConferenceForCallMerge());
    }

    @Test
    public void testStartInternal() {
        assertFalse(mConfProxyWrapper.startInternal(true));
    }

    @Test
    public void testListenerAddRemove() {
        MtcCall.Listener callListener = new MtcCall.Listener();
        MtcConference.Listener conferenceListener = new MtcConference.Listener();

        mConfProxyWrapper.addListener(callListener, conferenceListener);
        assertTrue(mConfProxyWrapper.hasListeners());

        mConfProxyWrapper.removeListener(callListener, conferenceListener);
        assertFalse(mConfProxyWrapper.hasListeners());
    }

    @Test
    public void testConferenceSessionCreation() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        boolean holdRequired;

        holdRequired = false;
        assertFalse(mConfProxyWrapper.start(mtcApp, holdRequired));
        mConfProxyWrapper.closeConferenceSessionWrapper();

        holdRequired = true;
        assertFalse(mConfProxyWrapper.start(mtcApp, holdRequired));

        mtcApp = null;
    }

    @Test
    public void testConferenceParticipants() {
        MtcCall newCall = Mockito.mock(MtcCall.class);
        MtcApp mtcApp = Mockito.mock(MtcApp.class);

        UsersInfo usersInfo = new UsersInfo();

        mConfProxyWrapper.setConferenceParticipantsWrapper(usersInfo);
        assertNotNull(mConfProxyWrapper.getConferenceParticipants());

        /* set and terminate conference */
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO | MtcCall.FLAG_CONFERENCE))
                .thenReturn(newCall);
        MtcCall mtcCall = mConfProxyWrapper.createConferenceCallWrapper(mtcApp);
        assertNotNull(mtcCall);
        mConfProxyWrapper.setConferenceCallWrapper(mtcCall);
        assertNotNull(mConfProxyWrapper.getConferenceCall());

        mConfProxyWrapper.terminateConferenceSessionWrapper();
        mConfProxyWrapper.closeConferenceSessionWrapper();
        assertNull(mConfProxyWrapper.getConferenceCall());

        mtcApp = null;
        newCall = null;
        mtcCall = null;
    }

    @Test
    public void testConferenceProxyStates() {
        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_IDLE);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_IDLE);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_HOLDING);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_HOLDING);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_SWAP_HOLDING);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_SWAP_HOLDING);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_UNHOLDING);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_UNHOLDING);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_MERGE_WAITING);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_MERGE_WAITING);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_MERGED);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_MERGED);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_CONFERENCE_EXTENDING);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_CONFERENCE_EXTENDING);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_CONFERENCE_EXTENDED);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_CONFERENCE_EXTENDED);

        mConfProxyWrapper.setStateWrapper(ConferenceProxy.STATE_IDLE);
        assertEquals(mConfProxyWrapper.getState(), ConferenceProxy.STATE_IDLE);
    }

    @Test
    public void testUnspecifiedCallReasonInfo() {
        CallReasonInfo reasonInfo = mConfProxyWrapper.createUnspecifiedCallReasonInfoWrapper();
        assertEquals(reasonInfo.mCode, CallReasonInfo.CODE_UNSPECIFIED);
    }

    @Test
    public void testExecuteHoldUnhold() {
        MtcCall call = Mockito.mock(MtcCall.class);
        CallInfo callInf = Mockito.mock(CallInfo.class);
        MediaInfo mediaInf = Mockito.mock(MediaInfo.class);

        mConfProxyWrapper.mHoldListenerCalled = false;
        mConfProxyWrapper.mUnholdListenerCalled = false;

        // Listener callbacks sets the respective flag
        MtcCall.Listener callListener = new MtcCall.Listener() {
            public void onCallProxyHold(MtcCall call) {
                Log.i(LOG_TAG, "OnCallProxyHold");
                mConfProxyWrapper.mHoldListenerCalled = true;
            }

            public void onCallProxyResume(MtcCall call) {
                Log.i(LOG_TAG, "onCallProxyResume");
                mConfProxyWrapper.mUnholdListenerCalled = true;
            }
        };

        MtcConference.Listener conferenceListener = new MtcConference.Listener();
        mConfProxyWrapper.addListener(callListener, conferenceListener);

        if (callListener == null) Log.i(LOG_TAG, "callListener is null");

        if (conferenceListener == null) Log.i(LOG_TAG, "conferenceListener is null");

        if (mConfProxyWrapper.hasListeners()) Log.i(LOG_TAG, "hasListeners");

        when(call.getCallInfo()).thenReturn(callInf);
        when(call.getMediaInfo()).thenReturn(mediaInf);

        /* hold operation */
        mConfProxyWrapper.executeHoldWrapper(call);
        assertEquals(mConfProxyWrapper.mHoldListenerCalled, true);

        /* unhold operation */
        mConfProxyWrapper.executeUnholdWrapper(call);
        assertEquals(mConfProxyWrapper.mUnholdListenerCalled, true);
    }

    @Test
    public void testExecuteCallTerminated() {
        MtcCall call = Mockito.mock(MtcCall.class);

        mConfProxyWrapper.mCallTerminatedrCalled = false;

        MtcCall.Listener callListener = new MtcCall.Listener() {
            public void onCallTerminated(MtcCall call, CallReasonInfo callReasonInfo) {
                Log.i(LOG_TAG, "onCallTerminated");
                mConfProxyWrapper.mCallTerminatedrCalled = true;
            }
        };
        MtcConference.Listener conferenceListener = new MtcConference.Listener();
        mConfProxyWrapper.addListener(callListener, conferenceListener);

        mConfProxyWrapper.notifySessionTerminatedWrapper(call);

        assertEquals(mConfProxyWrapper.mCallTerminatedrCalled, true);
    }
}
