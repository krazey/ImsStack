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
import static org.mockito.Mockito.verify;

import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.imsservice.mmtel.sms.SmsTransferLayer;
import com.android.imsstack.imsservice.mmtel.sms.SmsUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsSmsImplTest extends ImsSmsImplBase {
    @Mock ImsCallContext mImsCallContext;
    @Mock SmsTransferLayer mSmsTransferLayer;
    private ImsSmsImpl mImsSmsImpl;

    private int mToken = 1;
    private int mMessageRef = 1;
    private int mResult = ImsSmsImplBase.SEND_STATUS_OK;
    private byte[] mPdu = {21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2,
            99, (byte) 0x6C, 26, 03};

    @Before
    public void setUp() throws Exception {
        mImsCallContext = Mockito.mock(ImsCallContext.class);
        mSmsTransferLayer = Mockito.mock(SmsTransferLayer.class);
        mImsSmsImpl = new ImsSmsImpl(mImsCallContext, mSmsTransferLayer);
        mImsSmsImpl.onReady();
    }

    @Test
    public void test_acknowledgeSms() {
        mImsSmsImpl.acknowledgeSms(mToken, mMessageRef, mResult);
        verify(mSmsTransferLayer).sendReportTPdu(mToken, SmsUtils.TP_SMS_DELIVER, mMessageRef,
                mResult);
    }

    @Test
    public void test_acknowledgeSmsReport() {
        mImsSmsImpl.acknowledgeSmsReport(mToken, mMessageRef, mResult);
        verify(mSmsTransferLayer).sendReportTPdu(mToken, SmsUtils.TP_SMS_STATUS_REPORT, mMessageRef,
                mResult);
    }

    private SmsTransferLayer.Listener setupListener() {
        ArgumentCaptor<SmsTransferLayer.Listener> callbackArg =
                ArgumentCaptor.forClass(SmsTransferLayer.Listener.class);
        verify(mSmsTransferLayer).setListener(callbackArg.capture());
        SmsTransferLayer.Listener mListener = callbackArg.getValue();
        assertNotNull(mListener);
        return mListener;
    }

    @Test
    public void test_notifySmsReceived() {
        SmsTransferLayer.Listener mListener = setupListener();
        mListener.notifySmsReceived(mToken, SmsUtils.FORMAT_INT_3GPP, SmsUtils.TP_SMS_DELIVER,
                mPdu);
        mSmsTransferLayer.sendReportTPdu(mToken, SmsUtils.TP_SMS_STATUS_REPORT, mMessageRef,
                ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED);
        verify(mSmsTransferLayer).sendReportTPdu(mToken, SmsUtils.TP_SMS_STATUS_REPORT, mMessageRef,
                ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallContext = null;
        mImsSmsImpl = null;
        mSmsTransferLayer = null;
    }
}
