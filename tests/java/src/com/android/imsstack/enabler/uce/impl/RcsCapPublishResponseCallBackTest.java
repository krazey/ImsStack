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

package com.android.imsstack.enabler.uce.impl;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;

import android.telephony.ims.ImsException;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class RcsCapPublishResponseCallBackTest {
    @Mock
    private RcsCapabilityExchangeImplBase.PublishResponseCallback mPublishResponseCallback;
    private MessageExecutor mMessageExecutor;
    private RcsCapPublishResponseCallBack mRcsCapPublishResponseCallBack,
            mRcsCapPublishResponseCallBackNull;

    @Before
    public void setUp() {
        mPublishResponseCallback = Mockito.mock(
                RcsCapabilityExchangeImplBase.PublishResponseCallback.class);
        mMessageExecutor = new MessageExecutor("ResponseCallBackTest");
        mRcsCapPublishResponseCallBack = new RcsCapPublishResponseCallBack(mMessageExecutor);
        mRcsCapPublishResponseCallBack.setCallBack(mPublishResponseCallback);
        mRcsCapPublishResponseCallBackNull = new RcsCapPublishResponseCallBack(mMessageExecutor);
        mRcsCapPublishResponseCallBackNull.setCallBack(null);
    }

    @Test
    public void onCommandErrorPublishTest() throws ImsException {
        mRcsCapPublishResponseCallBackNull.setCallBack(null);
        mRcsCapPublishResponseCallBackNull.onCommandError(1);
        verify(mPublishResponseCallback, Mockito.times(0)).onCommandError(1);

        mRcsCapPublishResponseCallBack.setCallBack(mPublishResponseCallback);
        mRcsCapPublishResponseCallBack.onCommandError(1);
        verify(mPublishResponseCallback, Mockito.timeout(100).times(1)).onCommandError(1);

        doThrow(ImsException.class).when(mPublishResponseCallback).onCommandError(1);
        mRcsCapPublishResponseCallBack.onCommandError(1);
        reset(mPublishResponseCallback);
    }

    @Test
    public void onNetworkResponsePublishTest() throws ImsException {
        mRcsCapPublishResponseCallBackNull.setCallBack(null);
        mRcsCapPublishResponseCallBackNull.onNetworkResponse(200, "OK");
        verify(mPublishResponseCallback, Mockito.times(0)).onNetworkResponse(
                anyInt(), anyString());
        mRcsCapPublishResponseCallBack.setCallBack(mPublishResponseCallback);
        mRcsCapPublishResponseCallBack.onNetworkResponse(200, "OK");
        verify(mPublishResponseCallback, Mockito.timeout(100).times(1)).onNetworkResponse(200,
                "OK");

        doThrow(ImsException.class).when(mPublishResponseCallback).onNetworkResponse(anyInt(),
                anyString());
        mRcsCapPublishResponseCallBack.onNetworkResponse(200, "OK");
        reset(mPublishResponseCallback);
    }

    @Test
    public void onNetworkResponseReasonPublishTest() throws ImsException {
        mRcsCapPublishResponseCallBackNull.setCallBack(null);
        mRcsCapPublishResponseCallBackNull.onNetworkResponse(200, "OK", 100, "Reason");
        verify(mPublishResponseCallback, Mockito.timeout(100).times(0))
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapPublishResponseCallBack.setCallBack(mPublishResponseCallback);
        mRcsCapPublishResponseCallBack.onNetworkResponse(200, "OK", 100, "Reason");
        verify(mPublishResponseCallback, Mockito.timeout(100).times(1))
                .onNetworkResponse(200, "OK", 100, "Reason");

        doThrow(ImsException.class).when(mPublishResponseCallback)
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapPublishResponseCallBack.onNetworkResponse(200, "OK", 100, "Reason");
        reset(mPublishResponseCallback);
    }

    @After
    public void cleanUp() {
        mPublishResponseCallback = null;
        mMessageExecutor = null;
        mRcsCapPublishResponseCallBack = null;
    }
}
