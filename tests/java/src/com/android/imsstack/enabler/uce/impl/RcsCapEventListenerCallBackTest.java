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

import static com.android.imsstack.enabler.uce.interf.UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD;
import static com.android.imsstack.enabler.uce.interf.UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED;

import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.stub.CapabilityExchangeEventListener;

import com.android.imsstack.enabler.uce.interf.RemoteOptionsCallback;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.util.HashSet;
import java.util.Set;
@RunWith(JUnit4.class)
public class RcsCapEventListenerCallBackTest {
    @Mock
    CapabilityExchangeEventListener mListener;
    @Mock
    RemoteOptionsCallback mRemoteOptionsCallback;
    @Mock
    RcsCapOptionsResponseCallBack mRcsCapOptionsResponseCallBack;
    @Mock
    MessageExecutor mMessageExecutor;
    @Mock
    MessageExecutor mMessageExecutorRequest;
    private static final String TEST_PHONE_NUMBER = "+123456789";
    private RcsCapEventListenerCallBack mRcsCapEventListenerCallBack;
    static final String FEATURE_VIDEO = "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video ";
    @Before
    public void setUp() throws Exception {
        mListener = Mockito.mock(CapabilityExchangeEventListener.class);
        mRemoteOptionsCallback = Mockito.mock(RemoteOptionsCallback.class);
        mRcsCapOptionsResponseCallBack = Mockito.mock(RcsCapOptionsResponseCallBack.class);
        mMessageExecutor = new MessageExecutor("RcsUceCAllBack");
        mMessageExecutorRequest = new MessageExecutor("RcsUceRequestCAllBack");
        mRcsCapEventListenerCallBack = new RcsCapEventListenerCallBack(mListener,
                mMessageExecutor, mMessageExecutorRequest);
    }
    @Test
    public void onRequestPublishCapabilitiesTest() throws ImsException {
        int publishTriggerType = CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED;
        mRcsCapEventListenerCallBack.onRequestPublishCapabilities(publishTriggerType);
        Mockito.verify(mListener, Mockito.timeout(100).times(1))
                        .onRequestPublishCapabilities(publishTriggerType);
        publishTriggerType = CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD;
        mRcsCapEventListenerCallBack.onRequestPublishCapabilities(publishTriggerType);
        Mockito.verify(mListener, Mockito.timeout(100).times(1))
                .onRequestPublishCapabilities(publishTriggerType);
        doThrow(ImsException.class).when(mListener)
                .onRequestPublishCapabilities(publishTriggerType);
        mRcsCapEventListenerCallBack.onRequestPublishCapabilities(publishTriggerType);
    }
    @Test
    public void onPublishUpdatedTest() throws ImsException {
        mRcsCapEventListenerCallBack.onPublishUpdated(200, null, 0, null);
        Mockito.verify(mListener, Mockito.timeout(100).times(1))
                .onPublishUpdated(200, null, 0, null);
        mRcsCapEventListenerCallBack.onPublishUpdated(200, "", 400, "Bad reason");
        Mockito.verify(mListener, Mockito.timeout(100).times(1))
                .onPublishUpdated(200, "", 400, "Bad reason");
        doThrow(ImsException.class).when(mListener).onPublishUpdated(200, null, 0, null);
        mRcsCapEventListenerCallBack.onPublishUpdated(200, null, 0, null);
    }
    @Test
    public void onUnPublishTest() throws ImsException {
        mRcsCapEventListenerCallBack.onUnPublish();
        Mockito.verify(mListener, Mockito.timeout(100).times(1)).onUnpublish();
        doThrow(ImsException.class).when(mListener).onUnpublish();
        mRcsCapEventListenerCallBack.onUnPublish();
    }
    @Test
    public void onRemoteCapabilityRequestTest() throws ImsException {
        CapabilityExchangeEventListener.OptionsRequestCallback optionsCallback = Mockito.mock(
                CapabilityExchangeEventListener.OptionsRequestCallback.class);
        RemoteOptionsCallback callback = mock(RemoteOptionsCallback.class);
        Set<String> remoteCapabilities = new HashSet<>();
        remoteCapabilities.add(FEATURE_VIDEO);
        mRcsCapEventListenerCallBack.onRemoteCapabilityRequest(Uri.parse(TEST_PHONE_NUMBER),
                remoteCapabilities, callback);
        mListener.onRemoteCapabilityRequest(Uri.parse(TEST_PHONE_NUMBER),
                remoteCapabilities, optionsCallback);
        Mockito.verify(mListener).onRemoteCapabilityRequest(Uri.parse(TEST_PHONE_NUMBER),
                remoteCapabilities, optionsCallback);
        doThrow(ImsException.class).when(mListener).onRemoteCapabilityRequest(
                Uri.parse(TEST_PHONE_NUMBER),
                remoteCapabilities, optionsCallback);
        mRcsCapEventListenerCallBack.onRemoteCapabilityRequest(Uri.parse(TEST_PHONE_NUMBER),
                remoteCapabilities, callback);
    }
    @After
    public void cleanUp() {
        mRcsCapEventListenerCallBack = null;
        mListener = null;
        mMessageExecutor = null;
        mRcsCapOptionsResponseCallBack = null;
    }
}
