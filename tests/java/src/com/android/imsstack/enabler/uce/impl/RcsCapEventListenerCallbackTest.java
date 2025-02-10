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
import static org.mockito.Mockito.verify;

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.stub.CapabilityExchangeEventListener;

import com.android.imsstack.enabler.uce.interf.RemoteOptionsCallback;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class RcsCapEventListenerCallbackTest {
    private static final String TEST_PHONE_NUMBER = "+123456789";
    private static final String FEATURE_VIDEO = "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video";

    @Mock CapabilityExchangeEventListener mListener;
    @Mock RemoteOptionsCallback mRemoteOptionsCallback;
    @Mock RcsCapOptionsResponseCallback mRcsCapOptionsResponseCallback;
    @Mock Executor mMessageExecutorRequest;
    @Mock RcsCapOptionsRequestCallback mRcsCapOptionsRequestCallback;

    private RcsCapEventListenerCallback mRcsCapEventListenerCallback;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mRcsCapEventListenerCallback =
                new RcsCapEventListenerCallback(
                        mListener,
                        mExecutor,
                        mMessageExecutorRequest,
                        mRcsCapOptionsRequestCallback);
    }

    @After
    public void tearDown() throws Exception {
        mRcsCapEventListenerCallback = null;
    }

    private final Executor mExecutor = (r) -> r.run();

    @Test
    public void onRequestPublishCapabilitiesTest() throws ImsException {
        int publishTriggerType = CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED;
        mRcsCapEventListenerCallback.onRequestPublishCapabilities(publishTriggerType);
        verify(mListener).onRequestPublishCapabilities(publishTriggerType);
        publishTriggerType = CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD;
        mRcsCapEventListenerCallback.onRequestPublishCapabilities(publishTriggerType);
        verify(mListener).onRequestPublishCapabilities(publishTriggerType);
        doThrow(ImsException.class)
                .when(mListener)
                .onRequestPublishCapabilities(publishTriggerType);
        mRcsCapEventListenerCallback.onRequestPublishCapabilities(publishTriggerType);
    }

    @Test
    public void onPublishUpdatedTest() throws ImsException {
        mRcsCapEventListenerCallback.onPublishUpdated(200, null, 0, null);
        verify(mListener).onPublishUpdated(200, null, 0, null);
        mRcsCapEventListenerCallback.onPublishUpdated(200, "", 400, "Bad reason");
        verify(mListener).onPublishUpdated(200, "", 400, "Bad reason");
        doThrow(ImsException.class).when(mListener).onPublishUpdated(200, null, 0, null);
        mRcsCapEventListenerCallback.onPublishUpdated(200, null, 0, null);
    }

    @Test
    public void onUnPublishTest() throws ImsException {
        mRcsCapEventListenerCallback.onUnPublish();
        verify(mListener).onUnpublish();
        doThrow(ImsException.class).when(mListener).onUnpublish();
        mRcsCapEventListenerCallback.onUnPublish();
    }

    @Test
    public void onRemoteCapabilityRequestTest() throws ImsException {
        RemoteOptionsCallback callback = mock(RemoteOptionsCallback.class);
        Set<String> remoteCapabilities = new HashSet<>();
        remoteCapabilities.add(FEATURE_VIDEO);
        mRcsCapEventListenerCallback.onRemoteCapabilityRequest(
                Uri.parse(TEST_PHONE_NUMBER), remoteCapabilities, callback);
        verify(mRcsCapOptionsRequestCallback).setCallback(callback);
        verify(mListener)
                .onRemoteCapabilityRequest(
                        Uri.parse(TEST_PHONE_NUMBER),
                        remoteCapabilities,
                        mRcsCapOptionsRequestCallback);
        doThrow(ImsException.class)
                .when(mListener)
                .onRemoteCapabilityRequest(
                        Uri.parse(TEST_PHONE_NUMBER),
                        remoteCapabilities,
                        mRcsCapOptionsRequestCallback);
        mRcsCapEventListenerCallback.onRemoteCapabilityRequest(
                Uri.parse(TEST_PHONE_NUMBER), remoteCapabilities, callback);
    }
}
