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

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;
import android.util.Pair;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class RcsCapSubscribeResponseCallbackTest {
    private static final String TEST_PHONE_NUMBER = "+123456789";
    @Mock
    private RcsCapabilityExchangeImplBase.SubscribeResponseCallback mSubscribeResponseCallback;

    private RcsCapSubscribeResponseCallback mRcsCapSubscribeResponseCallback;
    private RcsCapSubscribeResponseCallback mRcsSubscribeResponseCallbackNull;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mRcsCapSubscribeResponseCallback = new RcsCapSubscribeResponseCallback(mExecutor);
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsSubscribeResponseCallbackNull = new RcsCapSubscribeResponseCallback(mExecutor);
        mRcsSubscribeResponseCallbackNull.setCallback(null);
    }

    @After
    public void tearDown() {
        mSubscribeResponseCallback = null;
        mRcsCapSubscribeResponseCallback = null;
    }

    private final Executor mExecutor = (r) -> r.run();

    @Test
    public void onCommandErrorSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallbackNull.setCallback(null);
        mRcsSubscribeResponseCallbackNull.onCommandError(1);
        verify(mSubscribeResponseCallback, never()).onCommandError(1);
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallback.onCommandError(1);
        verify(mSubscribeResponseCallback).onCommandError(1);
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onCommandError(1);
        mRcsCapSubscribeResponseCallback.onCommandError(1);
    }

    @Test
    public void onNetworkResponseSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallbackNull.setCallback(null);
        mRcsSubscribeResponseCallbackNull.onNetworkResponse(200, "OK");
        verify(mSubscribeResponseCallback, never())
                .onNetworkResponse(anyInt(), anyString());
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallback.onNetworkResponse(200, "OK");
        verify(mSubscribeResponseCallback).onNetworkResponse(200, "OK");
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onNetworkResponse(anyInt(), anyString());
        mRcsCapSubscribeResponseCallback.onNetworkResponse(200, "OK");
    }

    @Test
    public void onNetworkResponseTest() throws ImsException {
        mRcsSubscribeResponseCallbackNull.setCallback(null);
        mRcsSubscribeResponseCallbackNull.onNetworkResponse(200, "OK", 0, null);
        verify(mSubscribeResponseCallback, never())
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallback.onNetworkResponse(200, "OK", 400, "Bad Request");
        verify(mSubscribeResponseCallback).onNetworkResponse(200, "OK", 400, "Bad Request");
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapSubscribeResponseCallback.onNetworkResponse(200, "OK", 400, "Bad Request");
    }

    @Test
    public void onNotifyCapabilitiesUpdateSubscribeTest() throws ImsException {
        List<String> pidfs = new ArrayList();
        pidfs.add(readPidf());
        mRcsSubscribeResponseCallbackNull.setCallback(null);
        mRcsSubscribeResponseCallbackNull.onNotifyCapabilitiesUpdate(pidfs);
        verify(mSubscribeResponseCallback, never()).onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallback.onNotifyCapabilitiesUpdate(pidfs);
        verify(mSubscribeResponseCallback).onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallback.onNotifyCapabilitiesUpdate(null);
        verify(mSubscribeResponseCallback).onNotifyCapabilitiesUpdate(null);
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallback.onNotifyCapabilitiesUpdate(pidfs);
    }

    @Test
    public void onTerminatedSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallbackNull.setCallback(null);
        mRcsSubscribeResponseCallbackNull.onTerminated("terminatedReason", 1200);
        verify(mSubscribeResponseCallback, never()).onTerminated("terminatedReason", 1200);
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallback.onTerminated("terminatedReason", 1200);
        verify(mSubscribeResponseCallback).onTerminated("terminatedReason", 1200);
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onTerminated("terminatedReason", 1200);
        mRcsCapSubscribeResponseCallback.onTerminated("terminatedReason", 1200);
    }

    @Test
    public void onResourceTerminatedSubscribeTest() throws ImsException {
        List<Pair<Uri, String>> terminatedReasons = new ArrayList<Pair<Uri, String>>();
        Uri uri = Uri.parse(TEST_PHONE_NUMBER);
        terminatedReasons.add(new Pair<>(uri, "terminatedReason"));
        mRcsSubscribeResponseCallbackNull.setCallback(null);
        mRcsSubscribeResponseCallbackNull.onResourceTerminated(terminatedReasons);
        verify(mSubscribeResponseCallback, never())
                .onResourceTerminated(terminatedReasons);
        mRcsCapSubscribeResponseCallback.setCallback(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallback.onResourceTerminated(terminatedReasons);
        verify(mSubscribeResponseCallback).onResourceTerminated(terminatedReasons);
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onResourceTerminated(terminatedReasons);
        mRcsCapSubscribeResponseCallback.onResourceTerminated(terminatedReasons);
    }

    private static String readPidf() {
        String pidfBuilder =
                "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
                        + "<presence entity=\""
                        + "sip:0123@ims.mnc001.mcc001.3gppnetwork.org"
                        + "\""
                        + " xmlns=\"urn:ietf:params:xml:ns:pidf\""
                        + " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\""
                        + " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\">"
                        + "<tuple id=\"tid0\">"
                        // status
                        + "<status><basic>"
                        + "open"
                        + "</basic></status>"
                        // service description
                        + "<op:service-description>"
                        + "<op:service-id>service_id_01</op:service-id>"
                        + "<op:version>1.0</op:version>"
                        + "<op:description>"
                        + "Capabilities Discovery Service"
                        + "</op:description>"
                        + "</op:service-description>"
                        // service capabilities
                        + "<caps:servcaps>"
                        // audio capabilities
                        + "<caps:audio>"
                        + "true"
                        + "</caps:audio>"
                        // video capabilities
                        + "<caps:video>"
                        + "true"
                        + "</caps:video>"
                        // duplex mode
                        + "<caps:duplex>"
                        // support duplex mode
                        + "<caps:supported>"
                        + "<caps:"
                        + "duplex"
                        + "/>"
                        + "</caps:supported>"
                        // unsupported duplex mode
                        + "<caps:notsupported>"
                        + "<caps:"
                        + "full"
                        + "/>"
                        + "</caps:notsupported>"
                        + "</caps:duplex>"
                        + "</caps:servcaps>"
                        + "<contact>"
                        + "sip:0123@ims.mnc001.mcc001.3gppnetwork.org"
                        + "</contact>"
                        + "</tuple>"
                        + "</presence>";
        return pidfBuilder;
    }
}
