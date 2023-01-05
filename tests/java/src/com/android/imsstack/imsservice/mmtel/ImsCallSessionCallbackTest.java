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
import static org.mockito.Mockito.verify;

import android.os.Looper;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.imsservice.mmtel.ImsCallSessionCallback;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;

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
    private MessageExecutor mMessageExecutor;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        mMessageExecutor = new MessageExecutor(Looper.myLooper());
        mMockListener = Mockito.mock(ImsCallSessionListener.class);
        mImsCallSessionCallback = new ImsCallSessionCallback(mMessageExecutor);
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
    public void test_hasListener() {
        assertEquals(true, mImsCallSessionCallback.hasListener());
    }

    @Test
    public void testInvokeInitiating() {
        mImsCallSessionCallback.invokeInitiating(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionInitiating(mProfile);
    }

    @Test
    public void testinvokeProgressing() {
        mImsCallSessionCallback.invokeProgressing(mSession, mMediaProfile);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionProgressing(mMediaProfile);
    }

    @Test
    public void testinvokeStarted() {
        mImsCallSessionCallback.invokeStarted(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionInitiated(mProfile);
    }

    @Test
    public void testinvokeStartFailed() {
        mImsCallSessionCallback.invokeStartFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionInitiatingFailed(mReasonInfo);
    }

    @Test
    public void testinvokeTerminated() {
        mImsCallSessionCallback.invokeTerminated(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionTerminated(mReasonInfo);
    }

    @Test
    public void testinvokeHeld() {
        mImsCallSessionCallback.invokeHeld(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionHeld(mProfile);
    }

    @Test
    public void testinvokeHoldFailed() {
        mImsCallSessionCallback.invokeHoldFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionHoldFailed(mReasonInfo);
    }

    @Test
    public void testinvokeHoldReceived() {
        mImsCallSessionCallback.invokeHoldReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionHoldReceived(mProfile);
    }

    @Test
    public void testinvokeResumed() {
        mImsCallSessionCallback.invokeResumed(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionResumed(mProfile);
    }

    @Test
    public void testinvokeResumeFailed() {
        mImsCallSessionCallback.invokeResumeFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionResumeFailed(mReasonInfo);
    }

    @Test
    public void testinvokeResumeReceived() {
        mImsCallSessionCallback.invokeResumeReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionResumeReceived(mProfile);
    }

    @Test
    public void testinvokeMergeStarted() {
        mImsCallSessionCallback.invokeMergeStarted(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionMergeStarted(mConfSession, mProfile);
    }

    @Test
    public void testinvokeMergeComplete() {
        mImsCallSessionCallback.invokeMergeComplete(mSession, mNewSession);
        processAllMessages();
        verify(mMockListener).callSessionMergeComplete(mNewSession);
    }

    @Test
    public void testinvokeMergeFailed() {
        mImsCallSessionCallback.invokeMergeFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionMergeFailed(mReasonInfo);
    }

    @Test
    public void testinvokeUpdated() {
        mImsCallSessionCallback.invokeUpdated(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionUpdated(mProfile);
    }

    @Test
    public void testinvokeUpdateFailed() {
        mImsCallSessionCallback.invokeUpdateFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionUpdateFailed(mReasonInfo);
    }

    @Test
    public void testinvokeUpdateReceived() {
        mImsCallSessionCallback.invokeUpdateReceived(mSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionUpdateReceived(mProfile);
    }

    @Test
    public void testinvokeConferenceExtended() {
        mImsCallSessionCallback.invokeConferenceExtended(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionConferenceExtended(mConfSession, mProfile);
    }

    @Test
    public void testinvokeConferenceExtendFailed() {
        mImsCallSessionCallback.invokeConferenceExtendFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionConferenceExtendFailed(mReasonInfo);
    }

    @Test
    public void testinvokeConferenceExtendReceived() {
        mImsCallSessionCallback.invokeConferenceExtendReceived(mSession, mConfSession, mProfile);
        processAllMessages();
        verify(mMockListener).callSessionConferenceExtendReceived(mConfSession, mProfile);
    }

    @Test
    public void testinvokeInviteParticipantsRequestDelivered() {
        mImsCallSessionCallback.invokeInviteParticipantsRequestDelivered(mSession);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionInviteParticipantsRequestDelivered();
    }

    @Test
    public void testinvokeInviteParticipantsRequestFailed() {
        mImsCallSessionCallback.invokeInviteParticipantsRequestFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionInviteParticipantsRequestFailed(mReasonInfo);
    }

    @Test
    public void testinvokeRemoveParticipantsRequestDelivered() {
        mImsCallSessionCallback.invokeRemoveParticipantsRequestDelivered(mSession);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionRemoveParticipantsRequestDelivered();
    }

    @Test
    public void testinvokeRemoveParticipantsRequestFailed() {
        mImsCallSessionCallback.invokeRemoveParticipantsRequestFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionRemoveParticipantsRequestFailed(mReasonInfo);
    }

    @Test
    public void testinvokeConferenceStateUpdated() {
        mImsCallSessionCallback.invokeConferenceStateUpdated(mSession, mConfState);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionConferenceStateUpdated(mConfState);
    }

    @Test
    public void testinvokeUssdMessageReceived() {
        int mode = 0;
        String ussdMessage = "";
        mImsCallSessionCallback.invokeUssdMessageReceived(mSession, mode, ussdMessage);
        processAllMessages();
        verify(mMockListener).callSessionUssdMessageReceived(mode, ussdMessage);
    }

    @Test
    public void testinvokeMultipartyStateChanged() {
        boolean isMultiParty = false;
        mImsCallSessionCallback.invokeMultipartyStateChanged(mSession, isMultiParty);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionMultipartyStateChanged(isMultiParty);
    }

    @Test
    public void testinvokeCallSessionTransferred() {
        mImsCallSessionCallback.invokeCallSessionTransferred(mSession);
        processAllMessages();
        verify(mMockListener).callSessionTransferred();
    }

    @Test
    public void testinvokeCallSessionTransferFailed() {
        mImsCallSessionCallback.invokeCallSessionTransferFailed(mSession, mReasonInfo);
        processAllMessages();
        verify(mMockListener).callSessionTransferFailed(mReasonInfo);
    }

    @Test
    public void testinvokeHandover() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        mImsCallSessionCallback.invokeHandover(mSession, srcAccessTech, targetAccessTech);
        processAllMessages();
        verify(mMockListener).onHandover(anyInt(), anyInt(), any(ImsReasonInfo.class));
    }

    @Test
    public void testinvokeHandoverFailed() {
        int srcAccessTech = 0;
        int targetAccessTech = 0;
        mImsCallSessionCallback.invokeHandoverFailed(mSession, srcAccessTech, targetAccessTech,
                mReasonInfo);
        processAllMessages();
        verify(mMockListener).onHandoverFailed(srcAccessTech, targetAccessTech, mReasonInfo);
    }

    @Test
    public void testinvokeTtyModeReceived() {
        int mode = 0;
        mImsCallSessionCallback.invokeTtyModeReceived(mSession, mode);
        processAllMessages();
        verify(mMockListener).callSessionTtyModeReceived(mode);
    }

    @Test
    public void testinvokeDeflected_invokeDeflectFailed() {
        mMockListener = null;
        mImsCallSessionCallback.invokeDeflected(mSession);
        mImsCallSessionCallback.invokeDeflectFailed(mSession, mReasonInfo);
        assertNull(mMockListener);
    }

    @Test
    public void testinvokeSuppServiceReceived() {
        mImsCallSessionCallback.invokeSuppServiceReceived(mSession, mIssn);
        processAllMessages();
        verify(mMockListener).callSessionSuppServiceReceived(mIssn);
    }

    @Test
    public void testinvokeRttModifyRequestReceived() {
        mImsCallSessionCallback.invokeRttModifyRequestReceived(mSession, mProfile);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionRttModifyRequestReceived(mProfile);
    }

    @Test
    public void testinvokeRttModifyResponseReceived() {
        int status = 0;
        mImsCallSessionCallback.invokeRttModifyResponseReceived(mSession, status);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionRttModifyResponseReceived(status);
    }

    @Test
    public void testinvokeRttMessageReceived() {
        String data = " ";
        mImsCallSessionCallback.invokeRttMessageReceived(mSession, data);
        processAllMessages();
        verify(mMockListener).callSessionRttMessageReceived(data);
    }

    @Test
    public void testinvokeRttAudioIndicatorChanged() {
        mImsCallSessionCallback.invokeRttAudioIndicatorChanged(mSession, mMediaProfile);
        processAllMessages();
        Mockito.verify(mMockListener).callSessionRttAudioIndicatorChanged(mMediaProfile);
    }

    @Test
    public void testinvokeDtmfReceived() {
        char dtmf = '\u0000';
        mImsCallSessionCallback.invokeDtmfReceived(mSession, dtmf);
        processAllMessages();
        verify(mMockListener).callSessionDtmfReceived(dtmf);
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mImsCallSessionCallback = null;
        mProfile = null;
    }
}
