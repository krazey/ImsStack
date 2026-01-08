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
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;

import android.telephony.ims.ImsException;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class RcsCapPublishResponseCallbackTest {
    @Mock private RcsCapabilityExchangeImplBase.PublishResponseCallback mPublishResponseCallback;
    private RcsCapPublishResponseCallback mRcsCapPublishResponseCallback;
    private RcsCapPublishResponseCallback mRcsCapPublishResponseCallbackNull;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mRcsCapPublishResponseCallback = new RcsCapPublishResponseCallback(mExecutor);
        mRcsCapPublishResponseCallback.setCallback(mPublishResponseCallback);
        mRcsCapPublishResponseCallbackNull = new RcsCapPublishResponseCallback(mExecutor);
        mRcsCapPublishResponseCallbackNull.setCallback(null);
    }

    @After
    public void tearDown() {
        mPublishResponseCallback = null;
        mRcsCapPublishResponseCallback = null;
    }

    private final Executor mExecutor = (r) -> r.run();

    @Test
    public void onCommandErrorPublishTest() throws ImsException {
        mRcsCapPublishResponseCallbackNull.setCallback(null);
        mRcsCapPublishResponseCallbackNull.onCommandError(1);
        verify(mPublishResponseCallback, never()).onCommandError(1);

        mRcsCapPublishResponseCallback.setCallback(mPublishResponseCallback);
        mRcsCapPublishResponseCallback.onCommandError(1);
        verify(mPublishResponseCallback).onCommandError(1);

        doThrow(ImsException.class).when(mPublishResponseCallback).onCommandError(1);
        mRcsCapPublishResponseCallback.onCommandError(1);
    }

    @Test
    public void onNetworkResponsePublishTest() throws ImsException {
        mRcsCapPublishResponseCallbackNull.setCallback(null);
        mRcsCapPublishResponseCallbackNull.onNetworkResponse(200, "OK");
        verify(mPublishResponseCallback, never()).onNetworkResponse(anyInt(), anyString());
        mRcsCapPublishResponseCallback.setCallback(mPublishResponseCallback);
        mRcsCapPublishResponseCallback.onNetworkResponse(200, "OK");
        verify(mPublishResponseCallback).onNetworkResponse(200, "OK");

        doThrow(ImsException.class)
                .when(mPublishResponseCallback)
                .onNetworkResponse(anyInt(), anyString());
        mRcsCapPublishResponseCallback.onNetworkResponse(200, "OK");
    }

    @Test
    public void onNetworkResponseReasonPublishTest() throws ImsException {
        mRcsCapPublishResponseCallbackNull.setCallback(null);
        mRcsCapPublishResponseCallbackNull.onNetworkResponse(200, "OK", 100, "Reason");
        verify(mPublishResponseCallback, never())
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapPublishResponseCallback.setCallback(mPublishResponseCallback);
        mRcsCapPublishResponseCallback.onNetworkResponse(200, "OK", 100, "Reason");
        verify(mPublishResponseCallback).onNetworkResponse(200, "OK", 100, "Reason");

        doThrow(ImsException.class)
                .when(mPublishResponseCallback)
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapPublishResponseCallback.onNetworkResponse(200, "OK", 100, "Reason");
    }
}
