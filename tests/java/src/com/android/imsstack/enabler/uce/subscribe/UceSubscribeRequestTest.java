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
package com.android.imsstack.enabler.uce.subscribe;

import android.net.Uri;
import android.os.Parcel;
import android.util.Pair;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.subscribe.UceResourceInfo;
import com.android.imsstack.enabler.uce.impl.subscribe.UceSubscribeRequest;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class UceSubscribeRequestTest {
    private static final int SLOT_ID = 0;
    private static final int KEY = 10;
    @Mock SubscribeResponse subscribeCb;
    @Mock UceJNI jni;

    @Before
    public void setUp(){
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp(){}

    @Test
    @SmallTest
    public void test_sendRequestWithEmptyUri() throws Exception {
        ArrayList<String> remoteUris = new ArrayList<>();

        UceSubscribeRequest request = createUceSubscribeRequest();

        // verify that send command error if the remote uri is empty
        request.sendRequest(remoteUris);
        verify(subscribeCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_INVALID_PARAM));
        verifyNoMoreInteractions(subscribeCb);
    }

    @Test
    @SmallTest
    public void test_normalSendRequest() throws Exception {
        ArrayList<String> remoteUris = new ArrayList<>();
        String remoteUri = "test";
        remoteUris.add(remoteUri);

        UceSubscribeRequest request = createUceSubscribeRequest();

        request.sendRequest(remoteUris);

        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);

        verify(jni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(parcel.readInt(), UceMessage.UCE_SEND_SINGLE_SUBSCRIBE_CMD);
        assertEquals(parcel.readInt(), KEY);
        assertEquals(parcel.readInt(), remoteUris.size());
        assertEquals(parcel.readString(), remoteUri);

        verifyNoMoreInteractions(jni);
    }

    @Test
    @SmallTest
    public void test_informResponse() throws Exception {
        int responseCode = 200;
        String reason = "OK";
        int reasonHdrCause = 0;
        String reasonHdrText = "reasonHdrText_TEST";

        UceSubscribeRequest request = createUceSubscribeRequest();
        request.informNetworkResponse(responseCode, reason, reasonHdrCause, reasonHdrText);

        verify(subscribeCb, times(1)).onNetworkResponse(eq(responseCode), eq(reason));
        verifyNoMoreInteractions(subscribeCb);
    }

    @Test
    @SmallTest
    public void test_informCommandCode() throws Exception {
        int commandCode = 200;

        UceSubscribeRequest request = createUceSubscribeRequest();
        request.informCommandError(commandCode);

        verify(subscribeCb, times(1)).onCommandError(eq(commandCode));
        verifyNoMoreInteractions(subscribeCb);
    }

    @Test
    @SmallTest
    public void test_informCapabilitiesUpdate() throws Exception {
        List<String> pidfXmls = new ArrayList<>();
        pidfXmls.add(getPidfxml("123456"));
        pidfXmls.add(getPidfxml("234567"));
        pidfXmls.add(getPidfxml("345678"));

        UceSubscribeRequest request = createUceSubscribeRequest();
        request.informCapabilitiesUpdate(pidfXmls);

        ArgumentCaptor<List> captor = ArgumentCaptor.forClass(List.class);
        verify(subscribeCb, times(1)).onNotifyCapabilitiesUpdate(captor.capture());
        List<String> data = captor.getValue();
        assertEquals(data.size(), pidfXmls.size());
        assertEquals(data.get(0), pidfXmls.get(0));
        assertEquals(data.get(1), pidfXmls.get(1));
        assertEquals(data.get(2), pidfXmls.get(2));
        verifyNoMoreInteractions(subscribeCb);
    }

    @Test
    @SmallTest
    public void test_informTerminate() throws Exception {
        String reason = "test";
        int retryAfterSecond = 10;

        UceSubscribeRequest request = createUceSubscribeRequest();
        request.informTerminate(reason, retryAfterSecond);

        verify(subscribeCb, times(1)).onTerminated(eq(reason),
                eq(TimeUnit.SECONDS.toMillis(retryAfterSecond)));
        verifyNoMoreInteractions(subscribeCb);
    }

    @Test
    @SmallTest
    public void test_informResourceTerminate() throws Exception {
        ArrayList<UceResourceInfo> resourceInfoList = new ArrayList<>();
        UceResourceInfo info1 = new UceResourceInfo("id1", "reason1");
        UceResourceInfo info2 = new UceResourceInfo("id2", "reason2");
        resourceInfoList.add(info1);
        resourceInfoList.add(info2);

        UceSubscribeRequest request = createUceSubscribeRequest();
        request.informResourceTerminate(resourceInfoList);

        ArgumentCaptor<ArrayList> captor = ArgumentCaptor.forClass(ArrayList.class);
        verify(subscribeCb, times(1)).onResourceTerminated(captor.capture());
        List<Pair<Uri, String>> data = captor.getValue();
        assertEquals(data.size(), resourceInfoList.size());
        Pair<Uri, String> pairData = data.get(0);
        assertNotNull("first pair data should not be null",pairData);
        assertEquals(pairData.first, Uri.parse(info1.getId()));
        assertEquals(pairData.second, info1.getReason());
        pairData = data.get(1);
        assertNotNull("second pair data should not be null",pairData);
        assertEquals(pairData.first, Uri.parse(info2.getId()));
        assertEquals(pairData.second, info2.getReason());
        verifyNoMoreInteractions(subscribeCb);
    }

    private UceSubscribeRequest createUceSubscribeRequest() {
        UceSubscribeRequest request = new UceSubscribeRequest(subscribeCb, SLOT_ID, KEY, jni);
        return request;
    }

    private String getPidfxml(String contact) {
        StringBuilder pidfBuilder = new StringBuilder();
        pidfBuilder.append("<?xml version='1.0' encoding='utf-8' standalone='yes' ?>")
                .append("<presence entity=\"").append("123456").append("\"")
                .append(" xmlns=\"urn:ietf:params:xml:ns:pidf\"")
                .append(" xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"")
                .append(" xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\">")
                .append("<tuple id=\"tid0\"><status><basic>open</basic></status>")
                .append("<op:service-description>")
                .append("<op:service-id>service_id_01</op:service-id>")
                .append("<op:version>1.0</op:version>")
                .append("<op:description>description_test1</op:description>")
                .append("</op:service-description>")
                .append("<contact>").append(contact).append("</contact>")
                .append("</tuple>")
                .append("<tuple id=\"tid1\"><status><basic>open</basic></status>")
                .append("<op:service-description>")
                .append("<op:service-id>service_id_02</op:service-id>")
                .append("<op:version>1.0</op:version>")
                .append("<op:description>description_test2</op:description>")
                .append("</op:service-description>")
                .append("<contact>").append(contact).append("</contact>")
                .append("</tuple></presence>");
        return pidfBuilder.toString();
    }
}
