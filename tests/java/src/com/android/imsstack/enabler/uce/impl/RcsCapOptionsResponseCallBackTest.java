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

import java.util.ArrayList;
import java.util.List;

@RunWith(JUnit4.class)
public class RcsCapOptionsResponseCallBackTest {
    @Mock
    private RcsCapabilityExchangeImplBase.OptionsResponseCallback mOptionsResponseCallback;
    private MessageExecutor mMessageExecutor;
    private RcsCapOptionsResponseCallBack mRcsCapOptionsResponseCallBack,
            mRcsCapOptionsResponseCallBackNull;
    @Before
    public void setUp() {
        mOptionsResponseCallback = Mockito.mock(
                RcsCapabilityExchangeImplBase.OptionsResponseCallback.class);
        mMessageExecutor = new MessageExecutor("ResponseCallBackTest");
        mRcsCapOptionsResponseCallBack = new RcsCapOptionsResponseCallBack(mMessageExecutor);
        mRcsCapOptionsResponseCallBack.setCallBack(mOptionsResponseCallback);
        mRcsCapOptionsResponseCallBackNull = new RcsCapOptionsResponseCallBack(mMessageExecutor);
        mRcsCapOptionsResponseCallBackNull.setCallBack(null);
    }
    @Test
    public void onCommandErrorOptionsTest() throws ImsException {
        mRcsCapOptionsResponseCallBackNull.setCallBack(null);
        mRcsCapOptionsResponseCallBackNull.onCommandError(1);
        verify(mOptionsResponseCallback, Mockito.times(0)).onCommandError(1);
        mRcsCapOptionsResponseCallBack.setCallBack(mOptionsResponseCallback);
        mRcsCapOptionsResponseCallBack.onCommandError(1);
        verify(mOptionsResponseCallback, Mockito.timeout(100).times(1)).onCommandError(1);
        doThrow(ImsException.class).when(mOptionsResponseCallback).onCommandError(1);
        mRcsCapOptionsResponseCallBack.onCommandError(1);
        reset(mOptionsResponseCallback);
    }
    @Test
    public void onNetworkResponseOptionsTest() throws ImsException {
        List<String> capa = readCapabilities();
        mRcsCapOptionsResponseCallBackNull.setCallBack(null);
        mRcsCapOptionsResponseCallBackNull.onNetworkResponse(200, "OK", capa);
        verify(mOptionsResponseCallback, Mockito.times(0)).onNetworkResponse(
                anyInt(), anyString(), any());
        mRcsCapOptionsResponseCallBack.setCallBack(mOptionsResponseCallback);
        mRcsCapOptionsResponseCallBack.onNetworkResponse(200, "OK", capa);
        verify(mOptionsResponseCallback, Mockito.timeout(100).times(1)).onNetworkResponse(
                anyInt(), anyString(), any());
        doThrow(ImsException.class).when(mOptionsResponseCallback).onNetworkResponse(anyInt(),
                anyString(), any());
        mRcsCapOptionsResponseCallBack.onNetworkResponse(200, "OK", capa);
        reset(mOptionsResponseCallback);
    }
    public List<String> readCapabilities() {
        List<String> capabilities = new ArrayList();
        capabilities.add("Audio");
        capabilities.add("Video");
        return capabilities;
    }
    @After
    public void cleanUp() {
        mOptionsResponseCallback = null;
        mMessageExecutor = null;
        mRcsCapOptionsResponseCallBack = null;
    }
}
