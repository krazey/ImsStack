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
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.os.DeadSystemException;
import android.telephony.CallQuality;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.media.MediaTestUtils;
import com.android.imsstack.imsservice.mmtel.ImsCallSessionCallback;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.util.Set;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsCallSessionCallbackTest extends ImsStackTest {
    @Mock private ImsCallSessionListener mMockListener;
    @Mock private ImsCallSessionImplBase mSession, mConfSession, mNewSession;

    private ImsCallSessionCallback mImsCallSessionCallback;
    private ImsCallProfile mProfile;
    private ImsConferenceState mConfState;
    private ImsReasonInfo mReasonInfo;
    private ImsStreamMediaProfile mMediaProfile;
    private ImsSuppServiceNotification mIssn;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        mMockListener = Mockito.mock(ImsCallSessionListener.class);
        mImsCallSessionCallback = new ImsCallSessionCallback();
        mSession = Mockito.mock(ImsCallSessionImplBase.class);
        mConfSession = Mockito.mock(ImsCallSessionImplBase.class);
        mNewSession = Mockito.mock(ImsCallSessionImplBase.class);
        mProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        mReasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        mMediaProfile = new ImsStreamMediaProfile(ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID);
        mImsCallSessionCallback.setListener(mMockListener);
    }

    @Test
    public void testHasListener() {
        assertEquals(true, mImsCallSessionCallback.hasListener());
        mImsCallSessionCallback.setListener(null);
        assertEquals(false, mImsCallSessionCallback.hasListener());
    }

    @Test
    public void testInvokeInitiating() {
        mImsCallSessionCallback.invokeInitiating(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionInitiating(mProfile);
    }

    @Test
    public void testInvokeInitiatingForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeInitiating(mSession, mProfile);
        verify(mMockListener, never()).callSessionInitiating(mProfile);
    }

    @Test
    public void testInvokeInitiatingException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionInitiating(mProfile);
        mImsCallSessionCallback.invokeInitiating(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeProgressing() {
        mImsCallSessionCallback.invokeProgressing(mSession, mMediaProfile);
        processAllMessages();
        verify(mMockListener).callSessionProgressing(mMediaProfile);
    }

    @Test
    public void testInvokeProgressingForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeProgressing(mSession, mMediaProfile);
        verify(mMockListener, never()).callSessionProgressing(mMediaProfile);
    }

    @Test
    public void testInvokeProgressingException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionProgressing(mMediaProfile);
        mImsCallSessionCallback.invokeProgressing(mSession, mMediaProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeStarted() {
        mImsCallSessionCallback.invokeStarted(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionInitiated(mProfile);
    }

    @Test
    public void testInvokeStartedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeStarted(mSession, mProfile);
        verify(mMockListener, never()).callSessionInitiated(mProfile);
    }

    @Test
    public void testInvokeStartedException() {
        doThrow(newDeadSystemRuntimeException()).when(mMockListener).callSessionInitiated(mProfile);
        mImsCallSessionCallback.invokeStarted(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeStartFailed() {
        mImsCallSessionCallback.invokeStartFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionInitiatingFailed(mReasonInfo);
    }

    @Test
    public void testInvokeStartFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeStartFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionInitiatingFailed(mReasonInfo);
    }

    @Test
    public void testInvokeStartFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionInitiatingFailed(mReasonInfo);
        mImsCallSessionCallback.invokeStartFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeTerminated() {
        mImsCallSessionCallback.invokeTerminated(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionTerminated(mReasonInfo);
    }

    @Test
    public void testInvokeTerminatedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeTerminated(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionTerminated(mReasonInfo);
    }

    @Test
    public void testInvokeTerminatedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionTerminated(mReasonInfo);
        mImsCallSessionCallback.invokeTerminated(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeHeld() {
        mImsCallSessionCallback.invokeHeld(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionHeld(mProfile);
    }

    @Test
    public void testInvokeHeldForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeHeld(mSession, mProfile);
        verify(mMockListener, never()).callSessionHeld(mProfile);
    }

    @Test
    public void testInvokeHeldException() {
        doThrow(newDeadSystemRuntimeException()).when(mMockListener).callSessionHeld(mProfile);
        mImsCallSessionCallback.invokeHeld(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeHoldFailed() {
        mImsCallSessionCallback.invokeHoldFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionHoldFailed(mReasonInfo);
    }

    @Test
    public void testInvokeHoldFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeHoldFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionHoldFailed(mReasonInfo);
    }

    @Test
    public void testInvokeHoldFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionHoldFailed(mReasonInfo);
        mImsCallSessionCallback.invokeHoldFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeHoldReceived() {
        mImsCallSessionCallback.invokeHoldReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionHoldReceived(mProfile);
    }

    @Test
    public void testInvokeHoldReceivedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeHoldReceived(mSession, mProfile);
        verify(mMockListener, never()).callSessionHoldReceived(mProfile);
    }

    @Test
    public void testInvokeHoldReceivedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionHoldReceived(mProfile);
        mImsCallSessionCallback.invokeHoldReceived(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeResumed() {
        mImsCallSessionCallback.invokeResumed(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionResumed(mProfile);
    }

    @Test
    public void testInvokeResumedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeResumed(mSession, mProfile);
        verify(mMockListener, never()).callSessionResumed(mProfile);
    }

    @Test
    public void testInvokeResumedException() {
        doThrow(newDeadSystemRuntimeException()).when(mMockListener).callSessionResumed(mProfile);
        mImsCallSessionCallback.invokeResumed(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeResumeFailed() {
        mImsCallSessionCallback.invokeResumeFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionResumeFailed(mReasonInfo);
    }

    @Test
    public void testInvokeResumeFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeResumeFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionResumeFailed(mReasonInfo);
    }

    @Test
    public void testInvokeResumeFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionResumeFailed(mReasonInfo);
        mImsCallSessionCallback.invokeResumeFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeResumeReceived() {
        mImsCallSessionCallback.invokeResumeReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionResumeReceived(mProfile);
    }

    @Test
    public void testInvokeResumeReceivedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeResumeReceived(mSession, mProfile);
        verify(mMockListener, never()).callSessionResumeReceived(mProfile);
    }

    @Test
    public void testInvokeResumeReceivedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionResumeReceived(mProfile);
        mImsCallSessionCallback.invokeResumeReceived(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeMergeStarted() {
        mImsCallSessionCallback.invokeMergeStarted(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionMergeStarted(mConfSession, mProfile);
    }

    @Test
    public void testInvokeMergeStartedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeMergeStarted(mSession, mConfSession, mProfile);
        verify(mMockListener, never()).callSessionMergeStarted(mConfSession, mProfile);
    }

    @Test
    public void testInvokeMergeStartedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionMergeStarted(mConfSession, mProfile);
        mImsCallSessionCallback.invokeMergeStarted(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeMergeComplete() {
        mImsCallSessionCallback.invokeMergeComplete(mSession, mNewSession);
        processAllMessages();
        verify(mMockListener).callSessionMergeComplete(mNewSession);
    }

    @Test
    public void testInvokeMergeCompleteForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeMergeComplete(mSession, mNewSession);
        verify(mMockListener, never()).callSessionMergeComplete(mNewSession);
    }

    @Test
    public void testInvokeMergeCompleteException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionMergeComplete(mNewSession);
        mImsCallSessionCallback.invokeMergeComplete(mSession, mNewSession);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeMergeFailed() {
        mImsCallSessionCallback.invokeMergeFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionMergeFailed(mReasonInfo);
    }

    @Test
    public void testInvokeMergeFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeMergeFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionMergeFailed(mReasonInfo);
    }

    @Test
    public void testInvokeMergeFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionMergeFailed(mReasonInfo);
        mImsCallSessionCallback.invokeMergeFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeUpdated() {
        mImsCallSessionCallback.invokeUpdated(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionUpdated(mProfile);
    }

    @Test
    public void testInvokeUpdatedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeUpdated(mSession, mProfile);
        verify(mMockListener, never()).callSessionUpdated(mProfile);
    }

    @Test
    public void testInvokeUpdatedException() {
        doThrow(newDeadSystemRuntimeException()).when(mMockListener).callSessionUpdated(mProfile);
        mImsCallSessionCallback.invokeUpdated(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeUpdateFailed() {
        mImsCallSessionCallback.invokeUpdateFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionUpdateFailed(mReasonInfo);
    }

    @Test
    public void testInvokeUpdateFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeUpdateFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionUpdateFailed(mReasonInfo);
    }

    @Test
    public void testInvokeUpdateFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionUpdateFailed(mReasonInfo);
        mImsCallSessionCallback.invokeUpdateFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeUpdateReceived() {
        mImsCallSessionCallback.invokeUpdateReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionUpdateReceived(mProfile);
    }

    @Test
    public void testInvokeUpdateReceivedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeUpdateReceived(mSession, mProfile);
        verify(mMockListener, never()).callSessionUpdateReceived(mProfile);
    }

    @Test
    public void testInvokeUpdateReceivedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionUpdateReceived(mProfile);
        mImsCallSessionCallback.invokeUpdateReceived(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeConferenceExtended() {
        mImsCallSessionCallback.invokeConferenceExtended(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionConferenceExtended(mConfSession, mProfile);
    }

    @Test
    public void testInvokeConferenceExtendedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeConferenceExtended(mSession, mConfSession, mProfile);
        verify(mMockListener, never()).callSessionConferenceExtended(mConfSession, mProfile);
    }

    @Test
    public void testInvokeConferenceExtendedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionConferenceExtended(mConfSession, mProfile);
        mImsCallSessionCallback.invokeConferenceExtended(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeConferenceExtendFailed() {
        mImsCallSessionCallback.invokeConferenceExtendFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionConferenceExtendFailed(mReasonInfo);
    }

    @Test
    public void testInvokeConferenceExtendFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeConferenceExtendFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionConferenceExtendFailed(mReasonInfo);
    }

    @Test
    public void testInvokeConferenceExtendFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionConferenceExtendFailed(mReasonInfo);
        mImsCallSessionCallback.invokeConferenceExtendFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeConferenceExtendReceived() {
        mImsCallSessionCallback.invokeConferenceExtendReceived(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionConferenceExtendReceived(mConfSession, mProfile);
    }

    @Test
    public void testInvokeConferenceExtendReceivedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeConferenceExtendReceived(mSession, mConfSession, mProfile);
        verify(mMockListener, never()).callSessionConferenceExtendReceived(mConfSession, mProfile);
    }

    @Test
    public void testInvokeConferenceExtendReceivedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionConferenceExtendReceived(mConfSession, mProfile);
        mImsCallSessionCallback.invokeConferenceExtendReceived(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeInviteParticipantsRequestDelivered() {
        mImsCallSessionCallback.invokeInviteParticipantsRequestDelivered(mSession);
        processAllMessages();
        verify(mMockListener).callSessionInviteParticipantsRequestDelivered();
    }

    @Test
    public void testInvokeInviteParticipantsRequestDeliveredForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeInviteParticipantsRequestDelivered(mSession);
        verify(mMockListener, never()).callSessionInviteParticipantsRequestDelivered();
    }

    @Test
    public void testInvokeInviteParticipantsRequestDeliveredException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionInviteParticipantsRequestDelivered();
        mImsCallSessionCallback.invokeInviteParticipantsRequestDelivered(mSession);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeInviteParticipantsRequestFailed() {
        mImsCallSessionCallback.invokeInviteParticipantsRequestFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionInviteParticipantsRequestFailed(mReasonInfo);
    }

    @Test
    public void testInvokeInviteParticipantsRequestFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeInviteParticipantsRequestFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionInviteParticipantsRequestFailed(mReasonInfo);
    }

    @Test
    public void testInvokeInviteParticipantsRequestFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionInviteParticipantsRequestFailed(mReasonInfo);
        mImsCallSessionCallback.invokeInviteParticipantsRequestFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeRemoveParticipantsRequestDelivered() {
        mImsCallSessionCallback.invokeRemoveParticipantsRequestDelivered(mSession);
        processAllMessages();
        verify(mMockListener).callSessionRemoveParticipantsRequestDelivered();
    }

    @Test
    public void testInvokeRemoveParticipantsRequestDeliveredForNulllistener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRemoveParticipantsRequestDelivered(mSession);
        verify(mMockListener, never()).callSessionRemoveParticipantsRequestDelivered();
    }

    @Test
    public void testInvokeRemoveParticipantsRequestDeliveredException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionRemoveParticipantsRequestDelivered();
        mImsCallSessionCallback.invokeRemoveParticipantsRequestDelivered(mSession);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeRemoveParticipantsRequestFailed() {
        mImsCallSessionCallback.invokeRemoveParticipantsRequestFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionRemoveParticipantsRequestFailed(mReasonInfo);
    }

    @Test
    public void testInvokeRemoveParticipantsRequestFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRemoveParticipantsRequestFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionRemoveParticipantsRequestFailed(mReasonInfo);
    }

    @Test
    public void testInvokeRemoveParticipantsRequestFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionRemoveParticipantsRequestFailed(mReasonInfo);
        mImsCallSessionCallback.invokeRemoveParticipantsRequestFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeConferenceStateUpdated() {
        mImsCallSessionCallback.invokeConferenceStateUpdated(mSession, mConfState);
        processAllMessages();
        verify(mMockListener).callSessionConferenceStateUpdated(mConfState);
    }

    @Test
    public void testInvokeConferenceStateUpdatedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeConferenceStateUpdated(mSession, mConfState);
        verify(mMockListener, never()).callSessionConferenceStateUpdated(mConfState);
    }

    @Test
    public void testInvokeConferenceStateUpdatedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionConferenceStateUpdated(mConfState);
        mImsCallSessionCallback.invokeConferenceStateUpdated(mSession, mConfState);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeUssdMessageReceived() {
        int mode = 0;
        String ussdMessage = "";
        mImsCallSessionCallback.invokeUssdMessageReceived(mSession, mode, ussdMessage);
        processAllMessages();
        verify(mMockListener).callSessionUssdMessageReceived(mode, ussdMessage);
    }

    @Test
    public void testInvokeUssdMessageReceivedForNullListener() {
        int mode = 0;
        String ussdMessage = "";
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeUssdMessageReceived(mSession, mode, ussdMessage);
        verify(mMockListener, never()).callSessionUssdMessageReceived(mode, ussdMessage);
    }

    @Test
    public void testInvokeUssdMessageReceivedException() {
        int mode = 0;
        String ussdMessage = "";
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionUssdMessageReceived(mode, ussdMessage);
        mImsCallSessionCallback.invokeUssdMessageReceived(mSession, mode, ussdMessage);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeMultipartyStateChanged() {
        boolean isMultiParty = false;
        mImsCallSessionCallback.invokeMultipartyStateChanged(mSession, isMultiParty);
        processAllMessages();
        verify(mMockListener).callSessionMultipartyStateChanged(isMultiParty);
    }

    @Test
    public void testInvokeMultipartyStateChangedForNullListener() {
        boolean isMultiParty = false;
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeMultipartyStateChanged(mSession, isMultiParty);
        verify(mMockListener, never()).callSessionMultipartyStateChanged(isMultiParty);
    }

    @Test
    public void testInvokeMultipartyStateChangedException() {
        boolean isMultiParty = false;
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionMultipartyStateChanged(isMultiParty);
        mImsCallSessionCallback.invokeMultipartyStateChanged(mSession, isMultiParty);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeCallSessionTransferred() {
        mImsCallSessionCallback.invokeCallSessionTransferred(mSession);
        processAllMessages();
        verify(mMockListener).callSessionTransferred();
    }

    @Test
    public void testInvokeCallSessionTransferredForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeCallSessionTransferred(mSession);
        verify(mMockListener, never()).callSessionTransferred();
    }

    @Test
    public void testInvokeCallSessionTransferredException() {
        doThrow(newDeadSystemRuntimeException()).when(mMockListener).callSessionTransferred();
        mImsCallSessionCallback.invokeCallSessionTransferred(mSession);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeCallSessionTransferFailed() {
        mImsCallSessionCallback.invokeCallSessionTransferFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionTransferFailed(mReasonInfo);
    }

    @Test
    public void testInvokeCallSessionTransferFailedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeCallSessionTransferFailed(mSession, mReasonInfo);
        verify(mMockListener, never()).callSessionTransferFailed(mReasonInfo);
    }

    @Test
    public void testInvokeCallSessionTransferFailedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionTransferFailed(mReasonInfo);
        mImsCallSessionCallback.invokeCallSessionTransferFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeHandover() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        mImsCallSessionCallback.invokeHandover(mSession, srcAccessTech, targetAccessTech);
        processAllMessages();
        verify(mMockListener).onHandover(anyInt(), anyInt(), any(ImsReasonInfo.class));
    }

    @Test
    public void testInvokeHandoverForNullListener() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeHandover(mSession, srcAccessTech, targetAccessTech);
        verify(mMockListener, never()).onHandover(anyInt(), anyInt(), any(ImsReasonInfo.class));
    }

    @Test
    public void testInvokeHandoverException() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).onHandover(anyInt(), anyInt(), any(ImsReasonInfo.class));
        mImsCallSessionCallback.invokeHandover(mSession, srcAccessTech, targetAccessTech);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeHandoverFailed() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        mImsCallSessionCallback.invokeHandoverFailed(mSession, srcAccessTech, targetAccessTech,
                mReasonInfo);
        processAllMessages();
        verify(mMockListener).onHandoverFailed(srcAccessTech, targetAccessTech, mReasonInfo);
    }

    @Test
    public void testInvokeHandoverFailedForNullListener() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeHandoverFailed(mSession, srcAccessTech, targetAccessTech,
                mReasonInfo);
        verify(mMockListener, never()).onHandoverFailed(srcAccessTech, targetAccessTech,
                mReasonInfo);
    }

    @Test
    public void testInvokeHandoverFailedException() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).onHandoverFailed(srcAccessTech, targetAccessTech, mReasonInfo);
        mImsCallSessionCallback.invokeHandoverFailed(mSession, srcAccessTech, targetAccessTech,
                mReasonInfo);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeTtyModeReceived() {
        int mode = 0;
        mImsCallSessionCallback.invokeTtyModeReceived(mSession, mode);
        processAllMessages();
        verify(mMockListener).callSessionTtyModeReceived(mode);
    }

    @Test
    public void testInvokeTtyModeReceivedForNullListener() {
        int mode = 0;
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeTtyModeReceived(mSession, mode);
        verify(mMockListener, never()).callSessionTtyModeReceived(mode);
    }

    @Test
    public void testInvokeTtyModeReceivedException() {
        int mode = 0;
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionTtyModeReceived(mode);
        mImsCallSessionCallback.invokeTtyModeReceived(mSession, mode);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeDeflected_InvokeDeflectFailed() {
        mMockListener = null;
        mImsCallSessionCallback.invokeDeflected(mSession);
        mImsCallSessionCallback.invokeDeflectFailed(mSession, mReasonInfo);
        assertNull(mMockListener);
    }

    @Test
    public void testInvokeSuppServiceReceived() {
        mImsCallSessionCallback.invokeSuppServiceReceived(mSession, mIssn);
        processAllMessages();
        verify(mMockListener).callSessionSuppServiceReceived(mIssn);
    }

    @Test
    public void testInvokeSuppServiceReceivedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeSuppServiceReceived(mSession, mIssn);
        verify(mMockListener, never()).callSessionSuppServiceReceived(mIssn);
    }

    @Test
    public void testInvokeSuppServiceReceivedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionSuppServiceReceived(mIssn);
        mImsCallSessionCallback.invokeSuppServiceReceived(mSession, mIssn);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeRttModifyRequestReceived() {
        mImsCallSessionCallback.invokeRttModifyRequestReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionRttModifyRequestReceived(mProfile);
    }

    @Test
    public void testInvokeRttModifyRequestReceivedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRttModifyRequestReceived(mSession, mProfile);
        verify(mMockListener, never()).callSessionRttModifyRequestReceived(mProfile);
    }

    @Test
    public void testInvokeRttModifyRequestReceivedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionRttModifyRequestReceived(mProfile);
        mImsCallSessionCallback.invokeRttModifyRequestReceived(mSession, mProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeRttModifyResponseReceived() {
        int status = 0;
        mImsCallSessionCallback.invokeRttModifyResponseReceived(mSession, status);
        processAllMessages();
        verify(mMockListener).callSessionRttModifyResponseReceived(status);
    }

    @Test
    public void testInvokeRttModifyResponseReceivedForNullListener() {
        int status = 0;
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRttModifyResponseReceived(mSession, status);
        verify(mMockListener, never()).callSessionRttModifyResponseReceived(status);
    }

    @Test
    public void testInvokeRttModifyResponseReceivedException() {
        int status = 0;
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionRttModifyResponseReceived(status);
        mImsCallSessionCallback.invokeRttModifyResponseReceived(mSession, status);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeRttMessageReceived() {
        String data = " ";
        mImsCallSessionCallback.invokeRttMessageReceived(mSession, data);
        processAllMessages();
        verify(mMockListener).callSessionRttMessageReceived(data);
    }

    @Test
    public void testInvokeRttMessageReceivedForNullListener() {
        String data = " ";
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRttMessageReceived(mSession, data);
        verify(mMockListener, never()).callSessionRttMessageReceived(data);
    }

    @Test
    public void testInvokeRttMessageReceivedException() {
        String data = " ";
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionRttMessageReceived(data);
        mImsCallSessionCallback.invokeRttMessageReceived(mSession, data);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeRttAudioIndicatorChanged() {
        mImsCallSessionCallback.invokeRttAudioIndicatorChanged(mSession, mMediaProfile);
        processAllMessages();
        verify(mMockListener).callSessionRttAudioIndicatorChanged(mMediaProfile);
    }

    @Test
    public void testInvokeRttAudioIndicatorChangedForNullListener() {
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRttAudioIndicatorChanged(mSession, mMediaProfile);
        verify(mMockListener, never()).callSessionRttAudioIndicatorChanged(mMediaProfile);
    }

    @Test
    public void testInvokeRttAudioIndicatorChangedException() {
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionRttAudioIndicatorChanged(mMediaProfile);
        mImsCallSessionCallback.invokeRttAudioIndicatorChanged(mSession, mMediaProfile);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeDtmfReceived() {
        char dtmf = '\u0000';
        mImsCallSessionCallback.invokeDtmfReceived(mSession, dtmf);
        processAllMessages();
        verify(mMockListener).callSessionDtmfReceived(dtmf);
    }

    @Test
    public void testInvokeDtmfReceivedForNullListener() {
        char dtmf = '\u0000';
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeDtmfReceived(mSession, dtmf);
        verify(mMockListener, never()).callSessionDtmfReceived(dtmf);
    }

    @Test
    public void testInvokeDtmfReceivedException() {
        char dtmf = '\u0000';
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionDtmfReceived(dtmf);
        mImsCallSessionCallback.invokeDtmfReceived(mSession, dtmf);
        processAllMessages();
        verify(mSession).close();
    }

    @Test
    public void testInvokeCallQualityChanged() {
        CallQuality callQuality = new CallQuality();
        mImsCallSessionCallback.invokeCallQualityChanged(callQuality);
        processAllMessages();
        verify(mMockListener).callQualityChanged(callQuality);
    }

    @Test
    public void testInvokeCallQualityChangedForNullListener() {
        CallQuality callQuality = new CallQuality();
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeCallQualityChanged(callQuality);
        verify(mMockListener, never()).callQualityChanged(callQuality);
    }

    @Test
    public void testInvokeCallQualityChangedException() {
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        CallQuality callQuality = new CallQuality();
        doThrow(mockRuntimeException).when(mMockListener).callQualityChanged(callQuality);
        mImsCallSessionCallback.invokeCallQualityChanged(callQuality);
        processAllMessages();
        verify(mockRuntimeException).getMessage();
    }

    @Test
    public void testInvokeRtpHeaderExtensionsReceived() {
        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        mImsCallSessionCallback.invokeRtpHeaderExtensionsReceived(extensions);
        processAllMessages();
        verify(mMockListener, times(1)).callSessionRtpHeaderExtensionsReceived(extensions);
    }

    @Test
    public void testInvokeRtpHeaderExtensionsReceivedForNullListener() {
        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        mImsCallSessionCallback.setListener(null);
        mImsCallSessionCallback.invokeRtpHeaderExtensionsReceived(extensions);
        verify(mMockListener, never()).callSessionRtpHeaderExtensionsReceived(extensions);
    }

    @Test
    public void testInvokeRtpHeaderExtensionsReceivedException() {
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        doThrow(mockRuntimeException).when(mMockListener)
                .callSessionRtpHeaderExtensionsReceived(extensions);
        mImsCallSessionCallback.invokeRtpHeaderExtensionsReceived(extensions);
        processAllMessages();
        verify(mockRuntimeException, times(1)).getMessage();
    }

    @Test
    public void testCloseSessionException() {
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(newDeadSystemRuntimeException())
            .when(mMockListener).callSessionInitiating(mProfile);
        doThrow(mockRuntimeException).when(mSession).close();
        mImsCallSessionCallback.invokeInitiating(mSession, mProfile);
        processAllMessages();
        verify(mockRuntimeException).getMessage();
    }

    private static Exception newDeadSystemRuntimeException() {
        return new RuntimeException(new DeadSystemException());
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mImsCallSessionCallback = null;
        mProfile = null;
    }
}
