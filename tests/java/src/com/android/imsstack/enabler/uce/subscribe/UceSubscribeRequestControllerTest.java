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
import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.subscribe.UceResourceInfo;
import com.android.imsstack.enabler.uce.impl.subscribe.UceSubscribeRequest;
import com.android.imsstack.enabler.uce.impl.subscribe.UceSubscribeRequestController;
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

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.eq;

import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class UceSubscribeRequestControllerTest {
    private static final int MAX_WAIT_TIME = 1000;
    private static final int SLOT_ID = 0;
    @Mock SubscribeResponse subscribeCb;
    @Mock UceSubscribeRequest request;

    private final TestUceJni mUceJni = new TestUceJni();

    private class TestUceJni extends UceJNI {
        public IUceJNIListener mUceJniListener;
        public TestUceJni() {
            super();
            mUceJniListener = null;
        }

        @Override
        public void init(int nSimSlot) {};

        @Override
        public void release(int nSimSlot) {};

        @Override
        public void addListener(int nSimSlot, IUceJNIListener mListener, int nMsgType) {
            mUceJniListener = mListener;
        }
    }

    @Before
    public void setUp(){
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp(){}


    @Test
    @SmallTest
    public void test_subscribeCapabilities() throws Exception {
        Uri uri = Uri.parse("123456");
        Collection<Uri> contacts = new ArrayList<>();

        UceSubscribeRequestController controller = createUceSubscribeRequestController();

        controller.setImsRegistrationStatus(false);
        controller.subscribeCapabilities(contacts, subscribeCb, request);

        verify(subscribeCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE));

        controller.setImsRegistrationStatus(true);
        controller.subscribeCapabilities(contacts, subscribeCb, request);

        verify(subscribeCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_INVALID_PARAM));


        contacts.add(uri);
        controller.subscribeCapabilities(contacts, subscribeCb, request);

        ArgumentCaptor<ArrayList> captor = ArgumentCaptor.forClass(ArrayList.class);

        verify(request).sendRequest(captor.capture());
        List<String> data = captor.getValue();
        assertEquals(data.size(), 1);
        assertEquals(data.get(0), uri.toString());
        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedSubscribeResponse() throws Exception {
        int key = 10;
        int responseCode = 400;
        String reason = "test";
        int reasonCause = 10;
        String reasonCauseText = "reason text";

        UceSubscribeRequestController controller = createUceSubscribeRequestController();
        controller.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SUBSCRIBE_RESPONSE_IND);
        parcel.writeInt(key); // key
        parcel.writeInt(responseCode); // responseCode
        parcel.writeString(reason); // reason
        parcel.writeInt(reasonCause); // reasonHeaderCause
        parcel.writeString(reasonCauseText); // reasonHeaderText
        parcel.setDataPosition(0);

        mUceJni.mUceJniListener.onSubscribeResponseMessage(parcel);

        Handler handler = controller.getHandler();
        waitForHandlerActionDelayed(handler, MAX_WAIT_TIME, 0);

        verify(request, timeout(MAX_WAIT_TIME)).informNetworkResponse(eq(responseCode),
                eq(reason), eq(reasonCause), eq(reasonCauseText));
        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedPresenceNotify() throws Exception {
        int key = 10;
        int count = 1;
        String pidfxml = getPidfxml();

        UceSubscribeRequestController controller = createUceSubscribeRequestController();
        controller.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_PRESENCE_NOTIFY_IND);
        parcel.writeInt(key); // key
        parcel.writeInt(count); // count
        parcel.writeString(pidfxml); // pidfxml
        parcel.setDataPosition(0);

        mUceJni.mUceJniListener.onSubscribeResponseMessage(parcel);

        Handler handler = controller.getHandler();
        waitForHandlerActionDelayed(handler, MAX_WAIT_TIME, 0);

        ArgumentCaptor<List> captor = ArgumentCaptor.forClass(List.class);

        verify(request, timeout(MAX_WAIT_TIME)).informCapabilitiesUpdate(captor.capture());
        List<String> data = captor.getValue();

        assertEquals(data.size(), 1);
        assertEquals(data.get(0), pidfxml);

        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedCommandError() throws Exception {
        int key = 10;
        int commandCode = 100;

        UceSubscribeRequestController controller = createUceSubscribeRequestController();
        controller.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SUBSCRIBE_CMD_ERROR_IND);
        parcel.writeInt(key); // key
        parcel.writeInt(commandCode); // command error code
        parcel.setDataPosition(0);

        mUceJni.mUceJniListener.onSubscribeResponseMessage(parcel);

        Handler handler = controller.getHandler();
        waitForHandlerActionDelayed(handler, MAX_WAIT_TIME, 0);

        verify(request, timeout(MAX_WAIT_TIME)).informCommandError(eq(commandCode));
        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedTerminated() throws Exception {
        int key = 10;
        String reason = "test";
        int retryAfter = 100;

        UceSubscribeRequestController controller = createUceSubscribeRequestController();
        controller.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SUBSCRIBE_TERMINATED_IND);
        parcel.writeInt(key); // key
        parcel.writeString(reason); // reason
        parcel.writeInt(retryAfter); // retryAfterMillsecond
        parcel.setDataPosition(0);

        mUceJni.mUceJniListener.onSubscribeResponseMessage(parcel);

        Handler handler = controller.getHandler();
        waitForHandlerActionDelayed(handler, MAX_WAIT_TIME, 0);

        verify(request, timeout(MAX_WAIT_TIME)).informTerminate(eq(reason), eq(retryAfter));
        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedResourceTerminated() throws Exception {
        int key = 10;
        String[] id = {"test_id", "test_id2"};
        String[] reason = {"test reason", "test reason2"};
        int count = id.length;

        UceSubscribeRequestController controller = createUceSubscribeRequestController();
        controller.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND);
        parcel.writeInt(key); // key
        parcel.writeInt(count); // count
        parcel.writeString(id[0]); // id
        parcel.writeString(reason[0]); // reason
        parcel.writeString(id[1]); // id
        parcel.writeString(reason[1]); // reason
        parcel.setDataPosition(0);

        mUceJni.mUceJniListener.onSubscribeResponseMessage(parcel);

        Handler handler = controller.getHandler();
        waitForHandlerActionDelayed(handler, MAX_WAIT_TIME, 0);

        ArgumentCaptor<ArrayList> captor = ArgumentCaptor.forClass(ArrayList.class);

        verify(request, timeout(MAX_WAIT_TIME)).informResourceTerminate(captor.capture());
        ArrayList<UceResourceInfo> data = captor.getValue();
        assertEquals(data.size(), count);

        for (int i = 0; i < data.size(); i++) {
            UceResourceInfo info = data.get(i);
            assertEquals(info.getId(), id[i]);
            assertEquals(info.getReason(), reason[i]);
        }
        verifyNoMoreInteractions(request);
    }

    private UceSubscribeRequestController createUceSubscribeRequestController() {
        UceSubscribeRequestController controller = new UceSubscribeRequestController(SLOT_ID,
                mUceJni, Looper.getMainLooper());
        return controller;
    }

    private void waitForHandlerActionDelayed(Handler h, long timeoutMillis, long delayMs) {
        final CountDownLatch lock = new CountDownLatch(1);
        h.postDelayed(lock::countDown, delayMs);
        while (lock.getCount() > 0) {
            try {
                lock.await(timeoutMillis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // do nothing
            }
        }
    }

    private String getPidfxml() {
        StringBuilder pidfBuilder = new StringBuilder();
        pidfBuilder.append("<?xml version='1.0' encoding='utf-8' standalone='yes' ?>")
                .append("<presence entity=\"").append("123456").append("\"")
                .append(" xmlns=\"urn:ietf:params:xml:ns:pidf\"")
                .append(" xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"")
                .append(" xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\">")
                .append("<tuple id=\"tid0\"><status><basic>open</basic></status>")
                .append("<op:service-description>")
                .append("<op:service-id>org.openmobilealliance:StandaloneMsg</op:service-id>")
                .append("<op:version>1.0</op:version>")
                .append("<op:description>description_test1</op:description>")
                .append("</op:service-description>")
                .append("<contact>").append("123456").append("</contact>")
                .append("</tuple>")
                .append("<tuple id=\"tid1\"><status><basic>open</basic></status>")
                .append("<op:service-description>")
                .append("<op:service-id>org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel</op:service-id>")
                .append("<op:version>1.0</op:version>")
                .append("<op:description>description_test2</op:description>")
                .append("</op:service-description>")
                .append("<contact>").append("123456").append("</contact>")
                .append("</tuple></presence>");
        return pidfBuilder.toString();
    }
}
