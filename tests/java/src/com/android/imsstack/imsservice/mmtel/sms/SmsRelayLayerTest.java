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

package com.android.imsstack.imsservice.mmtel.sms;

import static org.junit.Assert.assertEquals;

import android.util.Base64;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class SmsRelayLayerTest {
    private static final String TAG = "Neon Test";

    @Mock ImsCallContext mImsCallContext;
    @Mock MtsController mMtsController;

    private SmsRelayLayer mSmsRelayLayer;
    private SmsRelayLayer.MtsControllerListenerProxy mProxyListener;

    private int mSmsFormat = SmsUtils.FORMAT_INT_3GPP;
    private int mToken = 1;
    private int mRpType = SmsUtils.RP_DATA;
    private String mSmsc = "+19037029920";
    private String mDestinationAddress = "0987654321";
    private byte[] mTpdu = {21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private byte[] mMtRpData = {01, 01, 07, (byte) 0x91, 91, 30, 07, 92, 29, (byte) 0xF0, 00, 12,
            21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2, 99,
            (byte) 0x6C, 26, 03};
    private byte[] mDeliverPdu = {20, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private String mEncodedData = Base64.encodeToString(mMtRpData, Base64.DEFAULT);

    @Before
    public void setUp() throws Exception {
        mImsCallContext = Mockito.mock(ImsCallContext.class);
        mMtsController = Mockito.mock(MtsController.class);
        mSmsRelayLayer = new SmsRelayLayer(mImsCallContext, mMtsController);
        mProxyListener = mSmsRelayLayer.new MtsControllerListenerProxy();
    }

    @Test
    public void test_sendRPMessageDefault() {
        assertEquals(SmsUtils.SMSRL_RESULT_EXCEPTION, mSmsRelayLayer.sendRPMessage(mToken, mRpType,
                mSmsc, mDestinationAddress, mTpdu));
    }

    @Test
    public void test_notifyIncomingMessageFailure() {
        assertEquals(mMtsController.MT_FAILURE,
                mProxyListener.notifyIncomingMessage(mSmsFormat, mEncodedData));
    }

    @After
    public void tearDown() throws Exception {
        mImsCallContext = null;
        mMtsController = null;
        mSmsRelayLayer = null;
        mProxyListener = null;
    }
}
