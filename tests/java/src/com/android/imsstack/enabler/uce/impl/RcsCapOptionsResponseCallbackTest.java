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

import static org.mockito.ArgumentMatchers.any;
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

import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class RcsCapOptionsResponseCallbackTest {
    @Mock private RcsCapabilityExchangeImplBase.OptionsResponseCallback mOptionsResponseCallback;
    private RcsCapOptionsResponseCallback mRcsCapOptionsResponseCallback;
    private RcsCapOptionsResponseCallback mRcsCapOptionsResponseCallbackNull;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mRcsCapOptionsResponseCallback = new RcsCapOptionsResponseCallback(mExecutor);
        mRcsCapOptionsResponseCallback.setCallback(mOptionsResponseCallback);
        mRcsCapOptionsResponseCallbackNull = new RcsCapOptionsResponseCallback(mExecutor);
        mRcsCapOptionsResponseCallbackNull.setCallback(null);
    }

    @After
    public void tearDown() {
        mOptionsResponseCallback = null;
        mRcsCapOptionsResponseCallback = null;
    }

    private final Executor mExecutor = (r) -> r.run();

    @Test
    public void onCommandErrorOptionsTest() throws ImsException {
        mRcsCapOptionsResponseCallbackNull.setCallback(null);
        mRcsCapOptionsResponseCallbackNull.onCommandError(1);
        verify(mOptionsResponseCallback, never()).onCommandError(1);
        mRcsCapOptionsResponseCallback.setCallback(mOptionsResponseCallback);
        mRcsCapOptionsResponseCallback.onCommandError(1);
        verify(mOptionsResponseCallback).onCommandError(1);
        doThrow(ImsException.class).when(mOptionsResponseCallback).onCommandError(1);
        mRcsCapOptionsResponseCallback.onCommandError(1);
    }

    @Test
    public void onNetworkResponseOptionsTest() throws ImsException {
        List<String> capability = readCapabilities();
        mRcsCapOptionsResponseCallbackNull.setCallback(null);
        mRcsCapOptionsResponseCallbackNull.onNetworkResponse(200, "OK", capability);
        verify(mOptionsResponseCallback, never())
                .onNetworkResponse(anyInt(), anyString(), any());
        mRcsCapOptionsResponseCallback.setCallback(mOptionsResponseCallback);
        mRcsCapOptionsResponseCallback.onNetworkResponse(200, "OK", capability);
        verify(mOptionsResponseCallback).onNetworkResponse(anyInt(), anyString(), any());
        doThrow(ImsException.class)
                .when(mOptionsResponseCallback)
                .onNetworkResponse(anyInt(), anyString(), any());
        mRcsCapOptionsResponseCallback.onNetworkResponse(200, "OK", capability);
    }

    private static List<String> readCapabilities() {
        return List.of("Audio", "Video");
    }
}
