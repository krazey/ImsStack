/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.mts;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MtsJniTest {
    private static final int SLOT_ID = 0;

    @Mock
    private Handler mMockHandler;
    @Mock
    private MtsJni.JniImsProxyWrapper mMockJniProxy;

    private MtsJni mMtsJni;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mMtsJni = new MtsJni(mMockJniProxy);
    }

    @Test
    public void testGetInstance() {
        MtsJni instance = MtsJni.getInstance();
        assertNotNull(instance);
        MtsJni secondInstance = MtsJni.getInstance();
        assertEquals(instance, secondInstance);
        instance.release(SLOT_ID);
    }

    @Test
    public void testInitAndRelease() {
        mMtsJni.init(mMockHandler, SLOT_ID);
        verify(mMockJniProxy).getInterface(anyInt(), anyInt());
        verify(mMockJniProxy).setListener(anyLong(), any());

        mMtsJni.release(SLOT_ID);
        verify(mMockJniProxy).releaseInterface(anyLong());
        verify(mMockJniProxy).removeListener(anyLong());
    }

    @Test
    public void testSendMessage() {
        mMtsJni.init(mMockHandler, SLOT_ID);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(1234); // some data
        parcel.setDataPosition(0);

        mMtsJni.sendMessage(parcel, SLOT_ID);

        verify(mMockJniProxy).sendData(anyLong(), any(byte[].class));
        mMtsJni.release(SLOT_ID);
    }

    @Test
    public void testOnMessageReportMoStatus() {
        // This test needs to run on a thread with a looper
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }
        ArgumentCaptor<Message> messageCaptor = ArgumentCaptor.forClass(Message.class);

        // Directly instantiate listener to test it.
        MtsJni.MtsJniImsListener listener = new MtsJni.MtsJniImsListener();
        listener.setHandler(mMockHandler);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MtsJni.REPORT_MTS_MO_STATUS);
        parcel.writeInt(MtsController.MO_SUCCESS);
        parcel.writeInt(MtsController.SMS_FORMAT_3GPP);
        parcel.writeInt(1); // seqId
        parcel.setDataPosition(0);

        listener.onMessage(parcel);

        verify(mMockHandler).sendMessage(messageCaptor.capture());
        Message capturedMessage = messageCaptor.getValue();
        assertEquals(MtsController.REQUEST_REPORT_MO_STATUS, capturedMessage.what);
        assertNotNull(capturedMessage.obj);
    }

    @Test
    public void testOnMessageReportMtSms() {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }
        ArgumentCaptor<Message> messageCaptor = ArgumentCaptor.forClass(Message.class);

        MtsJni.MtsJniImsListener listener = new MtsJni.MtsJniImsListener();
        listener.setHandler(mMockHandler);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MtsJni.REPORT_MTS_MT_SMS);
        parcel.writeInt(MtsController.SMS_FORMAT_3GPP);
        parcel.writeString("test pdu");
        parcel.setDataPosition(0);

        listener.onMessage(parcel);

        verify(mMockHandler).sendMessage(messageCaptor.capture());
        Message capturedMessage = messageCaptor.getValue();
        assertEquals(MtsController.REQUEST_REPORT_MT_SMS, capturedMessage.what);
        assertEquals(MtsController.SMS_FORMAT_3GPP, capturedMessage.arg1);
        assertEquals("test pdu", capturedMessage.obj);
    }

    @Test
    public void testInitTwice() {
        mMtsJni.init(mMockHandler, SLOT_ID);
        verify(mMockJniProxy, times(1)).getInterface(anyInt(), anyInt());
        verify(mMockJniProxy, times(1)).setListener(anyLong(), any());

        // Call init again, these should not be called again.
        mMtsJni.init(mMockHandler, SLOT_ID);
        verify(mMockJniProxy, times(1)).getInterface(anyInt(), anyInt());
        verify(mMockJniProxy, times(1)).setListener(anyLong(), any());
        mMtsJni.release(SLOT_ID);
    }

    @Test
    public void testReleaseWithoutInit() {
        mMtsJni.release(SLOT_ID);
        verify(mMockJniProxy, never()).releaseInterface(anyLong());
        verify(mMockJniProxy, never()).removeListener(anyLong());
    }

    @Test
    public void testSendMessageWithNullParcel() {
        mMtsJni.init(mMockHandler, SLOT_ID);
        mMtsJni.sendMessage(null, SLOT_ID);
        // Verify sendData is not called
        verify(mMockJniProxy, never()).sendData(anyLong(), any(byte[].class));
        mMtsJni.release(SLOT_ID);
    }

    @Test
    public void testSendMessageWithoutInit() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(1234);
        parcel.setDataPosition(0);

        mMtsJni.sendMessage(parcel, SLOT_ID);

        // Verify sendData is not called because native object doesn't exist.
        verify(mMockJniProxy, never()).sendData(anyLong(), any(byte[].class));
    }

    @Test
    public void testOnMessageUnknownMessage() {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        MtsJni.MtsJniImsListener listener = new MtsJni.MtsJniImsListener();
        listener.setHandler(mMockHandler);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(9999); // Unknown message
        parcel.setDataPosition(0);

        listener.onMessage(parcel);

        // No message should be sent to handler.
        verify(mMockHandler, never()).sendMessage(any(Message.class));
    }

    @Test(expected = NullPointerException.class)
    public void testOnMessageWithNullHandler() {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }
        MtsJni.MtsJniImsListener listener = new MtsJni.MtsJniImsListener();
        listener.setHandler(null);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MtsJni.REPORT_MTS_MO_STATUS);
        parcel.writeInt(MtsController.MO_SUCCESS);
        parcel.writeInt(MtsController.SMS_FORMAT_3GPP);
        parcel.writeInt(1); // seqId
        parcel.setDataPosition(0);

        listener.onMessage(parcel); // This should throw NullPointerException
    }

}
