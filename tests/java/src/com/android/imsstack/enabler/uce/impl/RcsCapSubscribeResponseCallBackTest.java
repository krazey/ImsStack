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
public class RcsCapSubscribeResponseCallBackTest {
    private static final String TEST_PHONE_NUMBER = "+123456789";
    @Mock
    private RcsCapabilityExchangeImplBase.SubscribeResponseCallback mSubscribeResponseCallback;

    private RcsCapSubscribeResponseCallBack mRcsCapSubscribeResponseCallBack;
    private RcsCapSubscribeResponseCallBack mRcsSubscribeResponseCallBackNull;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mRcsCapSubscribeResponseCallBack = new RcsCapSubscribeResponseCallBack(mExecutor);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsSubscribeResponseCallBackNull = new RcsCapSubscribeResponseCallBack(mExecutor);
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
    }

    @After
    public void tearDown() {
        mSubscribeResponseCallback = null;
        mRcsCapSubscribeResponseCallBack = null;
    }

    private final Executor mExecutor = (r) -> r.run();

    @Test
    public void onCommandErrorSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onCommandError(1);
        verify(mSubscribeResponseCallback, never()).onCommandError(1);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onCommandError(1);
        verify(mSubscribeResponseCallback).onCommandError(1);
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onCommandError(1);
        mRcsCapSubscribeResponseCallBack.onCommandError(1);
    }

    @Test
    public void onNetworkResponseSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onNetworkResponse(200, "OK");
        verify(mSubscribeResponseCallback, never())
                .onNetworkResponse(anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK");
        verify(mSubscribeResponseCallback).onNetworkResponse(200, "OK");
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onNetworkResponse(anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK");
    }

    @Test
    public void onNetworkResponseTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onNetworkResponse(200, "OK", 0, null);
        verify(mSubscribeResponseCallback, never())
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK", 400, "Bad Request");
        verify(mSubscribeResponseCallback).onNetworkResponse(200, "OK", 400, "Bad Request");
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK", 400, "Bad Request");
    }

    @Test
    public void onNotifyCapabilitiesUpdateSubscribeTest() throws ImsException {
        List<String> pidfs = new ArrayList();
        pidfs.add(readPidf());
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onNotifyCapabilitiesUpdate(pidfs);
        verify(mSubscribeResponseCallback, never()).onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(pidfs);
        verify(mSubscribeResponseCallback).onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(null);
        verify(mSubscribeResponseCallback).onNotifyCapabilitiesUpdate(null);
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(pidfs);
    }

    @Test
    public void onTerminatedSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onTerminated("terminatedReason", 1200);
        verify(mSubscribeResponseCallback, never()).onTerminated("terminatedReason", 1200);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onTerminated("terminatedReason", 1200);
        verify(mSubscribeResponseCallback).onTerminated("terminatedReason", 1200);
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onTerminated("terminatedReason", 1200);
        mRcsCapSubscribeResponseCallBack.onTerminated("terminatedReason", 1200);
    }

    @Test
    public void onResourceTerminatedSubscribeTest() throws ImsException {
        List<Pair<Uri, String>> terminatedReasons = new ArrayList<Pair<Uri, String>>();
        Uri uri = Uri.parse(TEST_PHONE_NUMBER);
        terminatedReasons.add(new Pair<>(uri, "terminatedReason"));
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onResourceTerminated(terminatedReasons);
        verify(mSubscribeResponseCallback, never())
                .onResourceTerminated(terminatedReasons);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onResourceTerminated(terminatedReasons);
        verify(mSubscribeResponseCallback).onResourceTerminated(terminatedReasons);
        doThrow(ImsException.class)
                .when(mSubscribeResponseCallback)
                .onResourceTerminated(terminatedReasons);
        mRcsCapSubscribeResponseCallBack.onResourceTerminated(terminatedReasons);
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
