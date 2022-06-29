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

import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
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

    private MergeProxy mMergeProxy = null;
    private static final String TAG = "MergeProxyTest";
    private MtcCall mFgCall = null;
    private MtcCall mBgCall = null;

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
        ImsCallContext mCallContext = Mockito.mock(ImsCallContext.class);
        ImsCallApp callApp = Mockito.mock(ImsCallApp.class);
        mFgCall = Mockito.mock(MtcCall.class);
        mBgCall = Mockito.mock(MtcCall.class);

        MessageExecutor mExecutor = new MessageExecutor(ImsCallUtils.class.getSimpleName());
        when(mCallContext.getExecutor()).thenReturn(mExecutor);

        mMergeProxy = new MergeProxy(mCallContext, mFgCall, mBgCall);
    }

    @After
    public void tearDown() {
        mMergeProxy = null;
        mFgCall = null;
        mBgCall = null;
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
        MtcCall.Listener mtcCallListenerProxy = mMergeProxy.getMtcCallListener();
        assertNotNull(mtcCallListenerProxy);

        ConferenceProxy confProxy = mMergeProxy;

        /* callbacks */
        mtcCallListenerProxy.onCallTerminated(mtcCall, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallHeld(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallHeld(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallHoldFailed(mtcCall, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallHoldReceived(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallResumed(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallResumeFailed(mtcCall, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallResumeReceived(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallAutoUpdated(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallUpdateReceived(mtcCall, callInfo, mediaInfo, suppInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCallListenerProxy.onCallInfoUpdated(mtcCall, 0, "", 0, false);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcCall = null;
        callInfo = null;
        mediaInfo = null;
        suppInfo = null;
        failInfo = null;
        mtcCallListenerProxy = null;

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
        MtcConference.Listener mtcConferencelistenerProxy = mMergeProxy.getMtcConferenceListener();
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcConferencelistenerProxy.onCallMerged(mtcConference, callInfo, mediaInfo, suppInfo,
                usersInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcConferencelistenerProxy.onCallMergeFailed(mtcConference, failInfo);
        assertEquals(confProxy.getState(), STATE_IDLE);

        mtcConferencelistenerProxy.onCallConferenceStateUpdated(mtcConference, usersInfo);
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

        assertNull(confProxy.getConferenceCall());
    }
}
