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

import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.os.RemoteException;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.ims.aidl.IImsSmsListener;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.imsservice.mmtel.sms.SmsTransferLayer;
import com.android.imsstack.imsservice.mmtel.sms.SmsUtils;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.util.HexDump;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsSmsImplTest extends ImsSmsImplBase {
    private ImsSmsImpl mImsSmsImpl;
    private MessageExecutor mExecutor;

    @Mock private SmsTransferLayer mMockSmsTransferLayer;
    @Mock private ImsCallContext mMockImsCallContext;
    @Mock private IImsSmsListener mListener;
    private int mToken = 1;
    private int mMessageRef = 10;
    private int mResult = ImsSmsImplBase.SEND_STATUS_OK;
    private byte[] mPdu = HexDump.hexStringToByteArray("21110A81785634121000000666B2996C2603");
    private int mFormat = SmsUtils.FORMAT_INT_3GPP;
    private String mSmsc = "+919876543210";

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mExecutor = new MessageExecutor(ImsSmsImpl.class.getSimpleName());
        when(mMockImsCallContext.getExecutor()).thenReturn(mExecutor);
        doReturn(Looper.getMainLooper()).when(mMockImsCallContext).getCallLooper();

        mImsSmsImpl = new ImsSmsImpl(mMockImsCallContext, mMockSmsTransferLayer, mSmsc);
        mImsSmsImpl.registerSmsListener(mListener);
        mImsSmsImpl.onReady();
    }

    @After
    public void tearDown() throws Exception {
        mImsSmsImpl.dispose();
        mImsSmsImpl = null;
        mExecutor = null;
    }

    @Test
    public void test_sendSms_Success() throws RemoteException {
        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsMessage.FORMAT_3GPP, "1111", true, mPdu);
        verify(mMockSmsTransferLayer).sendMoTPdu(mToken, mMessageRef, mFormat, "1111", mPdu);
    }

    @Test
    public void test_sendSms_Error() throws RemoteException {
        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsMessage.FORMAT_3GPP, "1111", true, null);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, SEND_STATUS_ERROR,
                SmsManager.RESULT_ERROR_NULL_PDU, RESULT_NO_NETWORK_ERROR);

        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsMessage.FORMAT_3GPP2, null, true, mPdu);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, SEND_STATUS_ERROR,
                SmsManager.RESULT_INVALID_SMSC_ADDRESS, RESULT_NO_NETWORK_ERROR);

        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsUtils.FORMAT_STRING[0], null, true, mPdu);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, SEND_STATUS_ERROR,
                SmsManager.RESULT_INVALID_SMS_FORMAT, RESULT_NO_NETWORK_ERROR);

        when(mMockSmsTransferLayer.sendMoTPdu(mToken, mMessageRef, mFormat,
                "1111", mPdu)).thenReturn(SmsUtils.SMS_RESULT_INVALID_SMSC_ADDRESS);
        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsMessage.FORMAT_3GPP, "1111", true, mPdu);
        verify(mListener, times(2)).onSendSmsResult(mToken, mMessageRef, SEND_STATUS_ERROR,
                SmsManager.RESULT_INVALID_SMSC_ADDRESS, RESULT_NO_NETWORK_ERROR);

        when(mMockSmsTransferLayer.sendMoTPdu(mToken, mMessageRef, mFormat,
                "1111", mPdu)).thenReturn(SmsUtils.SMSRL_RESULT_PDU_ENCODING_FAILED);
        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsMessage.FORMAT_3GPP, "1111", true, mPdu);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, SEND_STATUS_ERROR,
                SmsManager.RESULT_ENCODING_ERROR, RESULT_NO_NETWORK_ERROR);

        when(mMockSmsTransferLayer.sendMoTPdu(mToken, mMessageRef, mFormat,
                "1111", mPdu)).thenReturn(SmsUtils.SMSRL_RESULT_MTS_CONTROLLER_FAILED);
        mImsSmsImpl.sendSms(mToken, mMessageRef, SmsMessage.FORMAT_3GPP, "1111", true, mPdu);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, SEND_STATUS_ERROR,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE, RESULT_NO_NETWORK_ERROR);
    }

    @Test
    public void test_onMemoryAvailable() {
        mImsSmsImpl.onMemoryAvailable(mToken);
        verify(mMockSmsTransferLayer).sendMemoryAvailabilityNotification(eq(mToken), eq(mSmsc));
    }

    @Test
    public void test_acknowledgeSms() {
        mImsSmsImpl.acknowledgeSms(mToken, mMessageRef, mResult);
        verify(mMockSmsTransferLayer).sendReportTPdu(mToken, SmsUtils.TP_SMS_DELIVER, mMessageRef,
                mResult);
    }

    @Test
    public void test_acknowledgeSmsReport() {
        mImsSmsImpl.acknowledgeSmsReport(mToken, mMessageRef, mResult);
        verify(mMockSmsTransferLayer).sendReportTPdu(mToken, SmsUtils.TP_SMS_STATUS_REPORT,
                mMessageRef, mResult);
    }

    private SmsTransferLayer.Listener setupListener() {
        ArgumentCaptor<SmsTransferLayer.Listener> callbackArg =
                ArgumentCaptor.forClass(SmsTransferLayer.Listener.class);
        verify(mMockSmsTransferLayer, atLeastOnce()).setListener(callbackArg.capture());
        SmsTransferLayer.Listener listener = callbackArg.getValue();
        assertNotNull(listener);
        return listener;
    }

    @Test
    public void test_notifySmsResult() throws RemoteException {
        SmsTransferLayer.Listener listener = setupListener();
        listener.notifySmsResult(mToken, mMessageRef, mResult, SmsManager.RESULT_ERROR_NONE,
                ImsSmsImplBase.RESULT_NO_NETWORK_ERROR);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, mResult,
                SmsManager.RESULT_ERROR_NONE, ImsSmsImplBase.RESULT_NO_NETWORK_ERROR);

        mResult = SEND_STATUS_ERROR;
        listener.notifySmsResult(mToken, mMessageRef, mResult, SmsManager.RESULT_ENCODING_ERROR,
                ImsSmsImplBase.RESULT_NO_NETWORK_ERROR);
        verify(mListener).onSendSmsResult(mToken, mMessageRef, mResult,
                SmsManager.RESULT_ENCODING_ERROR, ImsSmsImplBase.RESULT_NO_NETWORK_ERROR);

    }

    @Test
    public void test_notifySmsReceived() throws RemoteException {
        SmsTransferLayer.Listener listener = setupListener();
        listener.notifySmsReceived(mToken, mFormat, SmsUtils.TP_SMS_DELIVER, mPdu);
        verify(mListener).onSmsReceived(mToken, SmsMessage.FORMAT_3GPP, mPdu);

        listener.notifySmsReceived(mToken, mFormat, SmsUtils.TP_SMS_STATUS_REPORT,
                mPdu);
        verify(mListener).onSmsStatusReportReceived(mToken, SmsMessage.FORMAT_3GPP, mPdu);

        when(mMockSmsTransferLayer.sendReportTPdu(anyInt(), anyInt(), anyInt(), anyInt()))
                .thenReturn(SmsUtils.RESULT_SUCCESS);
        listener.notifySmsReceived(mToken, mFormat, SmsUtils.TP_SMS_SUBMIT, mPdu);
        verify(mMockSmsTransferLayer).sendReportTPdu(mToken, SmsUtils.TP_SMS_SUBMIT, mMessageRef,
                ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED);

    }
}
