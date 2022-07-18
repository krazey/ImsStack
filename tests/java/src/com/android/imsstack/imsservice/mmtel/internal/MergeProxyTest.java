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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.util.Log;

import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;
import com.android.imsstack.imsservice.mmtel.internal.MergeProxy;
import com.android.imsstack.util.MessageExecutor;

import static org.mockito.Mockito.when;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MergeProxyTest {
    private static final String TAG = "MergeProxyTest";
    private TestMergeProxy mMergeProxy = null;
    private MtcCall mFgCall = null;
    private MtcCall mBgCall = null;
    private MtcCall mMockMtcCall = null;
    private boolean mIsHoldCalled = false;
    private ImsCallContext mCallContext = null;
    private MtcCall.Listener mMtcCallListenerProxy = null;
    private MtcConference.Listener mMtcConferencelistenerProxy = null;

    public static final int STATE_IDLE = 0;
    public static final int STATE_HOLDING = 1;
    public static final int STATE_SWAP_HOLDING = 2;
    public static final int STATE_UNHOLDING = 3;
    public static final int STATE_MERGE_WAITING = 4;
    public static final int STATE_MERGED = 5;
    public static final int STATE_CONFERENCE_EXTENDING = 6;
    public static final int STATE_CONFERENCE_EXTENDED = 7;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mCallContext = Mockito.mock(ImsCallContext.class);
        mFgCall = Mockito.mock(MtcCall.class);
        mBgCall = Mockito.mock(MtcCall.class);
        mMockMtcCall = Mockito.mock(MtcCall.class);

        MessageExecutor mExecutor = new MessageExecutor(ImsCallUtils.class.getSimpleName());
        when(mCallContext.getExecutor()).thenReturn(mExecutor);

        mMergeProxy = new TestMergeProxy(mCallContext, mFgCall, mBgCall);
        mMtcCallListenerProxy = mMergeProxy.getMtcCallListener();
        mMtcConferencelistenerProxy = mMergeProxy.getMtcConferenceListener();
    }

    @After
    public void tearDown() {
        mMergeProxy = null;
        mFgCall = null;
        mBgCall = null;
        mMockMtcCall = null;
    }

    @Test
    public void testMtcCallListenerCallbacks() {
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        CallInfo callInfo = Mockito.mock(CallInfo.class);
        MediaInfo mediaInfo = Mockito.mock(MediaInfo.class);
        SuppInfo suppInfo = Mockito.mock(SuppInfo.class);
        CallReasonInfo failInfo = new CallReasonInfo();

        assertNotNull(mMergeProxy);

        /* MTC listener object */
        assertNotNull(mMtcCallListenerProxy);

        ConferenceProxy confProxy = mMergeProxy;

        /* callbacks */
        mMtcCallListenerProxy.onCallTerminated(mtcCall, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallHeld(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallHeld(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallHoldFailed(mtcCall, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallHoldReceived(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallResumed(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallResumeFailed(mtcCall, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallResumeReceived(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallAutoUpdated(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallUpdateReceived(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcCallListenerProxy.onCallInfoUpdated(mtcCall, 0, "", 0, false);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCall = null;
        callInfo = null;
        mediaInfo = null;
        suppInfo = null;
        failInfo = null;
        mMtcCallListenerProxy = null;
    }

    @Test
    public void testMtcConferenceListenerCallbacks() {
        MtcConference mtcConference = Mockito.mock(MtcConference.class);
        CallInfo callInfo = Mockito.mock(CallInfo.class);
        MediaInfo mediaInfo = Mockito.mock(MediaInfo.class);
        SuppInfo suppInfo = Mockito.mock(SuppInfo.class);
        UsersInfo usersInfo = Mockito.mock(UsersInfo.class);

        CallReasonInfo failInfo = new CallReasonInfo();
        ConferenceProxy confProxy = mMergeProxy;

        /* MTC listener object */
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcConferencelistenerProxy.onCallMerged(mtcConference, callInfo, mediaInfo, suppInfo,
                usersInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcConferencelistenerProxy.onCallMergeFailed(mtcConference, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mMtcConferencelistenerProxy.onCallConferenceStateUpdated(mtcConference, usersInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcConference = null;
        callInfo = null;
        mediaInfo = null;
        suppInfo = null;
        usersInfo = null;
        failInfo = null;
    }

    @Test
    public void testConferenceExtension() {
        MtcCall mtcCall = Mockito.mock(MtcCall.class);

        assertTrue(mMergeProxy.isConferenceForCallMerge());
        assertFalse(mMergeProxy.isConferenceExtensionRequestor(mtcCall));
        assertTrue(mMergeProxy.isConferenceExtensionRequestor(mFgCall));

        mtcCall = null;
    }

    @Test
    public void testMergeProxyCleanup() {
        ConferenceProxy confProxy = mMergeProxy;
        mMergeProxy.abort();
        mMergeProxy.dispose();
        mMockMtcCall = null;
        assertNull(confProxy.getConferenceCall());
    }

    @Test
    public void testStartInternal() {
        mMockMtcCall = null;
        mMergeProxy.startInternal(true);

        mMockMtcCall = Mockito.mock(MtcCall.class);
        mMergeProxy.setInitialConferenceExtensionForTest(true);
        mMergeProxy.startInternal(true);
        verify(mMockMtcCall).setListener(mMtcCallListenerProxy);
        verify(mFgCall, times(1)).setListener(mMtcCallListenerProxy);
        verify(mBgCall, times(1)).setListener(mMtcCallListenerProxy);

        when(mCallContext.getSlotId()).thenReturn(0);
        assertTrue(mIsHoldCalled);
        mIsHoldCalled = false;

        mMergeProxy.setInitialConferenceExtensionForTest(false);
        when(mCallContext.getSlotId()).thenReturn(0);
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        mMergeProxy.startInternal(false);

        verify(mFgCall, times(2)).setListener(mMtcCallListenerProxy);
        verify(mBgCall, times(2)).setListener(mMtcCallListenerProxy);
        assertTrue(mIsHoldCalled);
    }

    private class TestMergeProxy extends MergeProxy {
        TestMergeProxy(ICallContext callContext, MtcCall fgCall, MtcCall bgCall) {
            super(callContext, fgCall, bgCall);
        }

        public MtcCall getConferenceCall() {
            Log.d(TAG, "getConferenceCall");
            return mMockMtcCall;
        }

        protected boolean startInternal(boolean holdRequired) {
            return super.startInternal(holdRequired);
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
    }
}
