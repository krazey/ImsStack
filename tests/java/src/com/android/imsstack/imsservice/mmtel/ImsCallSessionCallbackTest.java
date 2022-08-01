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

import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.verify;

import android.os.DeadObjectException;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.util.Log;

import com.android.imsstack.*;
import com.android.imsstack.imsservice.mmtel.ImsCallSessionCallback;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsCallSessionCallbackTest {
    private static final String LOG_TAG = "ImsCallSessionCallbackTest";
    private static final int LATCH_WAIT = 0;
    private static final int LATCH_MAX = 1;
    private static final int WAIT_TIMER = 100;
    private final Object mLock = new Object();

    private boolean isMultiParty;
    private char dtmf;
    private int mode;
    private int srcAccessTech;
    private int status;
    private int targetAccessTech;
    private String data;
    private String ussdMessage;

    @Mock
    private ImsCallSessionListener mMockListener;
    private ImsCallSessionCallback mImsCallSessionCallback;
    private ImsCallSessionImplBase session, confSession, newSession;
    private ImsCallProfile profile, callProfile;
    private ImsConferenceState confState;
    private ImsReasonInfo reasonInfo;
    private ImsStreamMediaProfile mediaProfile;
    private ImsSuppServiceNotification issn;
    private MessageExecutor mMessageExecutor;

    private static final CountDownLatch[] sLatches = new CountDownLatch[LATCH_MAX];
    static {
        for (int i = 0; i < LATCH_MAX; i++) {
            sLatches[i] = new CountDownLatch(1);
        }
    }

    public boolean callingTestLatchCountdown(int latchIndex, int waitMs) {
        boolean complete = false;
        try {
            CountDownLatch latch;
            synchronized (mLock) {
                latch = sLatches[latchIndex];
            }
            complete = latch.await(waitMs, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
             //complete == false
        }
        synchronized (mLock) {
            sLatches[latchIndex] = new CountDownLatch(1);
        }
        return complete;
    }

    public void countDownLatch(int latchIndex) {
        synchronized (mLock) {
            sLatches[latchIndex].countDown();
        }
    }

    @Before
    public void setUp() throws Exception {
        Log.d(LOG_TAG, " Unit Test");
        mMessageExecutor = new MessageExecutor("Test");
        mMockListener = Mockito.mock(ImsCallSessionListener.class);
        mImsCallSessionCallback = new ImsCallSessionCallback(mMessageExecutor);
    }

    @Test
    public void testInvokeInitiating(){
        mImsCallSessionCallback.invokeInitiating(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionInitiating(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeProgressing(){
        mImsCallSessionCallback.invokeProgressing(session, mediaProfile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionProgressing(mediaProfile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeStarted(){
        mImsCallSessionCallback.invokeStarted(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionInitiated(any());
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeStartFailed(){
        mImsCallSessionCallback.invokeStartFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionInitiatingFailed(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeTerminated(){
        mImsCallSessionCallback.invokeStartFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionTerminated(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeHeld(){
        mImsCallSessionCallback.invokeHeld(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionHeld(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeHoldFailed(){
        mImsCallSessionCallback.invokeHoldFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionHoldFailed(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeHoldReceived(){
        mImsCallSessionCallback.invokeHeld(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionHoldReceived(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeResumed(){
        mImsCallSessionCallback.invokeResumed(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionResumed(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeResumeFailed(){
        mImsCallSessionCallback.invokeResumeFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionResumeFailed(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeResumeReceived(){
        mImsCallSessionCallback.invokeResumeReceived(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionResumeReceived(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeMergeStarted(){
        mImsCallSessionCallback.invokeMergeStarted(session, confSession, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionMergeStarted(confSession,
                    profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeMergeComplete(){
        mImsCallSessionCallback.invokeMergeComplete(session, newSession);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionMergeComplete(newSession);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeMergeFailed(){
        mImsCallSessionCallback.invokeMergeFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionMergeFailed(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeUpdated(){
        mImsCallSessionCallback.invokeUpdated(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionUpdated(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeUpdateFailed(){
        mImsCallSessionCallback.invokeUpdateFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionUpdateFailed(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeUpdateReceived(){
        mImsCallSessionCallback.invokeUpdated(session, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionUpdateReceived(profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeConferenceExtended(){
        mImsCallSessionCallback.invokeConferenceExtended(session, confSession, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionConferenceExtended(confSession,
                    profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeConferenceExtendFailed(){
        mImsCallSessionCallback.invokeUpdateFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionConferenceExtendFailed(
                    reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeConferenceExtendReceived(){
        mImsCallSessionCallback.invokeConferenceExtendReceived(session, confSession, profile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionConferenceExtendReceived(
                    confSession, profile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeInviteParticipantsRequestDelivered(){
        mImsCallSessionCallback.invokeInviteParticipantsRequestDelivered(session);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionInviteParticipantsRequestDelivered();
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeInviteParticipantsRequestFailed(){
        mImsCallSessionCallback.invokeInviteParticipantsRequestFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionInviteParticipantsRequestFailed(
                    reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeRemoveParticipantsRequestDelivered(){
        mImsCallSessionCallback.invokeRemoveParticipantsRequestDelivered(session);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionRemoveParticipantsRequestDelivered();
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeRemoveParticipantsRequestFailed(){
        mImsCallSessionCallback.invokeRemoveParticipantsRequestFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionRemoveParticipantsRequestFailed(
                    reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeConferenceStateUpdated(){
        mImsCallSessionCallback.invokeConferenceStateUpdated(session, confState);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionConferenceStateUpdated(confState);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeUssdMessageReceived(){
        mImsCallSessionCallback.invokeUssdMessageReceived(session, mode, ussdMessage);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionUssdMessageReceived(mode,
                    ussdMessage);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeMultipartyStateChanged(){
        mImsCallSessionCallback.invokeMultipartyStateChanged(session, isMultiParty);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionMultipartyStateChanged(isMultiParty);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeCallSessionTransferred(){
        mImsCallSessionCallback.invokeCallSessionTransferred(session);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionTransferred();
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeCallSessionTransferFailed(){
        mImsCallSessionCallback.invokeCallSessionTransferFailed(session, reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionTransferFailed(reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeHandover(){
        mImsCallSessionCallback.invokeHandover(session, srcAccessTech, targetAccessTech);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionHandover(srcAccessTech,
                    targetAccessTech, new ImsReasonInfo());
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeHandoverFailed(){
        mImsCallSessionCallback.invokeHandoverFailed(session, srcAccessTech, targetAccessTech,
            reasonInfo);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionHandoverFailed(
                            srcAccessTech, targetAccessTech, reasonInfo);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeTtyModeReceived(){
        mImsCallSessionCallback.invokeTtyModeReceived(session, mode);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionTtyModeReceived(mode);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeSuppServiceReceived(){
        mImsCallSessionCallback.invokeSuppServiceReceived(session, issn);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionSuppServiceReceived(issn);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeRttModifyRequestReceived(){
        mImsCallSessionCallback.invokeRttModifyRequestReceived(session, callProfile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionRttModifyRequestReceived(callProfile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeRttModifyResponseReceived(){
        mImsCallSessionCallback.invokeRttModifyResponseReceived(session, status);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionRttModifyResponseReceived(status);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeRttMessageReceived(){
        mImsCallSessionCallback.invokeRttMessageReceived(session, data);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionRttMessageReceived(data);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeRttAudioIndicatorChanged(){
        mImsCallSessionCallback.invokeRttAudioIndicatorChanged(session, mediaProfile);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                Mockito.verify(mMockListener).callSessionRttAudioIndicatorChanged(mediaProfile);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @Test
    public void testinvokeDtmfReceived(){
        mImsCallSessionCallback.invokeDtmfReceived(session, dtmf);
        callingTestLatchCountdown(LATCH_WAIT, WAIT_TIMER);
        postAndRunTask(() -> {
            try {
                if (mMockListener == null) {
                    return;
                }
                verify(mMockListener, Mockito.times(1)).callSessionDtmfReceived(dtmf);
            } catch (Throwable t) {
                Throwable cause = t.getCause();
                if (t instanceof DeadObjectException
                        || (cause != null && cause instanceof DeadObjectException)) {
                    fail("starting cause Throwable to be thrown: " + t);
                }
            }
        });
    }

    @After
    public void tearDown() throws Exception {
        mImsCallSessionCallback = null;
        mMockListener = null;
    }

    private void postAndRunTask(Runnable task) {
        mMessageExecutor.execute(task);
    }

    private static Looper createLooper(String name) {
        HandlerThread thread = new HandlerThread(name);
        thread.start();

        Looper looper = thread.getLooper();

        if (looper == null) {
            return Looper.getMainLooper();
        }
        return looper;
    }
    /**
     * Executes the tasks in the other thread rather than the calling thread.
     */
    public class MessageExecutor extends Handler implements Executor {
        public MessageExecutor(String name) {
            super(createLooper(name));
        }

        @Override
        public void execute(Runnable r) {
            Message m = Message.obtain(this, 0 /* don't care */, r);
            m.sendToTarget();
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.obj instanceof Runnable) {
                executeInternal((Runnable) msg.obj);
            } else {
                Log.d(LOG_TAG, "[MessageExecutor] handleMessage :: "
                        + "Not runnable object; ignore the msg=" + msg);
            }
        }

        private void executeInternal(Runnable r) {
            try {
                r.run();
            } catch (Throwable t) {
                Log.d(LOG_TAG, "[MessageExecutor] executeInternal :: run task=" + r);
                t.printStackTrace();
            }
        }
    }
}
