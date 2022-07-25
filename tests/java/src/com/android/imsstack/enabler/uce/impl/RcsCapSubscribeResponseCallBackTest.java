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

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;
import android.util.Pair;

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
public class RcsCapSubscribeResponseCallBackTest {
    @Mock
    private RcsCapabilityExchangeImplBase.SubscribeResponseCallback mSubscribeResponseCallback;
    private MessageExecutor mMessageExecutor;
    private RcsCapSubscribeResponseCallBack mRcsCapSubscribeResponseCallBack,
            mRcsSubscribeResponseCallBackNull;
    private static final String TEST_PHONE_NUMBER = "+123456789";

    @Before
    public void setUp() {
        mSubscribeResponseCallback = Mockito.mock(
                RcsCapabilityExchangeImplBase.SubscribeResponseCallback.class);
        mMessageExecutor = new MessageExecutor("RcsUceCAllBack");
        mRcsCapSubscribeResponseCallBack = new RcsCapSubscribeResponseCallBack(mMessageExecutor);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsSubscribeResponseCallBackNull = new RcsCapSubscribeResponseCallBack(mMessageExecutor);
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
    }

    @Test
    public void onCommandErrorSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onCommandError(1);
        verify(mSubscribeResponseCallback, Mockito.times(0)).onCommandError(1);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onCommandError(1);
        verify(mSubscribeResponseCallback, Mockito.timeout(100).times(1)).onCommandError(1);
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onCommandError(1);
        mRcsCapSubscribeResponseCallBack.onCommandError(1);
        reset(mSubscribeResponseCallback);
    }

    @Test
    public void onNetworkResponseSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onNetworkResponse(200, "OK");
        verify(mSubscribeResponseCallback, Mockito.times(0)).onNetworkResponse(
                anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK");
        verify(mSubscribeResponseCallback, Mockito.timeout(100).times(1)).onNetworkResponse(200,
                "OK");
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onNetworkResponse(anyInt(),
                anyString());
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK");
        reset(mSubscribeResponseCallback);
    }

    @Test
    public void onNetworkResponseTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onNetworkResponse(200, "OK", 0, null);
        verify(mSubscribeResponseCallback, Mockito.times(0))
                .onNetworkResponse(anyInt(), anyString(), anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK",
                400, "Bad Request");
        verify(mSubscribeResponseCallback, Mockito.timeout(100).times(1))
                .onNetworkResponse(200, "OK", 400, "Bad Request");
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onNetworkResponse(anyInt(),
                anyString(), anyInt(), anyString());
        mRcsCapSubscribeResponseCallBack.onNetworkResponse(200, "OK",
                400, "Bad Request");
        reset(mSubscribeResponseCallback);
    }

    @Test
    public void onNotifyCapabilitiesUpdateSubscribeTest() throws ImsException {
        List<String> pidfs = new ArrayList();
        pidfs.add(readpidf());
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onNotifyCapabilitiesUpdate(pidfs);
        verify(mSubscribeResponseCallback,
                Mockito.times(0)).onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(pidfs);
        verify(mSubscribeResponseCallback,
                Mockito.timeout(100).times(1)).onNotifyCapabilitiesUpdate(pidfs);
        mRcsCapSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(null);
        verify(mSubscribeResponseCallback,
                Mockito.timeout(100).times(1)).onNotifyCapabilitiesUpdate(null);
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onNotifyCapabilitiesUpdate(
                pidfs);
        mRcsCapSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(pidfs);
        reset(mSubscribeResponseCallback);
    }

    @Test
    public void onTerminatedSubscribeTest() throws ImsException {
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onTerminated("terminatedReason", 1200);
        verify(mSubscribeResponseCallback, Mockito.times(0))
                .onTerminated("terminatedReason", 1200);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onTerminated("terminatedReason", 1200);
        verify(mSubscribeResponseCallback, Mockito.timeout(100)
                .times(1)).onTerminated("terminatedReason", 1200);
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onTerminated(
                "terminatedReason", 1200);
        mRcsCapSubscribeResponseCallBack.onTerminated("terminatedReason", 1200);
        reset(mSubscribeResponseCallback);
    }

    @Test
    public void onResourceTerminatedSubscribeTest() throws ImsException {
        List<Pair<Uri, String>> terminatedReasons = new ArrayList<Pair<Uri, String>>();
        Uri uri = Uri.parse(TEST_PHONE_NUMBER);
        terminatedReasons.add(new Pair<>(uri, "terminatedReason"));
        mRcsSubscribeResponseCallBackNull.setCallBack(null);
        mRcsSubscribeResponseCallBackNull.onResourceTerminated(terminatedReasons);
        verify(mSubscribeResponseCallback, Mockito.times(0))
                .onResourceTerminated(terminatedReasons);
        mRcsCapSubscribeResponseCallBack.setCallBack(mSubscribeResponseCallback);
        mRcsCapSubscribeResponseCallBack.onResourceTerminated(terminatedReasons);
        verify(mSubscribeResponseCallback, Mockito.timeout(100).times(1))
                .onResourceTerminated(terminatedReasons);
        doThrow(ImsException.class).when(mSubscribeResponseCallback).onResourceTerminated(
                terminatedReasons);
        mRcsCapSubscribeResponseCallBack.onResourceTerminated(terminatedReasons);
        reset(mSubscribeResponseCallback);
    }

    public String readpidf() {
        String pidfBuilder = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
                + "<presence entity=\"" + "sip:0123@ims.mnc001.mcc001.3gppnetwork.org" + "\""
                + " xmlns=\"urn:ietf:params:xml:ns:pidf\""
                + " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\""
                + " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\">"
                + "<tuple id=\"tid0\">"
                // status
                + "<status><basic>" + "open" + "</basic></status>"
                // service description
                + "<op:service-description>"
                + "<op:service-id>service_id_01</op:service-id>"
                + "<op:version>1.0</op:version>"
                + "<op:description>" + "Capabilities Discovery Service" + "</op:description>"
                + "</op:service-description>"
                // service capabilities
                + "<caps:servcaps>"
                // audio capabilities
                + "<caps:audio>" + "true" + "</caps:audio>"
                // video capabilities
                + "<caps:video>" + "true" + "</caps:video>"
                // duplex mode
                + "<caps:duplex>"
                // support duplex mode
                + "<caps:supported>"
                + "<caps:" + "duplex" + "/>"
                + "</caps:supported>"
                // unsupported duplex mode
                + "<caps:notsupported>"
                + "<caps:" + "full" + "/>"
                + "</caps:notsupported>"
                + "</caps:duplex>"
                + "</caps:servcaps>"
                + "<contact>" + "sip:0123@ims.mnc001.mcc001.3gppnetwork.org" + "</contact>"
                + "</tuple>"
                + "</presence>";
        return pidfBuilder;
    }

    @After
    public void cleanUp() {
        mSubscribeResponseCallback = null;
        mMessageExecutor = null;
        mRcsCapSubscribeResponseCallBack = null;
    }
}
