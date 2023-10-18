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
package com.android.imsstack.enabler.sipcontroller;

import static android.telephony.ims.SipDelegateManager.MESSAGE_FAILURE_REASON_UNKNOWN;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.verify;

import android.os.Parcel;
import android.telephony.ims.SipMessage;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.sipcontroller.impl.SipControllerAgent;
import com.android.imsstack.enabler.sipcontroller.impl.SipControllerConstants;
import com.android.imsstack.imsservice.sipcontroller.remote.SipTransportRemoteListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public final class SipControllerAgentTest {

    @Mock
    SipTransportRemoteListener mListener;

    private static final String STARTLINE = "INVITE sip:YOU@test.example.com SIP/2.0";
    private static final String HEADERSECTION = "Via: SIP/2.0/TCP ;branch=z9hG4bK07380a980\n"
            + "To: YOU <sip:YOU@example.com>\n"
            + "From: I <sip:I@example.com>;tag=1928301774\n"
            + "Accept-Contact: *;+tag\n"
            + "Call-ID: 24102b6c5-56055215@fc01:bbbb:cdcd:efe0:1cb4:8ebf:f2c2:e7ec";
    private static final String TRANSACTION_ID = "z9hG4bK07380a980";
    private static final byte[] CONTENT = new byte[0];

    private SipControllerAgent mScAgent;
    private int mSlotId = 0;
    private int mSubId = 0;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mScAgent = SipControllerAgent.getInstance(mSlotId, mSubId);
        mScAgent.setSipTransportListener(mListener);
    }

    @After
    public void tearDown() {
        mScAgent = null;
    }

    @Test
    @SmallTest
    public void messageSend_Sent() {
        sendMessage();
        assertTrue(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerConstants.MESSAGE_SENT_IND);
        parcel.writeString(TRANSACTION_ID);
        parcel.setDataPosition(0);

        mScAgent.onMessage(parcel);
        verify(mListener).onMessageSent(TRANSACTION_ID, mSubId);
        assertFalse(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));
    }

    @Test
    @SmallTest
    public void messageSend_SendFailure() {
        sendMessage();
        assertTrue(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(SipControllerConstants.SEND_MESSAGE_FAILURE_IND);
        parcel.writeInt(MESSAGE_FAILURE_REASON_UNKNOWN);
        parcel.writeString(TRANSACTION_ID);
        parcel.setDataPosition(0);

        mScAgent.onMessage(parcel);
        verify(mListener).onMessageSendFailure(TRANSACTION_ID,
                MESSAGE_FAILURE_REASON_UNKNOWN, mSubId);
        assertFalse(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));
    }

    @Test
    @SmallTest
    public void messageReceived_Notify() {
        Parcel parcel = Parcel.obtain();
        messageReceived(parcel);

        if (parcel.readInt() == SipControllerConstants.MESSAGE_RECEIVED_IND) {
            SipMessage message = SipMessage.CREATOR.createFromParcel(parcel);
            verify(mListener).onMessageReceived(message, mSubId);
        }
        assertTrue(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));

        mScAgent.notifyMessageReceived(TRANSACTION_ID);
        assertFalse(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));
    }

    @Test
    @SmallTest
    public void messageReceive_NotifyError() {
        Parcel parcel = Parcel.obtain();
        messageReceived(parcel);

        if (parcel.readInt() == SipControllerConstants.MESSAGE_RECEIVED_IND) {
            SipMessage message = SipMessage.CREATOR.createFromParcel(parcel);
            verify(mListener).onMessageReceived(message, mSubId);
        }
        assertTrue(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));

        mScAgent.notifyMessageReceiveError(TRANSACTION_ID, MESSAGE_FAILURE_REASON_UNKNOWN);
        assertFalse(mScAgent.mSipMsgMap.containsKey(TRANSACTION_ID));
    }

    public void sendMessage() {
        Parcel parcel = Parcel.obtain();

        parcel.writeString(STARTLINE);
        parcel.writeString(HEADERSECTION);
        parcel.writeInt(CONTENT.length);
        parcel.writeByteArray(CONTENT);
        parcel.setDataPosition(0);

        SipMessage message = SipMessage.CREATOR.createFromParcel(parcel);
        mScAgent.sendMessage(message, 001);
    }

    public void messageReceived(Parcel parcel) {

        parcel.writeInt(SipControllerConstants.MESSAGE_RECEIVED_IND);
        parcel.writeString(STARTLINE);
        parcel.writeString(HEADERSECTION);
        parcel.writeInt(CONTENT.length);
        parcel.writeByteArray(CONTENT);
        parcel.setDataPosition(0);

        mScAgent.onMessage(parcel);
        parcel.setDataPosition(0);
    }
}
